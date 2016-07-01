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

#include "ivug-exif.h"

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
#include "ivug-comment.h"
#include "ivug-config.h"
#include "ivug-language-mgr.h"
#include "ivug-transcoder.h"

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

static void _launch_share_app(Ivug_MainView *pMainView, const char* filepath)
{
	if (pMainView->back_timer) {
		ecore_timer_del(pMainView->back_timer);
	}
	pMainView->back_timer = ecore_timer_add(0.5, _on_back_timer_expired, pMainView);
	pMainView->bPreventBackKey = true;

#ifdef IV_EXTENDED_FEATURES
	ivug_ext_launch_default(filepath, "http://tizen.org/appcontrol/operation/share", NULL, NULL, NULL);
#endif
}

bool ivug_is_agif(Ivug_MainView *pMainView, const char *filepath)
{
	Evas_Object *obj = evas_object_image_add(evas_object_evas_get(gGetCurrentWindow()));
	evas_object_image_file_set(obj, filepath, NULL);

	return evas_object_image_animated_get(obj);
}

static void _on_add_tag_view_destroy(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("transition finished");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	ivug_name_view_destroy(pMainView->pNameView);
	pMainView->pNameView = NULL;

	evas_object_smart_callback_del(pMainView->navi_bar, "transition,finished",
	                               _on_add_tag_view_destroy);
}

static void _on_add_tag_view_response(Ivug_NameView *pView, ivug_name_response resp, const char *str, void *pClientData)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)pClientData;

	switch (resp) {
	case NAME_VIEW_RESPONSE_OK:

		evas_object_smart_callback_add(pMainView->navi_bar, "transition,finished",
		                               _on_add_tag_view_destroy, pMainView);

		elm_naviframe_item_pop(pMainView->navi_bar);

		pMainView->navi_it = elm_naviframe_top_item_get(pMainView->navi_bar);

		break;
	case NAME_VIEW_RESPONSE_CANCEL:
		MSG_MAIN_HIGH("Add tag is canceled");
		evas_object_smart_callback_add(pMainView->navi_bar, "transition,finished",
		                               _on_add_tag_view_destroy, pMainView);

		elm_naviframe_item_pop(pMainView->navi_bar);

		pMainView->navi_it = elm_naviframe_top_item_get(pMainView->navi_bar);

		break;
	default:
		MSG_MAIN_ERROR("Unhandled mode : %d", resp);
		break;
	}

	ivug_main_view_set_hide_timer(pMainView);
}

static void _on_edit_weather_view_destroy(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("transition finished");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	evas_object_smart_callback_del(pMainView->navi_bar, "transition,finished",
	                               _on_edit_weather_view_destroy);
}


static void _ivug_crop_view_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *path = (char *)event_info;
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	MSG_MAIN_HIGH("_ivug_crop_view_ok_clicked_cb path=%s", path);

	evas_object_smart_callback_del(obj, "ok,clicked", _ivug_crop_view_ok_clicked_cb);

	if (path == NULL) {
		MSG_MAIN_ERROR("Crop failed! pMainView(0x%08x)", pMainView);
		return ;
	}

	media_handle m_handle = NULL;

	m_handle = ivug_db_insert_file_to_DB(path);
	if (m_handle == NULL) {
		MSG_MAIN_ERROR("ivug_db_insert_file_to_DB failed %s", path);
	} else {
		ivug_db_destroy_file_handle(m_handle);
	}


	Media_Item *mitem = ivug_medialist_prepend_item(pMainView->mList, path);

#ifdef USE_THUMBLIST
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (pMainView->thumbs) {
		bool bVideo = (mdata->slide_type == SLIDE_TYPE_VIDEO);
		Image_Object *img = ivug_thumblist_prepend_item(pMainView->thumbs, mdata->thumbnail_path, bVideo, mitem);
		ivug_thumblist_select_item(pMainView->thumbs, img);	// set focus to cropped image
	}
#endif

	ivug_medialist_set_current_item(pMainView->mList, mitem);
	ivug_slider_new_update_list(pMainView->pSliderNew, pMainView->mList);
}

void *ivug_listpopup_item_get_data(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	return item->data;
}

const char *ivug_listpopup_item_get_text(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	return item->caption_id;
}

void _on_addtag_selected(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)event_info;

	const char *label = ivug_listpopup_item_get_text(item);

	if (label == NULL) {
		MSG_MAIN_ERROR("label is NULL");
		evas_object_del(pMainView->ctx_popup2);
		pMainView->ctx_popup2 = NULL;
		ivug_main_view_set_hide_timer(pMainView);
		return;
	}

	MSG_MAIN_HIGH("text(%s) is clicked", label);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (mdata == NULL) {
		MSG_MAIN_ERROR("sd is NULL");
		evas_object_del(pMainView->ctx_popup2);
		pMainView->ctx_popup2 = NULL;
		ivug_main_view_set_hide_timer(pMainView);
		return;
	}

	if (strncmp(label, IDS_CREATE_TAG, strlen(label)) == 0) {
		pMainView->pNameView = ivug_name_view_create(pMainView->layout, NAME_VIEW_MODE_SINGLE_LINE);
		IV_ASSERT(pMainView->pNameView != NULL);

		ivug_name_view_set_title(pMainView->pNameView, IDS_CREATE_TAG);
		ivug_name_view_set_max_length(pMainView->pNameView, MAX_BYTE_LEN);
		ivug_name_view_set_guide_text(pMainView->pNameView, IDS_TAG);

		ivug_name_view_set_response_callback(pMainView->pNameView, _on_add_tag_view_response, (void*)pMainView);

		Evas_Object *layout = ivug_name_view_object_get(pMainView->pNameView);

		pMainView->navi_it = elm_naviframe_item_push(pMainView->navi_bar, NULL, NULL, NULL, layout, NULL);

		elm_naviframe_item_title_enabled_set(pMainView->navi_it, EINA_FALSE, EINA_FALSE);
		elm_object_item_signal_emit(pMainView->navi_it, "elm,state,toolbar,close", "");

		ivug_name_view_set_focus(pMainView->pNameView);
	} else if (strncmp(label, IDS_FAVOURITE, strlen(label)) == 0) {
		if (ivug_mediadata_set_favorite(mdata, true) == false) {
			MSG_MAIN_ERROR("Error!. Set favorite for ID=%s", uuid_getchar(mdata->mediaID));
			goto ADD_FAIL;
		}

		ivug_main_view_set_hide_timer(pMainView);
	} else {
		bool ret = ivug_mediadata_set_tag(mdata, (char *)label);
		if (ret == false) {
			goto ADD_FAIL;
		}
		ivug_main_view_set_hide_timer(pMainView);
	}

	evas_object_del(pMainView->ctx_popup2);
	pMainView->ctx_popup2 = NULL;

	return;
ADD_FAIL:
	evas_object_del(pMainView->ctx_popup2);
	pMainView->ctx_popup2 = NULL;

	ivug_main_view_set_hide_timer(pMainView);

	return;

}

static bool _save_to_folder(Ivug_MainView *pMainView, const char *path, const char *folder)
{
	char dest_file[IVUG_MAX_FILE_PATH_LEN + 1] = {0,};
	const char *new_filename = ivug_file_get(path);
	char *temp_filename = NULL;
	char error_msg[256] = {0,};
	//int err = 0;

	if (new_filename == NULL) {
		MSG_MAIN_ERROR("File does not exist filepath=%s", path);
		ivug_notification_popup_create(pMainView->layout, "File download failed");
		return false;
	}

	if (ivug_is_dir_empty(folder) == -1) {
		MSG_MAIN_WARN("Destination path doesn't exist. %s", folder);
		if (mkdir(folder, DIR_MASK_DEFAULT) != 0) {
			if (errno != EEXIST) {
				//err = strerror_r(errno, error_msg, sizeof(error_msg));
				MSG_MAIN_ERROR("Cannot make dir=%s error=%s", DIR_MASK_DEFAULT, strerror_r(errno, error_msg, sizeof(error_msg)));
				// return false;	// TODO: Need to test.
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

static void _on_save_view_destroy(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("transition finished");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	ivug_name_view_destroy(pMainView->pNameView);
	pMainView->pNameView = NULL;

	evas_object_smart_callback_del(pMainView->navi_bar, "transition,finished",
	                               _on_save_view_destroy);
}

static void _on_save_view_response(Ivug_NameView *pView, ivug_name_response resp, const char *str, void *pClientData)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)pClientData;
	Media_Item *mitem = NULL;
	Media_Data *mdata = NULL;

	char buf[IVUG_MAX_FILE_PATH_LEN];

	switch (resp) {
	case NAME_VIEW_RESPONSE_OK:
		mitem = ivug_medialist_get_current_item(pMainView->mList);
		mdata = ivug_medialist_get_data(mitem);

		snprintf(buf, (size_t)sizeof(buf), "%s/%s",	DEFAULT_IMAGE_FOLDER, str);
		_save_to_folder(pMainView, mdata->filepath, buf);

		evas_object_smart_callback_add(pMainView->navi_bar, "transition,finished",
		                               _on_save_view_destroy, pMainView);

		elm_naviframe_item_pop(pMainView->navi_bar);

		pMainView->navi_it = elm_naviframe_top_item_get(pMainView->navi_bar);

		break;
	case NAME_VIEW_RESPONSE_CANCEL:
		MSG_MAIN_HIGH("Create album is canceled");
		evas_object_smart_callback_add(pMainView->navi_bar, "transition,finished",
		                               _on_save_view_destroy, pMainView);

		elm_naviframe_item_pop(pMainView->navi_bar);

		pMainView->navi_it = elm_naviframe_top_item_get(pMainView->navi_bar);

		break;
	default:
		MSG_MAIN_ERROR("Unhandled mode : %d", resp);
		break;
	}

	ivug_main_view_set_hide_timer(pMainView);
}

void _on_save_selected(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)event_info;

	const char *label = ivug_listpopup_item_get_text(item);

	if (label == NULL) {
		MSG_MAIN_ERROR("label is NULL");
		evas_object_del(pMainView->popup);
		pMainView->popup = NULL;
		ivug_main_view_set_hide_timer(pMainView);
		return;
	}

	MSG_MAIN_HIGH("Selected folder name = %s", label);

	char dst_file[IVUG_MAX_FILE_PATH_LEN + 1] = {0,};
	char *dst_path = (char *)ivug_listpopup_item_get_data(item);
	const char *filename = NULL;
	media_handle m_handle = NULL;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (mdata == NULL) {
		MSG_MAIN_ERROR("Cannot get slide data.");
		goto SAVE_FAIL;
	}

	if (strncmp(label, IDS_CREATE_ALBUM, strlen(label)) == 0) {
		pMainView->pNameView = ivug_name_view_create(pMainView->layout, NAME_VIEW_MODE_SINGLE_LINE);
		IV_ASSERT(pMainView->pNameView != NULL);

		ivug_name_view_set_title(pMainView->pNameView, IDS_CREATE_ALBUM);
		ivug_name_view_set_max_length(pMainView->pNameView, MAX_BYTE_LEN);
		ivug_name_view_set_guide_text(pMainView->pNameView, IDS_ENTER_NAME);
		ivug_name_view_set_filter_text(pMainView->pNameView, INVALID_FILENAME_CHAR);

		ivug_name_view_set_response_callback(pMainView->pNameView, _on_save_view_response, (void*)pMainView);

		Evas_Object *layout = ivug_name_view_object_get(pMainView->pNameView);

		pMainView->navi_it = elm_naviframe_item_push(pMainView->navi_bar, NULL, NULL, NULL, layout, NULL);

		elm_naviframe_item_title_enabled_set(pMainView->navi_it, EINA_FALSE, EINA_FALSE);
		elm_object_item_signal_emit(pMainView->navi_it, "elm,state,toolbar,close", "");

		ivug_name_view_set_focus(pMainView->pNameView);
		return;
	}

	filename = ivug_file_get(mdata->filepath);
	if (filename == NULL) {
		MSG_MAIN_ERROR("File does not existfilepath=%s", mdata->filepath);
		goto SAVE_FAIL;
	}

	snprintf(dst_file, IVUG_MAX_FILE_PATH_LEN, "%s/%s", dst_path, filename);

	if (!ivug_file_cp(mdata->filepath, dst_file)) {
		MSG_MAIN_ERROR("ivug_file_cp failed. From %s To %s", mdata->filepath, dst_file);
		goto SAVE_FAIL;
	}

	m_handle = ivug_db_insert_file_to_DB(dst_file);
	if (m_handle == NULL) {
		MSG_MAIN_ERROR("ivug_db_insert_file_to_DB failed %s", dst_file);
	} else {
		ivug_db_destroy_file_handle(m_handle);
	}

	if (dst_path) {
		free(dst_path);
	}

	evas_object_del(pMainView->popup);
	pMainView->popup = NULL;

	ivug_main_view_set_hide_timer(pMainView);

	return;

SAVE_FAIL:
	if (dst_path) {
		free(dst_path);
	}

	evas_object_del(pMainView->popup);
	pMainView->popup = NULL;

	ivug_main_view_set_hide_timer(pMainView);

	return;
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
		ivug_main_view_destroy(pMainView);
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

	if (pMainView->mList) {
		MSG_MAIN_HIGH("Remove media list. mList=0x%08x", pMainView->mList);
		ivug_medialist_del(pMainView->mList);
		// ivug_medialist_del() is not working on destroy cb.
		pMainView->mList = NULL;
	}
}

void on_btn_copy_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	// Destroy copy popup
	pMainView->longpress_popup = NULL;
	// object is removed automatically

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (mdata == NULL) {
		MSG_MAIN_ERROR("slider data is NULL");
		return;
	}

	int len = 0;
	// This Will add to the article
	char buf[IVUG_MAX_FILE_PATH_LEN] = {0,};
	len = strlen(mdata->filepath) + strlen("file://") + 1;
	snprintf(buf, IVUG_MAX_FILE_PATH_LEN, "file://%s", mdata->filepath);


	if (len < IVUG_MAX_FILE_PATH_LEN) {
		Eina_Bool bRet;

		if (ivug_is_web_uri(buf) == false) {
			MSG_MAIN_HIGH("CNP As Image : Buf=\"%s\" len=%d", buf, strlen(buf));
			bRet = elm_cnp_selection_set(pMainView->layout, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_IMAGE, buf, strlen(buf));
		} else {
			MSG_MAIN_HIGH("CNP As Text : Buf=\"%s\" len=%d", buf, strlen(buf));
			bRet = elm_cnp_selection_set(pMainView->layout, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_TEXT, buf, strlen(buf));
		}

		if (bRet == EINA_FALSE) {
			MSG_MAIN_ERROR("CNP Selection is failed");
		} else {
			MSG_MAIN_HIGH("CNP Selection is Success");
		}
	} else {
		MSG_MAIN_ERROR("slider file path is too long. len=%d", len);
		// No need failed????
	}
}

static void _on_name_view_destroy(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("transition finished");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	ivug_name_view_destroy(pMainView->pNameView);
	pMainView->pNameView = NULL;// Will removed in add tag view.

	evas_object_smart_callback_del(pMainView->navi_bar, "transition,finished",
	                               _on_name_view_destroy);
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

	/*
		ivug_db_rename() Ŀ Media UUID  ʴ´.
	*/
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
		//ivug_notification_create(IDS_RENAME_FILE_EXIST);

		/*toast popup: file name is already in use*/

		ivug_notification_create(IDS_RENAME_FILE_EXIST);

		ivug_name_view_show_imf(pMainView->pNameView);

		/*
		Evas_Object *popup = ivug_toast_popup_show(pMainView->layout, IDS_FILE_NAME_IN_USE, _on_rename_ctrl_timeout_cb, pMainView);

		if (!popup) {
			MSG_MAIN_ERROR("toast popup was created failed");
		}
		*/

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
		}

// No need to reload image when rename!it just change file name
//		pMainView->pPhotoViewer->ReloadImage();

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
}

static void
_on_btn_download_clicked(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	char buf[IVUG_MAX_FILE_PATH_LEN] = {0,};
	snprintf(buf, (size_t)sizeof(buf), "%s", DEFAULT_DOWNLOADS_FOLDER);
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

bool _share_pkg_cb(app_control_h service, const char *package, void *user_data)
{
	return true;
}

void _on_mainview_share(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	if (pMainView->popup) {
		MSG_MAIN_WARN("popup already exist");
		return;
	}

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	IV_ASSERT(mdata != NULL);

	if (mdata->filepath == NULL) {
		MSG_MAIN_ERROR("File path is NULL");
		return;
	}

#ifdef USE_SOUNDIMAGE_SHARE
	if (mdata->iType == MIMAGE_TYPE_SOUNDSCENE) {
		MSG_MAIN_HIGH("Sharing Sound&Image");

		ivug_main_view_del_hide_timer(pMainView);

		// Select popup shows.
		Evas_Object *popup = ivug_listpopup_add(pMainView->layout);

		ivug_listpopup_set_style(popup, IVUG_POPUP_STYLE_LIST);

		ivug_listpopup_title_set(popup, IDS_SHARE_AS);
		ivug_listpopup_item_append(popup,  NULL, IDS_SHARE_AS_VIDEO, (void *)GET_STR(IDS_SHARE_AS_VIDEO));
		ivug_listpopup_item_append(popup,  NULL, IDS_SHARE_AS_IMAGE, (void *)GET_STR(IDS_SHARE_AS_IMAGE));

		ivug_listpopup_popup_show(popup);

		pMainView->popup = popup;
		evas_object_smart_callback_add(popup, "popup,dismissed", _share_type_dismissed, (void *)pMainView);
		evas_object_smart_callback_add(popup, "popup,selected", _share_type_selected, (void *)pMainView);

		return;
	}
#endif

	_launch_share_app(pMainView, mdata->filepath);
}

void _on_mainview_save(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	if (pMainView->popup) {
		MSG_MAIN_ERROR("popup already exist");
		return;
	}

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	IV_ASSERT(mdata != NULL);

	if (mdata->filepath == NULL) {
		MSG_MAIN_ERROR("File path is NULL");
		return;
	}

	MSG_MAIN_SEC("Web image filepath %s, fileurl %s", mdata->filepath, mdata->fileurl);

#ifdef USE_DEFAULT_DOWNLOADS_FOLDER
	char *path = NULL;
	{
		path = strdup(DEFAULT_DOWNLOADS_FOLDER);
	}

	free(path);
#else
	ivug_main_view_del_hide_timer(pMainView);

	ivug_listpopup_itemlist items = ivug_listpopup_itemlist_new();

// Get local album name.

	bool ret = ivug_db_get_all_folder_list(_iter_album_list, items);
	if (ret == false) {
		MSG_SDATA_ERROR("ivug_db_get_all_folder_list failed: %d", ret);
		return;
	}

	pMainView->popup = ivug_listpopup_show(pMainView->layout, IDS_SAVE, items, _on_save_selected, pMainView);

	ivug_listpopup_itemlist_free(items);
#endif	//USE_DEFAULT_DOWNLOADS_FOLDER
	return;
}
#ifndef USE_THUMBLIST
static void _ivug_slideshow_ext_ug_destroy_cb(ui_gadget_h ug, void *priv)
{
	IV_ASSERT(priv != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)priv;

	ug_destroy(ug);
	//__gl_ext_destroy_ug(ad);
	/*Enable the focus permission of the app layout,or else the app layout can't flick the highlight*/
	elm_object_tree_focus_allow_set(pMainView->layout, EINA_TRUE);
	elm_object_focus_set(pMainView->layout, EINA_TRUE);
}

static void _ivug_slideshow_ext_ug_end_cb(ui_gadget_h ug, void *priv)
{
	IV_ASSERT(priv != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)priv;

	/*Enable the focus permission of the app layout,or else the app layout can't flick the highlight*/
	elm_object_tree_focus_allow_set(pMainView->layout, EINA_TRUE);
	elm_object_focus_set(pMainView->layout, EINA_TRUE);

	if (pMainView->bStartSlideshow == true) {
		// here , start slideshow
		//ivug_main_view_start_slideshow(pMainView, EINA_TRUE);

		Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
		Media_Data *mdata = ivug_medialist_get_data(mitem);

		ivug_ext_launch_slideshow(mdata->fileurl, mdata->index);

		pMainView->bStartSlideshow = false;
	}
}

static void _ivug_slideshow_ext_ug_result_cb(ui_gadget_h ug, app_control_h result, void *priv)
{
	IV_ASSERT(priv != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)priv;

	char *start = NULL;
	app_control_get_extra_data(result, "Result", &start);
	if (start) {
		if (!g_strcmp0(start, "StartSlideShow")) {
			/* Destory ug */
			//ug_destroy(ug);

			pMainView->bStartSlideshow = true;

		} else {
			pMainView->bStartSlideshow = false;
		}
	}
}

void _on_slideshow_selected(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)event_info;

	const char *label = ivug_listpopup_item_get_text(item);
	if (label == NULL) {
		MSG_MAIN_ERROR("label is NULL");
		evas_object_del(obj);
		ivug_main_view_set_hide_timer(pMainView);
		return;
	}

	evas_object_del(obj);

// This is not right way. but no other options.
// When X window disappeared, they capture current frame for use when activate.
// Evas renders screen when entering idle. so it is possibility for X win to capture screen before evas remove popup image.
// So in here. force render is needed.
// later this part should move to ivug-slide-show.cpp. right before X win capture.
	evas_render(evas_object_evas_get(pMainView->layout));

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (strncmp(label, IDS_ALL_PICTURES, strlen(label)) == 0) {
		//ivug_main_view_start_slideshow(pMainView, EINA_TRUE);
		ivug_ext_launch_slideshow(mdata->fileurl, mdata->index);
	} else  if (strncmp(label, IDS_SELECT_PICTURE, strlen(label)) == 0) {
		int retSvc;
		app_control_h serviceHandle;
		retSvc = app_control_create(&serviceHandle);
		if (retSvc != APP_CONTROL_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("app_control_create Failed: 0x%x", retSvc);
			return;
		}
		ivug_view_by view_by = pMainView->view_by;
		if (view_by == IVUG_VIEW_BY_ALL) {
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "all");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
		} else if (view_by == IVUG_VIEW_BY_PLACES) {
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "places");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
			/* TODO : all gps info */
		} else if (view_by == IVUG_VIEW_BY_TIMELINE) {
			TODO("it is right?");
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "timeline");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
		} else if (view_by == IVUG_VIEW_BY_FAVORITES) {
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "favourites");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
		} else if (view_by == IVUG_VIEW_BY_TAG) {
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "tags");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
			/* TODO : add tag name */
		} else {
			retSvc = app_control_add_extra_data(serviceHandle, "view-by", "albums");
			if (retSvc != APP_CONTROL_ERROR_NONE) {
				MSG_IMAGEVIEW_ERROR("app_control_add_extra_data Failed: 0x%x", retSvc);
			}
		}
		ivug_ext_launch_select_image(serviceHandle, NULL, _ivug_slideshow_ext_ug_destroy_cb, data);
		app_control_destroy(serviceHandle);
	} else  if (strncmp(label, IDS_SETTINGS, strlen(label)) == 0) {
		ivug_ext_launch_setting_gallery(_ivug_slideshow_ext_ug_result_cb, _ivug_slideshow_ext_ug_destroy_cb, _ivug_slideshow_ext_ug_end_cb, data);
	}
}

void _on_slideshow_btn_cancel(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

void _ivug_slideshow_popup_create_menu(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);
	Evas_Object* popup = NULL;
	popup = ivug_listpopup_add(pMainView->layout);
	ivug_listpopup_lang_set(popup, gGetLanguageHandle());

	ivug_listpopup_item_append(popup, NULL, IDS_ALL_PICTURES, GET_STR(IDS_ALL_PICTURES));
	ivug_listpopup_item_append(popup, NULL, IDS_SELECT_PICTURE, GET_STR(IDS_SELECT_PICTURE));
	ivug_listpopup_item_append(popup, NULL, IDS_SETTINGS, GET_STR(IDS_SETTINGS));
	ivug_listpopup_title_set(popup, IDS_SLIDE_SHOW);
//	ivug_listpopup_button_set(popup, IDS_CANCEL, NULL);
	ivug_listpopup_popup_show(popup);

	evas_object_smart_callback_add(popup, "popup,dismissed", _dismissed_cb, pMainView);
	evas_object_smart_callback_add(popup, "popup,selected", _on_slideshow_selected, pMainView);
	evas_object_smart_callback_add(popup, "popup,btn,selected", _on_slideshow_btn_cancel, pMainView);

	MSG_MAIN_HIGH("Create slideshow Popup(0x%08x)", popup);
}
#endif

static void
_on_slideshow_finished(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("_on_slideshow_finished");
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	//int ss_state = (int)event_info;

	evas_object_smart_callback_del_full(ivug_ss_object_get(pMainView->ssHandle),
	                                    "slideshow,finished", _on_slideshow_finished, pMainView);

	ivug_ss_delete(pMainView->ssHandle);
	pMainView->ssHandle = NULL;


#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		ivug_thumblist_set_edit_mode(pMainView->thumbs, EINA_FALSE);
	}
#endif

	ivug_main_view_show_menu_bar(pMainView);


	ivug_medialist_del(pMainView->temp_mlist);
	pMainView->temp_mlist = NULL;
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

		// Register callback
		evas_object_smart_callback_add(ivug_ss_object_get(pMainView->ssHandle),
		                               "slideshow,finished", _on_slideshow_finished, pMainView);

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

#ifdef IV_EXTENDED_FEATURES
static void _ivug_ext_app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)user_data;
	IV_ASSERT(pMainView != NULL);

	MSG_IMAGEVIEW_HIGH("ivug_ext_app_control_reply_cb");

// Enable menu again
	edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,enable,toolbtn", "user");

	switch (result) {
	case APP_CONTROL_RESULT_SUCCEEDED:
		MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_SUCCEEDED");
		break;
	case APP_CONTROL_RESULT_FAILED:
		MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_FAILED");
		break;
	case APP_CONTROL_RESULT_CANCELED:
		MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_CANCELED");
		break;
	default:
		MSG_IMAGEVIEW_ERROR("unhandled value %d", result);
		break;
	}

	pMainView->ext_svc = NULL;
}
#endif

void _on_mainview_edit(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	IV_ASSERT(mdata != NULL);

	if (mdata->slide_type == SLIDE_TYPE_IMAGE) {
		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,disable,toolbtn", "user");
		//ivug_ext_launch_default(mdata->filepath, APP_CONTROL_OPERATION_EDIT, NULL, NULL, NULL);
#ifdef IV_EXTENDED_FEATURES
		pMainView->ext_svc = ivug_ext_launch_image_editor(mdata->filepath, _ivug_ext_app_control_reply_cb, pMainView);
#endif
	} else  if (mdata->slide_type == SLIDE_TYPE_VIDEO) {
		ivug_ext_launch_videoeditor(mdata->filepath, NULL, NULL);
	}

}

#include <Ecore_File.h>

static void _on_add_comment_view_destroy(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("transition finished");
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	ivug_name_view_destroy(pMainView->pNameView);
	pMainView->pNameView = NULL;	// Will removed in add tag view.

	evas_object_smart_callback_del(pMainView->navi_bar, "transition,finished",
	                               _on_add_comment_view_destroy);
}

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
_ivug_ctxpopup_rename_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	_on_btn_rename_clicked(pMainView);
}

Evas_Object *ivug_listpopup_add(Evas_Object *parent)
{
	Evas_Object *obj = NULL;

// Creating dummy object because we cannot get determine popup or ctxpopup here.
	obj = elm_box_add(parent);

	ListPopup *pListPopup = (ListPopup *)calloc(1, sizeof(ListPopup));

	if (pListPopup != NULL) {
		pListPopup->parent = parent;
		pListPopup->list = NULL;
		pListPopup->obj = obj;
		pListPopup->style = IVUG_POPUP_STYLE_LIST;
		pListPopup->eStatus = IVUG_LISTPOPUP_TYPE_HIDDEN;
		pListPopup->show_count = 0;
		evas_object_data_set(obj, "LISTPOPUP", (void *)pListPopup);
	}

	MSG_MAIN_HIGH("List popup is added. obj=0x%08x", obj);

	return obj;
}

void ivug_listpopup_lang_set(Evas_Object *obj, language_handle_t hLang)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		pListPopup->hLang = hLang;
	}
}

static char *
_on_label_set(void *data, Evas_Object *obj, const char *part)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;

	MSG_LOW("_on_label_set Part %s", part);

	return strdup(GET_STR(item->caption_id)); //dump
}

static void _on_item_del(void *data, Evas_Object *obj)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)(data);

	MSG_MED("Remove item. obj=0x%08x", obj);

	if (item->caption_id)
		eina_stringshare_del(item->caption_id);
	item->caption_id = NULL;

	if (item->iconpath)
		eina_stringshare_del(item->iconpath);
	item->iconpath = NULL;

	free(item);
}

static void
_on_genlist_selected(void *data, Evas_Object *obj, void *event_info)
{
//	MSG_ASSERT(data != NULL);
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)(data);
	ListPopup *pListPopup = item->pListPopup;

	int index;

	index = item->index;
	MSG_HIGH("Selected Index = %d", index);

//	pListPopup->selected_index = index;
	pListPopup->item_selected = item;

	MSG_SEC("Genlist item selected. item=0x%08x %s", item, item->caption_id);

	elm_genlist_item_update(item->item);

	evas_object_smart_callback_call(item->obj, "popup,selected", item);

	elm_genlist_item_selected_set(item->item, EINA_FALSE);
}

static Evas_Object*
_on_content_set(void *data, Evas_Object *obj, const char *part)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;
	ListPopup *pListPopup = item->pListPopup;

	Evas_Object *radio;

	if (strcmp(part, "elm.icon.2") == 0)
	{
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, item->index);

		elm_radio_group_add(radio, pListPopup->rgroup);

		if (pListPopup->item_selected == item)
		{
			elm_radio_value_set(radio, item->index);
		}

		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return radio;
	}
	else if (strcmp(part, "elm.icon.1") == 0)
	{
		Evas_Object *icon;

		icon = elm_icon_add(obj);
		char *icon_edj_path = full_path(EDJ_PATH, "/ivug-icons.edj");
//		snprintf(edc_path, 100,"%s/res/edje/%s/ivug-icons.edj", PREFIX, PACKAGE);
		elm_image_file_set(icon, icon_edj_path, item->iconpath);
//		elm_image_file_set(icon, edc_path,item->iconpath);

		free(icon_edj_path);
		return icon;
	}

	MSG_MAIN_HIGH("_on_content_set Part %s", part);

	return NULL;
}

Ivug_ListPopup_Item *ivug_listpopup_item_append(Evas_Object *obj, const char *iconpath, const char *caption_id, void *data)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return NULL;
	}

	Ivug_ListPopup_Item *pItem = (Ivug_ListPopup_Item *)calloc(1,sizeof(Ivug_ListPopup_Item));

	if (pItem != NULL) {
		pItem->obj = obj;
	}

	if (iconpath && pItem != NULL)
	{
		pItem->iconpath = eina_stringshare_add(iconpath);
		pListPopup->bIconUsed = true;
	}

	if (caption_id && pItem != NULL)
	{
		pItem->caption_id = eina_stringshare_add(caption_id);
	}

	if (pItem != NULL) {
		pItem->bDisabled = false;
		pItem->data = data;
		pItem->pListPopup = pListPopup;
	}

	MSG_MAIN_HIGH("Item append: %s", caption_id);

	pListPopup->list = eina_list_append(pListPopup->list, pItem);

	int newIndex = eina_list_count(pListPopup->list);

// If already showed. need to update dynamically
	if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_POPUP && pItem != NULL)
	{
		static Elm_Genlist_Item_Class itc = {0,};

		itc.version = ELM_GENLIST_ITEM_CLASS_VERSION;

		if (pListPopup->style == IVUG_POPUP_STYLE_RADIO)
		{
			itc.item_style = "1text.2icon.6/popup";
			itc.func.content_get = _on_content_set;
		}
		else
		{
			itc.item_style = "1text/popup";
			itc.func.content_get = NULL;
		}

		itc.func.text_get = _on_label_set;
		itc.func.state_get = NULL;
		itc.func.del = _on_item_del;

		Elm_Object_Item *gItem;

		MSG_MAIN_HIGH("Add popup item. %s", pItem->caption_id);
		gItem = elm_genlist_item_append(pListPopup->genlist, &itc, pItem, NULL /* parent */, ELM_GENLIST_ITEM_NONE, _on_genlist_selected, pItem);
		pItem->item = gItem;
		pItem->index = newIndex;

		elm_object_item_disabled_set(gItem, pItem->bDisabled);

// When item count is increase, popup height should be increased..
		unsigned int idx;

		idx = pListPopup->show_count;

		if (pListPopup->show_count == 0)
		{
			// Default policy.
			idx = newIndex;

	//		if (count < 2) idx = 2;

	// Check landscape mode also.
			if (newIndex > 4) idx = 4;
		}
		else
		{
			idx = pListPopup->show_count;
		}

		evas_object_size_hint_min_set(pListPopup->box, GET_POPUP_WIDTH(idx) * elm_config_scale_get(), GET_POPUP_HEIGHT(idx) * elm_config_scale_get());

	}

	return pItem;
}

bool ivug_listpopup_context_set_rotate_enable(Evas_Object *obj, bool enable)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		pListPopup->bRotationEnable = enable;
	}

	return true;
}

bool ivug_listpopup_context_get_rotate_enable(Evas_Object *obj)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		return pListPopup->bRotationEnable;
	} else {
		return false;
	}
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

static void _on_ctxpopup_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	ListPopup *pListPopup = (ListPopup *)data;

	MSG_MAIN_HIGH("Remove ctx popup. pListPopup=0x%08x", pListPopup);

	if (pListPopup->title)
		eina_stringshare_del(pListPopup->title);

	pListPopup->title = NULL;

	int i = 0;

	for (i = 0; i < MAX_BUTTON; i++)
	{
		if (pListPopup->btn_caption[i])
		{
			eina_stringshare_del(pListPopup->btn_caption[i]);
		}
	}

	void *item;

	EINA_LIST_FREE(pListPopup->list, item)
	{
		_on_item_del(item, NULL);
	}

	pListPopup->list = NULL;

	if (pListPopup->popup)
	{
		evas_object_del(pListPopup->popup);
	}

	free(pListPopup);
}

static void _on_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	ListPopup *pListPopup = (ListPopup *)data;

	MSG_MAIN_HIGH("Ctx Popup Dismissed. pListPopup=0x%08x", pListPopup);

	pListPopup->state = IVUG_LISTPOPUP_STATE_DISMISSED;
	evas_object_smart_callback_call(pListPopup->obj, "popup,dismissed", NULL);
}

static void
_on_ctxpopup_selected(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;

	MSG_MAIN_WARN("Ctxpopup item selected. item=0x%08x %s", item, item->caption_id);

	evas_object_smart_callback_call(item->obj, "popup,selected", item);
}

bool ivug_listpopup_context_show(Evas_Object *obj, Evas_Object *hover, int x, int y)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return false;
	}

	if (pListPopup->popup)
	{
		MSG_MAIN_WARN("Previous popup is remained");
	}

	Evas_Object* ctxpopup = NULL;
	ctxpopup = elm_ctxpopup_add(hover);

	if (!ctxpopup)
	{
		MSG_MAIN_ERROR("Error : popup create failed.");
		return false;
	}

	pListPopup->popup = ctxpopup;

//	elm_object_style_set(ctxpopup, "more/default");	// for more

	elm_ctxpopup_hover_parent_set(ctxpopup, hover);

	{
		Evas_Object *obj = hover;
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		MSG_MAIN_HIGH("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, pass, repeat, visible, propagate);
	}

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
												ELM_CTXPOPUP_DIRECTION_UP,
												ELM_CTXPOPUP_DIRECTION_RIGHT,
												ELM_CTXPOPUP_DIRECTION_LEFT);


//	Ivug_ListPopup_Item *pItem = NULL;
	void *pItem = NULL;
	Eina_List *tmp;
	Elm_Object_Item *gItem;

	Evas_Object *icon;

	EINA_LIST_FOREACH(pListPopup->list, tmp, pItem)
	{
		MSG_MAIN_HIGH("Add popup item. %s", ((Ivug_ListPopup_Item *)pItem)->caption_id);
		char edc_path[100];

		if (((Ivug_ListPopup_Item *)pItem)->iconpath)
		{
			icon = elm_icon_add(ctxpopup);

//			snprintf(edc_path, 100,"%s/res/edje/%s/ivug-icons.edj", PREFIX,PACKAGE);

			elm_image_file_set(icon, full_path(EDJ_PATH, "/ivug-icons.edj"), ((Ivug_ListPopup_Item *)pItem)->iconpath);
			elm_image_file_set(icon, edc_path,((Ivug_ListPopup_Item *)pItem)->iconpath);
		}
		else if (pListPopup->bIconUsed == true)		// For icon align
		{
			icon = elm_icon_add(ctxpopup);
//			snprintf(edc_path, 100, "%s/res/edje/%s/ivug-icons.edj", PREFIX, PACKAGE);

			elm_image_file_set(icon, full_path(EDJ_PATH, "/ivug-icons.edj"), "icon.mainmenu.transparent");
//			elm_image_file_set(icon, edc_path, "icon.mainmenu.transparent");
		}
		else
		{

			icon = NULL;
		}

		gItem = elm_ctxpopup_item_append(ctxpopup, GET_STR(((Ivug_ListPopup_Item *)pItem)->caption_id), icon, _on_ctxpopup_selected, ((Ivug_ListPopup_Item *)pItem));
		((Ivug_ListPopup_Item *)pItem)->pListPopup = pListPopup;
		((Ivug_ListPopup_Item *)pItem)->item = gItem;

		elm_object_item_disabled_set(gItem, ((Ivug_ListPopup_Item *)pItem)->bDisabled);
	}

	evas_object_event_callback_add(pListPopup->obj, EVAS_CALLBACK_DEL, _on_ctxpopup_deleted, pListPopup);

	evas_object_smart_callback_add(ctxpopup, "dismissed", _on_dismissed_cb, pListPopup);

	if (pListPopup->bRotationEnable)
		elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	evas_object_move(ctxpopup, x, y);

	MSG_SEC("Show Context Popup(%d,%d)", x, y);
	pListPopup->eStatus = IVUG_LISTPOPUP_TYPE_CTX_POPUP;

	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, pListPopup);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, _on_btn_back_clicked, pListPopup);		// When pressed backbutton 2 time, closed ctx popup

	evas_object_show(ctxpopup);

	return true;
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
		// If menu is visible, do not hide menu during popup is displaying
		ivug_main_view_del_hide_timer(pMainView);
	}

	Evas_Object *ctxpopup = elm_ctxpopup_add(gGetCurrentWindow());
	elm_object_style_set(ctxpopup, "more/default");

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN,
										ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	pMainView->ctx_popup = ctxpopup;

	if (pMainView->mode == IVUG_MODE_EMAIL) {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_FILE_DOWNLOAD), NULL, _ivug_ctxpopup_download_sel_cb, pMainView);

	} else if (pMainView->view_by != IVUG_VIEW_BY_FAVORITES) {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_SLIDE_SHOW), NULL, _ivug_ctxpopup_slideshow_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_DELETE_IMAGE), NULL, _ivug_ctxpopup_delete_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_RENAME_IMAGE), NULL, _ivug_ctxpopup_rename_sel_cb, pMainView);
	} else {
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_SLIDE_SHOW), NULL, _ivug_ctxpopup_slideshow_sel_cb, pMainView);
		elm_ctxpopup_item_append(ctxpopup, GET_STR(IDS_RENAME_IMAGE), NULL, _ivug_ctxpopup_rename_sel_cb, pMainView);
	}
	elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	_ivug_move_more_ctxpopup(gGetCurrentWindow(), ctxpopup);
	evas_object_show(ctxpopup);

	if (ctxpopup) {
		_ivug_ctxpopup_add_callbacks(pMainView, ctxpopup);
	}
}

