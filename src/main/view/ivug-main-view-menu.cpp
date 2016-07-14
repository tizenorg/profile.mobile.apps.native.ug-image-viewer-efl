/*
* Copyright (c) 2000-2015  Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "ivug-common.h"
#include "ivug-main-view.h"
#include "ivug-main-view-toolbar.h"
#include "ivug-main-view-menu.h"
#include "ivug-main-view-priv.h"

#include "ivug-ext-ug.h"
#include "ivug-popup.h"

#include "ivug-media.h"
#include "ivug-slideshow.h"

#include "ivug-file-info.h"
#include "ivug-thumblist.h"

#include <storage/storage.h>
#include <shortcut_manager.h>
#include <efl_extension.h>
#include <app_manager.h>
#include <efl_extension.h>
#include <notification.h>
#include <string>
#include <glib.h>

#include "ivug-db.h"
#include "ivug-crop-view.h"
#include "ivug-util.h"
#include "ivug-config.h"
#include "ivug-language-mgr.h"

#define GALLERY_PKG_NAME	"com.samsung.gallery"
#define SHORTCUT_PREFIX		"gallery:imageviewer:"
#define SHORTCUT_PREFIX_LEN	strlen(SHORTCUT_PREFIX)
#define ICON_EDJ_PATH	full_path(EDJ_PATH, "/ivug-icons.edj")
#define ICON_NEAR_BY_PHONE		"icon.nearby.phone"
#define ICON_NEAR_BY_UNKNOWN	"icon.nearby.unknown"

static void
ivug_notification_popup_create(Evas_Object * pParent, const char* text)
{
	MSG_IMAGEVIEW_HIGH("ivug_notification_popup_create. Text=%s", GET_STR(text));

	Evas_Object *pPopup = elm_popup_add(pParent);
	elm_object_style_set(pPopup, "default");
	elm_object_part_text_set(pPopup, "default", text);
	elm_popup_timeout_set(pPopup, 3.0);
	elm_popup_orient_set(pPopup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_show(pPopup);
}

static Eina_Bool _on_back_timer_expired(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	pMainView->back_timer = NULL;

	pMainView->bPreventBackKey = false;

	return ECORE_CALLBACK_CANCEL;
}

bool ivug_is_agif(Ivug_MainView *pMainView, const char *filepath)
{
	Evas_Object *obj = evas_object_image_add(evas_object_evas_get(gGetCurrentWindow()));
	evas_object_image_file_set(obj, filepath, NULL);

	return evas_object_image_animated_get(obj);
}

static bool _save_to_folder(Ivug_MainView *pMainView, const char *path, const char *folder)
{
	char dest_file[IVUG_MAX_FILE_PATH_LEN + 1] = {0,};
	const char *new_filename = ivug_file_get(path);
	char *temp_filename = NULL;
	char error_msg[256] = {0,};

	if (new_filename == NULL) {
		MSG_MAIN_ERROR("File does not exist filepath=%s", path);
		ivug_notification_popup_create(pMainView->layout, "File download failed");
		return false;
	}

	if (ivug_is_dir_empty(folder) == -1) {
		MSG_MAIN_WARN("Destination path doesn't exist. %s", folder);
		if (mkdir(folder, DIR_MASK_DEFAULT) != 0) {
			if (errno != EEXIST) {
				MSG_MAIN_ERROR("Cannot make dir=%s error=%s", DIR_MASK_DEFAULT, strerror_r(errno, error_msg, sizeof(error_msg)));
			}
		}
	}

	snprintf(dest_file, IVUG_MAX_FILE_PATH_LEN, "%s/%s", folder, new_filename);
	if (ivug_file_exists(dest_file)) {
		ivug_notification_popup_create(pMainView->layout, "File already exists");
		temp_filename = ivug_generate_file_name(dest_file, NULL, NULL, false);
		snprintf(dest_file, IVUG_MAX_FILE_PATH_LEN, "%s/%s", folder, ivug_file_get(temp_filename));
		free(temp_filename);
		return false;
	} else {
		if (ivug_copy_file(path, dest_file) == false) {
			ivug_notification_popup_create(pMainView->layout, "File download failed");
			return false;
		}
	}

	/* Add to album */
	media_handle m_handle = ivug_db_insert_file_to_DB(dest_file);
	if (m_handle == NULL) {
		ivug_notification_popup_create(pMainView->layout, "File download failed");
		MSG_MAIN_ERROR("Cannot insert to db %s", dest_file);
		return false;
	}
	ivug_db_destroy_file_handle(m_handle);
	ivug_notification_popup_create(pMainView->layout, "File downloaded");
	return true;
}

static bool _idler_delete_end(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	bool ug_end = false;
	ivug_medialist_set_current_item(pMainView->mList, pMainView->cur_mitem);
	if (pMainView->cur_mitem) {
		Image_Object *img = ivug_thumblist_find_item_by_data(pMainView->thumbs, pMainView->cur_mitem);
		if (img) {
			ivug_thumblist_set_edit_mode(pMainView->thumbs, EINA_FALSE);
			ivug_thumblist_select_item(pMainView->thumbs, img);
		}
	} else {
		ug_end = true;
	}

	ivug_slider_new_update_list(pMainView->pSliderNew, pMainView->mList);
	ivug_slider_call_changed_callback(pMainView->pSliderNew, pMainView->cur_mitem);
	pMainView->cur_mitem = NULL;

	return ug_end;
}

static void
_progress_delete_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("_progress_delete_end_cb");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	evas_object_del(pMainView->progress_popup);

	pMainView->progress_popup = NULL;

	if (pMainView->delete_idler) {
		ecore_idler_del(pMainView->delete_idler);
		pMainView->delete_idler = NULL;
		_idler_delete_end(data);
	}
}

static bool _delete_mitem(Ivug_MainView *pMainView, Media_Item *mitem)
{
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	IV_ASSERT(mdata != NULL);

	MSG_MAIN_HIGH("Delete button is selected. Removing mdata=0x%08x", mdata);

	ivug_medialist_delete_item(pMainView->mList, mitem, true); //delete data.

	return true;
}

static void
_on_delete_selected(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	bool ret = false;

	evas_object_del(pMainView->popup);
	pMainView->popup = NULL;

	int *response_id = (int *)event_info;
	if (*response_id == POPUP_RESPONSE_CANCEL) {
		MSG_MAIN_HIGH("cancel selected");
		ivug_main_view_set_hide_timer(pMainView);
		return;
	}

	ivug_main_view_set_hide_timer(pMainView);

	MSG_MAIN_HIGH("Delete button is selected");

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);

	/* First, remove thumbnail and delete item */

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		Image_Object *deleted = ivug_thumblist_get_selected_item(pMainView->thumbs);
		pMainView->delete_list = ivug_thumblist_get_items_checked(pMainView->thumbs);
		if (pMainView->delete_list) {	/* remove selected list item */
			pMainView->cur_mitem = ivug_medialist_get_current_item(pMainView->mList);
			pMainView->delete_total = eina_list_count(pMainView->delete_list);
			pMainView->progress_popup = ivug_progress_popup_show(pMainView->layout, (char *)IDS_DELETE, _progress_delete_end_cb, pMainView);
			return;
		} else if (deleted) {	/* remove current thumbnail item */
			ivug_thumblist_delete_item(pMainView->thumbs, deleted);
		}
	}
#endif

	/* remove current item only */
	ret = _delete_mitem(pMainView, mitem);
	if (ret == false) {
		return;
	}

	/* Current Item */
	mitem = ivug_medialist_get_current_item(pMainView->mList);

	if (mitem == NULL) {
		MSG_MAIN_HIGH("Current item is NULL");
		elm_exit();
		return;
	}

	/* ivug_slider_new_delete_cur_image(pMainView->pSliderNew); */
	ivug_main_view_start(pMainView, NULL);
}

void _on_remove_main_view_ui(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	ivug_main_view_del_hide_timer(pMainView);

	if (pMainView->tagbuddy_idler) {
		ecore_timer_del(pMainView->tagbuddy_idler);
		pMainView->tagbuddy_idler = NULL;
	}

	if (pMainView->transcoding_thread) {
		ecore_thread_cancel(pMainView->transcoding_thread);
		pMainView->transcoding_thread = NULL;
	}

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		evas_object_del(pMainView->thumbs);
		pMainView->thumbs = NULL;
	}
#endif

	if (pMainView->pNameView) {
		MSG_MAIN_HIGH("Name View Destroy");
		ivug_name_view_destroy(pMainView->pNameView);
		pMainView->pNameView = NULL;
	}

	if (pMainView->pSliderNew) {
		MSG_MAIN_HIGH("slider new View Destroy");

		ivug_slider_new_destroy(pMainView->pSliderNew);
		pMainView->pSliderNew = NULL;
	}

	if (pMainView->lyContent) {
		evas_object_del(pMainView->lyContent);
		pMainView->lyContent = NULL;
	}

	if (pMainView->slide_move_timer) {
		ecore_timer_del(pMainView->slide_move_timer);
		pMainView->slide_move_timer = NULL;
	}

	if (pMainView->mList) {
		MSG_MAIN_HIGH("Remove media list. mList=0x%08x", pMainView->mList);
		ivug_medialist_del(pMainView->mList);
		// ivug_medialist_del() is not working on destroy cb.
		pMainView->mList = NULL;
	}
}

static bool _is_exist(Media_Data *mdata, const char *file)
{
	IV_ASSERT(mdata != NULL);
	char *old_dir = ivug_get_directory(mdata->filepath);
	char *ext = ivug_fileinfo_get_file_extension(mdata->filepath);
	char new_fullpath[IVUG_MAX_FILE_PATH_LEN] = {0,};
	struct stat info = {0,};
	if (ext) {
		snprintf(new_fullpath, sizeof(new_fullpath), "%s/%s.%s", old_dir, file, ext);
		free(ext);
	} else {
		snprintf(new_fullpath, sizeof(new_fullpath), "%s/%s", old_dir, file);
	}

	//Check if File Exists
	if (stat(new_fullpath, &info) == 0) {
		MSG_UTIL_WARN("Destination file is exist : %s", new_fullpath);
		if (old_dir) {
			free(old_dir);
			old_dir = NULL;
		}
		return true;
	}

	return false;
}

static bool _rename(Media_Data *mdata, const char *str)
{
	IV_ASSERT(mdata != NULL);

	if (mdata->filepath == NULL || mdata->thumbnail_path == NULL) {
		MSG_MAIN_ERROR("filepath is NULL");
		return false;
	}

	const char *old_fullpath = mdata->filepath;
	char *old_dir = ivug_get_directory(old_fullpath);
	char *ext = ivug_fileinfo_get_file_extension(old_fullpath);

	char new_fullpath[IVUG_MAX_FILE_PATH_LEN] = {0,};
	if (ext) {
		snprintf(new_fullpath, sizeof(new_fullpath), "%s/%s.%s", old_dir, str, ext);
		free(ext);
	} else {
		snprintf(new_fullpath, sizeof(new_fullpath), "%s/%s", old_dir, str);
	}
	if (old_dir) {
		free(old_dir);
	}
	if (old_fullpath) {
		if (!strcmp(old_fullpath, new_fullpath)) {
			MSG_MAIN_HIGH("duplicate name, no need to rename!");
			return true;
		}
	}
	if (ivug_rename_file(old_fullpath, new_fullpath) == false) {
		MSG_MAIN_ERROR("ivug_rename_file to %s failed", new_fullpath);
		return false;
	}

	if (ivug_db_rename(mdata->m_handle, new_fullpath) == false) {
		MSG_MAIN_ERROR("ivug_db_rename to %s failed", new_fullpath);
		return false;
	}

	ivug_db_destroy_file_handle(mdata->m_handle);

	mdata->m_handle = ivug_db_get_file_handle_from_media_id(mdata->mediaID);

	MSG_MAIN_SEC("Rename %s -> %s", old_fullpath, new_fullpath);

	free(mdata->filepath);
	mdata->filepath = strdup(new_fullpath);

	free(mdata->fileurl);
	mdata->fileurl = strdup(new_fullpath);

	free(mdata->thumbnail_path);
	mdata->thumbnail_path = ivug_db_get_thumbnail_path(mdata->m_handle);

	return true;
}

void
_set_thumblist_mode(Ivug_MainView *pMainView, Media_Data *mdata, Image_Object *image_obj)
{
	if (mdata->iType == MIMAGE_TYPE_BESTSHOT) {
		ivug_thumblist_set_item_mode(pMainView->thumbs, image_obj, THUMBLIST_BESTPIC);
	} else  if (mdata->iType == MIMAGE_TYPE_SOUNDSCENE) {
		ivug_thumblist_set_item_mode(pMainView->thumbs, image_obj, THUMBLIST_SOUNDPIC);
	} else  if (mdata->iType == MIMAGE_TYPE_PANORAMA) {
		ivug_thumblist_set_item_mode(pMainView->thumbs, image_obj, THUMBLIST_PANORAMA);
	} else {
		MSG_IMAGEVIEW_MED("Thumblist is not updated. Unhandled iType=%d", mdata->iType);
	}
}

void
ivug_notification_create(const char* text)
{
	MSG_IMAGEVIEW_HIGH("ivug_notification_create. Text=%s", GET_STR(text));

	int ret = notification_status_message_post(GET_STR(text));
	if (ret != NOTIFICATION_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("notification_status_message_post() ERROR [0x%x]", ret);
	}
}

static void
_on_button_rename_view_response(Ivug_NameView *pView, ivug_name_response resp, const char *str, void *pClientData)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)pClientData;

	pMainView->popup = NULL;
	if (resp != NAME_VIEW_RESPONSE_OK) {
		MSG_MAIN_ERROR("rename is canceled");

		ivug_name_view_destroy(pMainView->pNameView);
		pMainView->pNameView = NULL;	// Will removed in add tag view.

		return;
	}

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mData = ivug_medialist_get_data(mitem);

	if (_is_exist(mData, str)) {
		MSG_MAIN_ERROR("%s already exist", str);

		ivug_notification_create(IDS_RENAME_FILE_EXIST);

		ivug_name_view_show_imf(pMainView->pNameView);

		return;
	} else {
		if (_rename(mData, str) == false) {
			MSG_MAIN_ERROR("_rename to %s failed", str);
			ivug_notification_create(IDS_FAILED);
		} else {
			//update main view
			if (pMainView->view_by == IVUG_VIEW_BY_FILE) {
				// Update title
				const char *title = ecore_file_file_get(mData->filepath);
				elm_object_item_part_text_set(pMainView->navi_it, "elm.text.title", title);
			}

#ifdef USE_THUMBLIST
			if (pMainView->thumbs) {
				Image_Object *img_obj = NULL;
				img_obj = ivug_thumblist_get_selected_item(pMainView->thumbs);

				if (img_obj != NULL) {
					bool bVideo = (mData->slide_type == SLIDE_TYPE_VIDEO);
					ivug_thumblist_update_item(pMainView->thumbs, img_obj, mData->thumbnail_path, bVideo, mitem);
					_set_thumblist_mode(pMainView, mData, img_obj);
				}
			}
#endif
			Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);

			if (mData->slide_type == SLIDE_TYPE_VIDEO) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
				pMainView->is_play_Icon = true;
				elm_photocam_file_set(ivug_slider_new_get_photocam(pMainView->pSliderNew), mData->thumbnail_path);
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
				pMainView->is_play_Icon = false;
				Evas_Load_Error e = elm_photocam_file_set(ivug_slider_new_get_photocam(pMainView->pSliderNew), mData->filepath);

				if (EVAS_LOAD_ERROR_NONE != e) {
					MSG_HIGH("Loading default Thumbnail");
					elm_photocam_file_set(pMainView->photocam, DEFAULT_THUMBNAIL_PATH);
				}
			}
		}
	}

	ivug_name_view_destroy(pMainView->pNameView);
	pMainView->pNameView = NULL;	// Will removed in add tag view.
}

static void
_on_btn_rename_clicked(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	pMainView->pNameView = ivug_name_view_create(pMainView->layout, NAME_VIEW_MODE_SINGLE_LINE);
	IV_ASSERT(pMainView->pNameView != NULL);

	ivug_name_view_set_title(pMainView->pNameView, IDS_RENAME);
	ivug_name_view_set_guide_text(pMainView->pNameView, IDS_ENTER_NAME);
	ivug_name_view_set_filter_text(pMainView->pNameView, INVALID_FILENAME_CHAR);

	char *name = ecore_file_strip_ext(ecore_file_file_get(mdata->filepath));

	ivug_name_view_set_text(pMainView->pNameView, name);

	// file name have to be smaller then MAX_BYTE_LEN include extension + '.'
	int limit_len = MAX_CHAR_LEN;

	char *ext = ivug_fileinfo_get_file_extension(mdata->filepath);
	if (ext) {
		limit_len -= (strlen(ext) + 1); // 1 is '.'
	}

	ivug_name_view_set_max_length(pMainView->pNameView, limit_len);

	ivug_name_view_set_response_callback(pMainView->pNameView, _on_button_rename_view_response, (void*)pMainView);

	if (ext) {
		free(ext);
	}

	free(name);

	ivug_name_view_set_focus(pMainView->pNameView);

	pMainView->popup = ivug_name_view_get_popup(pMainView->pNameView);
}

static void
_on_btn_download_clicked(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	char *download_folder = NULL;
	storage_get_directory(STORAGE_TYPE_INTERNAL, STORAGE_DIRECTORY_DOWNLOADS, &download_folder);

	char buf[IVUG_MAX_FILE_PATH_LEN] = {0,};
	snprintf(buf, (size_t)sizeof(buf), "%s", download_folder);
	free(download_folder);
	MSG_MAIN_HIGH("%s", buf);
	if (_save_to_folder(pMainView, mdata->filepath, buf)) {
		MSG_MAIN_HIGH("File Downloaded");
	} else {
		MSG_MAIN_HIGH("File  Not Downloaded");
	}

}

void _on_mainview_delete(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_HIGH("Btn Delete is clicked. pMainView(0x%08x)", pMainView);

	if (pMainView->popup) {
		MSG_MAIN_WARN("popup already exist");
		return;
	}

	ivug_main_view_del_hide_timer(pMainView);
	Media_Data *mdata = ivug_medialist_get_data(pMainView->cur_mitem);

	if (mdata->slide_type == SLIDE_TYPE_IMAGE) {
		pMainView->popup = ivug_deletepopup_show(pMainView->navi_bar,
		                   GET_STR(IDS_DELETE_IMAGE),
		                   GET_STR(IDS_DELETE_IMAGE_MESSAGE),
		                   _on_delete_selected,
		                   pMainView);
	} else {
		pMainView->popup = ivug_deletepopup_show(pMainView->navi_bar,
		                   GET_STR(IDS_DELETE_IMAGE),
		                   GET_STR(IDS_DELETE_VIDEO_MESSAGE),
		                   _on_delete_selected,
		                   pMainView);
	}

	return;
}

void on_btn_slideshow_clicked(Ivug_MainView *pMainView)
{
#ifdef USE_THUMBLIST
	Eina_List *selected_list = NULL;
	Eina_List *list = NULL;
	Eina_List *l = NULL;
	void *l_data = NULL;
	Image_Object *img = NULL;
	Media_Item *mitem = NULL;
	Media_Data *mdata = NULL;

	if (pMainView->thumbs != NULL && ivug_thumblist_get_edit_mode(pMainView->thumbs) == EINA_TRUE) {
		list = ivug_thumblist_get_items_checked(pMainView->thumbs);
		if (list == NULL) {
			MSG_MAIN_HIGH("checked item is null");
			return;
		}

		char *filepath = NULL;	// start file path

		EINA_LIST_FOREACH(list, l, l_data) {
			img = (Image_Object *)l_data;
			mitem = (Media_Item *)ivug_thumblist_get_item_data(pMainView->thumbs, img);
			mdata = ivug_medialist_get_data(mitem);
			if (filepath == NULL) {
				filepath = mdata->fileurl;
			}
			selected_list = eina_list_append(selected_list, (void *)mdata->index);
		}

		if (filepath == NULL) {
			MSG_MAIN_ERROR("checked item's path is invalid");
			return;
		}

		Media_List *mlist = ivug_medialist_create();
		Filter_struct * filter = ivug_medialist_get_filter(pMainView->mList);
		filter->selected_list = selected_list;
		if (filter->filepath) {
			free(filter->filepath);
		}
		filter->filepath = strdup(filepath);
		Media_Item *current = ivug_medialist_load(mlist, filter);

		pMainView->temp_mlist = mlist;

		pMainView->ssHandle = ivug_ss_create(pMainView->layout);

//		ivug_allow_lcd_off();

		ivug_main_view_hide_menu_bar(pMainView);

		ivug_ss_start(pMainView->ssHandle, current, mlist, false);

	} else {
		ivug_main_view_start_slideshow(pMainView, EINA_FALSE);
	}
#else
	_ivug_slideshow_popup_create_menu(data, obj, event_info);
#endif

}

#include <Ecore_File.h>

static void
_ivug_ctxpopup_download_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	_on_btn_download_clicked(pMainView);
}

static void
_ivug_ctxpopup_slideshow_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	on_btn_slideshow_clicked(pMainView);
}

static void
_ivug_ctxpopup_delete_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	_on_mainview_delete(pMainView);
}

static void
_ivug_ctxpopup_details_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	//[ToDo]
	return;
}

static void
_ivug_ctxpopup_rename_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	_on_btn_rename_clicked(pMainView);
}

static void _on_btn_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	ListPopup *pListPopup = (ListPopup *)data;

	if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_POPUP)
	{

		MSG_MAIN_HIGH("List Popup Dismissed. pListPopup=0x%08x", pListPopup);
		pListPopup->state = IVUG_LISTPOPUP_STATE_DISMISSED;
		evas_object_smart_callback_call(pListPopup->obj, "popup,dismissed", NULL);

	}
	else if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_CTX_POPUP)
	{
		MSG_MAIN_HIGH("Ctx Popup Dismissed. pListPopup=0x%08x", pListPopup);

		elm_ctxpopup_dismiss(pListPopup->popup);
	}
}

static void
_on_ctxpopup_selected(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;

	MSG_MAIN_WARN("Ctxpopup item selected. item=0x%08x %s", item, item->caption_id);

	evas_object_smart_callback_call(item->obj, "popup,selected", item);
}

static void _ivug_move_more_ctxpopup( Evas_Object *win, Evas_Object *ctxpopup)
{
	Evas_Coord w, h;
	int pos = -1;
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);

	pos = elm_win_rotation_get(win);

	switch (pos) {
	case 0:
	case 180:
		evas_object_move(ctxpopup, w/2, h);
		break;
	case 90:
	case 270:
		evas_object_move(ctxpopup, h/2, w);
		break;
	}
}

static void __ivug_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *ei)
{
	IV_ASSERT(data != NULL);
	IV_ASSERT(obj != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);


	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}
}

static void __ivug_ctxpopup_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		_ivug_move_more_ctxpopup(gGetCurrentWindow(), pMainView->ctx_popup);
		evas_object_show(pMainView->ctx_popup);
	}
}

static void __ivug_ctxpopup_del_cb(void *data, Evas *e, Evas_Object *obj, void *ei)
{
	IV_ASSERT(data != NULL);
	IV_ASSERT(obj != NULL);
	Evas_Object *ctxpopup = obj;
	IV_ASSERT(ctxpopup != NULL);


	evas_object_smart_callback_del(ctxpopup, "dismissed",
								   __ivug_ctxpopup_hide_cb);

	evas_object_smart_callback_del(elm_object_top_widget_get(ctxpopup),
								   "rotation,changed",
								   __ivug_ctxpopup_rotate_cb);

	evas_object_event_callback_del(ctxpopup, EVAS_CALLBACK_DEL,
								   __ivug_ctxpopup_del_cb);

}

static int _ivug_ctxpopup_add_callbacks(void *data, Evas_Object *ctxpopup)
{
	IV_ASSERT(data != NULL);
	IV_ASSERT(ctxpopup != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	evas_object_smart_callback_add(ctxpopup, "dismissed",
								   __ivug_ctxpopup_hide_cb, data);
	evas_object_event_callback_add(ctxpopup, EVAS_CALLBACK_DEL,
								   __ivug_ctxpopup_del_cb, data);

	evas_object_smart_callback_add(elm_object_top_widget_get(ctxpopup),
								   "rotation,changed",
								   __ivug_ctxpopup_rotate_cb, data);

	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	return 0;
}

void on_btn_more_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	MSG_MAIN_HIGH("More clicked. Mode=%d", pMainView->mode);

	if (gGetDestroying() == true) {
		MSG_MAIN_WARN("UG is destroying");
		return;
	}

	if (pMainView->ctx_popup || pMainView->isSliding == true) {
		MSG_MAIN_HIGH("Previous CTX popup is exist");
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
		return;
	}

	if (pMainView->mode == IVUG_MODE_SAVE) {
		MSG_MAIN_HIGH("Current mode is SAVE. Ignore More event");
		return;
	}

	IV_ASSERT(mdata != NULL);

	if (pMainView->bShowMenu == true) {
		ivug_main_view_del_hide_timer(pMainView);
	}

	Evas_Object *ctxpopup = elm_ctxpopup_add(gGetCurrentWindow());
	elm_object_style_set(ctxpopup, "more/default");

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	pMainView->ctx_popup = ctxpopup;
	char *default_thumbnail_edj_path = DEFAULT_THUMBNAIL_PATH;

	if (pMainView->mode == IVUG_MODE_EMAIL) {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_FILE_DOWNLOAD), NULL, _ivug_ctxpopup_download_sel_cb, pMainView);
	} else if (pMainView->view_by == IVUG_VIEW_BY_FAVORITES){
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_SLIDE_SHOW), NULL, _ivug_ctxpopup_slideshow_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_RENAME_IMAGE), NULL, _ivug_ctxpopup_rename_sel_cb, pMainView);
	} else if (!strcmp(elm_photocam_file_get(ivug_slider_new_get_photocam(pMainView->pSliderNew)),
					  default_thumbnail_edj_path)) {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_DELETE_IMAGE), NULL, _ivug_ctxpopup_delete_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_DETAILS), NULL, _ivug_ctxpopup_details_sel_cb, pMainView);
	} else {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_SLIDE_SHOW), NULL, _ivug_ctxpopup_slideshow_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_DELETE_IMAGE), NULL, _ivug_ctxpopup_delete_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_RENAME_IMAGE), NULL, _ivug_ctxpopup_rename_sel_cb, pMainView);
	}

	free(default_thumbnail_edj_path);
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	_ivug_move_more_ctxpopup(gGetCurrentWindow(), ctxpopup);
	evas_object_show(ctxpopup);

	if (ctxpopup) {
		_ivug_ctxpopup_add_callbacks(pMainView, ctxpopup);
	}
}
