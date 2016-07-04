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

#include <efl_extension.h>
#include <media_content.h>

#include "ivug-common.h"
#include "ivug-parameter.h"
#include "ivug-main-view.h"
#include "ivug-main-view-toolbar.h"
#include "ivug-main-view-menu.h"
#include "ivug-photocam.h"
#include "ivug-filter.h"
#include "ivug-ext-ug.h"
#include "ivug-db.h"
#include "ivug-util.h"
#include "ivug-media.h"
#include "ivug-popup.h"
#include "ivug-name-view.h"
#include "ivug-slideshow.h"
#include "ivug-thumblist.h"
#include "ivug-ext-ug.h"
#include "ivug-decolayer.h"
#include "ivug-comment.h"
#include "ivug-language-mgr.h"
#include "ivug-file-info.h"
#include "ivug-main-view-priv.h"
#include "ivug-popup.h"

extern "C" int app_control_send_terminate_request(app_control_h service);


#define LONGTAP_TIMEOUT	(2.0) // 2secs

#define ABS(x) ((x) < 0 ? -(x) : (x))

typedef enum {
	LONGTAP_ENDED,
	LONGTAP_CANCELED,
} longtap_state;

#undef LOG_LVL
#define LOG_LVL (DBG_MSG_LVL_HIGH | DBG_MSG_LVL_DEBUG)

#undef LOG_CAT
#define LOG_CAT "IV-MAIN-VIEW"

// EDJE
#define IVUG_MAIN_EDJ 	full_path(EDJ_PATH, "/ivug-main.edj")
///////////////////////////////////////////////////////////////////////////////////////////////////
//
//
static bool _destory_slideshow_and_ug(Ivug_MainView *pMainView, bool bMmc_out);
static void _on_slideshow_finished(void *data, Evas_Object *obj, void *event_info);

#ifdef USE_THUMBLIST
static void _on_receive_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _on_receive_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _thumb_realized_cb(void *data, Evas_Object *obj, void *event_info);
#endif

static void _update_check_title(Ivug_MainView *pMainView)
{
	char buf[64] = {0,};
	snprintf(buf, 64, GET_STR(IDS_PD_SELECTED), pMainView->total_selected);
	elm_layout_text_set(pMainView->select_bar, "elm.text.title", buf);
}

static void
_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	Eina_Bool state = elm_check_state_get(obj);
	MSG_MAIN_HIGH("Check %d", state);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	bool isSupported = ivug_is_supported_file_type(mdata->filepath);

	/* Return only if caller is "ug-gallery-efl" */
	if ((pMainView->media_type != IVUG_MEDIA_TYPE_ALL) && (isSupported == false)) {
		elm_check_state_set(obj, !state);
		ivug_notification_create(IDS_UNKOWN_FORMAT);
		MSG_MAIN_HIGH("Unsupported File");
		return;
	}

	if (state == true) {
		struct stat stFileInfo;
		stat(mdata->fileurl, &stFileInfo);

		if (pMainView->total_selected < pMainView->max_count &&
				(pMainView->select_size + stFileInfo.st_size) <= pMainView->limit_size) {

			pMainView->selected_path_list = eina_list_append(pMainView->selected_path_list, mdata->filepath);
			pMainView->total_selected++;
			evas_object_color_set(obj, 255, 255, 255, 255);
			elm_check_state_set(obj, EINA_TRUE);
		} else {
			//[ToDo] Show the popup for Max count or size
			evas_object_color_set(obj, 128, 138, 137, 255);
			elm_check_state_set(obj, EINA_FALSE);
		}
	} else {
		pMainView->selected_path_list = eina_list_remove(pMainView->selected_path_list, mdata->filepath);
		if ((pMainView->total_selected - 1) >= 0) {
			pMainView->total_selected--;
		}
		evas_object_color_set(obj, 128, 138, 137, 255);
		elm_check_state_set(obj, EINA_FALSE);
	}
	_update_check_title(pMainView);
}


void _update_favorite(Ivug_MainView *pMainView)
{
	bool isFavorite = false;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (mdata == NULL) {
		MSG_MAIN_ERROR("slider data is NULL");
		return;
	}

	if (pMainView->bShowMenu == true) {
		elm_object_disabled_set(pMainView->btn_favorite, EINA_FALSE);
		Evas_Object *btn = elm_object_part_content_get(pMainView->btn_favorite, "elm.swallow.content");
		elm_object_disabled_set(btn, EINA_FALSE);
		edje_object_signal_emit(_EDJ(btn), "image,normal", "prog");
	}

	ivug_mediadata_get_favorite(mdata, &isFavorite);
	Evas_Object *btn = NULL;
	if (isFavorite) {
		btn = elm_object_part_content_get(pMainView->btn_favorite, "elm.swallow.content");
		MSG_MAIN_HIGH("image,on,effect");
		edje_object_signal_emit(_EDJ(btn), "image,on,effect", "prog");
	} else {
		btn = elm_object_part_content_get(pMainView->btn_favorite, "elm.swallow.content");
		MSG_MAIN_HIGH("image,off,effect");
		edje_object_signal_emit(_EDJ(btn), "image,off,effect", "prog");
	}
}

static void _on_btn_favorite_cb(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("Btn Favorite clicked");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	if (mitem == NULL) {
		MSG_MAIN_HIGH("current item was removed");
		return;
	}

	ivug_main_view_del_hide_timer(pMainView);

	Media_Data *mdata = ivug_medialist_get_data(mitem);

	bool isFavorite = false;
	ivug_mediadata_get_favorite(mdata, &isFavorite);

	isFavorite = !isFavorite;

	if (ivug_mediadata_set_favorite(mdata, isFavorite) == false) {
		MSG_MAIN_ERROR("Error!. Set favorite for ID=%s", uuid_getchar(mdata->mediaID));
	} else {
		if (isFavorite) {
			Evas_Object *btn = elm_object_part_content_get(pMainView->btn_favorite, "elm.swallow.content");
			MSG_MAIN_HIGH("image,on,effect");
			edje_object_signal_emit(_EDJ(btn), "image,on,effect", "prog");
		} else {
			Evas_Object *btn = elm_object_part_content_get(pMainView->btn_favorite, "elm.swallow.content");
			MSG_MAIN_HIGH("image,off,effect");
			edje_object_signal_emit(_EDJ(btn), "image,off,effect", "prog");

			if (pMainView->view_by == IVUG_VIEW_BY_FAVORITES) {
				ivug_medialist_delete_item(pMainView->mList, mitem, false);

				mitem = ivug_medialist_get_current_item(pMainView->mList);
				if (mitem == NULL) {
					MSG_MAIN_HIGH("Current item is NULL");
					ivug_main_view_destroy(pMainView);
					elm_exit();
					return;
				}
				ivug_main_view_start(pMainView, NULL);
			}
		}
	}

	ivug_main_view_set_hide_timer(pMainView);
}
/*
Evas_Object *create_layout(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);
//	MSG_ASSERT(parent != NULL);

	Evas_Object *layout;
	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_MAIN_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_MAIN_ERROR("Layout file set failed! %s in %s", group, edj);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

//	evas_object_show(layout);
	return layout;
}*/

static Evas_Object* _create_favorite_button(Evas_Object *parent)
{
	Evas_Object *focusButton = elm_button_add(parent);
	elm_object_style_set(focusButton, "focus");
	char *edj_path = full_path(EDJ_PATH, "/ivug-button_new.edj");

	Evas_Object *btnlayout = create_layout(parent, edj_path, "ivug.btn.favorite");
	edje_object_signal_emit(_EDJ(btnlayout), "image,dim", "prog");
	elm_object_part_content_set(focusButton, "elm.swallow.content", btnlayout);
	evas_object_show(focusButton);

	free(edj_path);
	return focusButton;
}

static void
_on_longpress_popup_selected(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

// Destroy copy popup
	pMainView->longpress_popup = NULL;		// object is removed automatically

	int *response = (int *)event_info;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (mdata == NULL) {
		MSG_MAIN_ERROR("slider data is NULL");
		return;
	}

	if (*response == LPPOPUP_RESPONSE_COPY) {
		//get current file path.
		int len = 0;
		// This Will add to the article
		char buf[IVUG_MAX_FILE_PATH_LEN] = {0,};
		{
			len = strlen(mdata->filepath) + strlen("file://") + 1;
			snprintf(buf, IVUG_MAX_FILE_PATH_LEN, "file://%s", mdata->filepath);
		}

		if (len < IVUG_MAX_FILE_PATH_LEN) {
			MSG_MAIN_HIGH("CNP : Buf=\"%s\" len=%d", buf, strlen(buf));

			Eina_Bool bRet;

			bRet = elm_cnp_selection_set(pMainView->layout, ELM_SEL_TYPE_CLIPBOARD, ELM_SEL_FORMAT_IMAGE, buf, strlen(buf));

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
}

static Eina_Bool _on_btn_back_clicked(void *data, Elm_Object_Item *it)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_WARN("Back key from mainview(0x%08x) pressed", pMainView);

	if (pMainView->mode == IVUG_MODE_SELECT) {
		app_control_h service;

		Eina_List *l = NULL;
		void *data = NULL;
		char **files = NULL;
		int i = 0 ;
		int count_selected = eina_list_count(pMainView->selected_path_list);
		files = (char **)malloc(sizeof(char *) * count_selected);
		if (!files) {
			MSG_MAIN_WARN("failed to allocate memory");
			return EINA_FALSE;
		}
		if (pMainView->selected_path_list) {
			EINA_LIST_FOREACH(pMainView->selected_path_list, l, data) {
				files[i] = strdup((char *)data);
				i++;
			}
		}
		app_control_create(&service);
		if (pMainView->view_by == IVUG_VIEW_BY_FAVORITES) {
			app_control_add_extra_data_array(service, "Selected index fav", (const char **)files, count_selected);
		} else {
			app_control_add_extra_data_array(service, "Selected index", (const char **)files, count_selected);
		}
		app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(service);
		i--;
		while(i >= 0) {
			free(files[i--]);
		}
		free(files);
	}

	if (pMainView->bPreventBackKey == true) {
		MSG_MAIN_WARN("Back key is blocked");
		return EINA_FALSE;
	}

	if (pMainView->transcoding_thread) {
		ecore_thread_cancel(pMainView->transcoding_thread);
		pMainView->transcoding_thread = NULL;
	}

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		if (ivug_thumblist_get_edit_mode(pMainView->thumbs) == EINA_TRUE) {
			ivug_thumblist_set_edit_mode(pMainView->thumbs, EINA_FALSE);
			return EINA_FALSE;
		}
	}
#endif

	if (pMainView->bTestMode) {	// Test mode.
		MSG_MAIN_HIGH("Test Mode. Removing all views. pMainView=0x%08x", pMainView);
		_on_remove_main_view_ui(pMainView);

		DESTROY_ME();

		return EINA_TRUE;
	}

	if (pMainView->mode != IVUG_MODE_SAVE) {
		if (pMainView->bStandAlone == true) {
			MSG_MAIN_HIGH("appsvc hide");

			elm_win_lower(gGetCurrentWindow());

			return EINA_FALSE;
		}

		_on_remove_main_view_ui(pMainView);

	} else {	// Mode is save view
		MSG_MAIN_HIGH("Select Cancel");

		_on_remove_main_view_ui(pMainView);

		//send result to caller
		int ret = 0;
		app_control_h service = NULL;
		ret = app_control_create(&service);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_MAIN_HIGH("app_control_create failed");
			return EINA_FALSE;
		}

		ret = app_control_add_extra_data(service, "Result", "Cancel");
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_MAIN_HIGH("app_control_add_extra_data()... [0x%x]", ret);
			app_control_destroy(service);
			return EINA_FALSE;
		}
		app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(service);
	}

	DESTROY_ME();

//	ivug_set_indicator_overlap_mode(false);

	return EINA_TRUE;

}

static void _on_cancel_btn_clicked(void *data, Evas_Object *obj, const char * s, const char *dest)
{
	MSG_IMAGEVIEW_ERROR("cancel btn clicked");
	_on_btn_back_clicked(data, NULL);
}

static void _on_save_btn_clicked(void *data, Evas_Object *obj, const char * s, const char *dest)
{
	MSG_IMAGEVIEW_ERROR("save btn clicked");
	int ret = 0;
	app_control_h service = NULL;
	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_MAIN_HIGH("app_control_create failed");
		return;
	}

	ret = app_control_add_extra_data(service, "Result", "Save");
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_MAIN_HIGH("app_control_add_extra_data()... [0x%x]", ret);
		app_control_destroy(service);
		return;
	}
	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);
	DESTROY_ME();
	elm_exit();
}

static Eina_Bool _on_key_down(void *user_data, int type, void *event)
{
	if (!user_data) {
		MSG_IMAGEVIEW_ERROR("user data is NULL");
		return ECORE_CALLBACK_PASS_ON;
	}

	Ivug_MainView *pMainView = (Ivug_MainView *)user_data;

	Ecore_Event_Key *key_event = (Ecore_Event_Key *) event;

	if (!strcmp(key_event->keyname, "XF86Stop")) {
		MSG_IMAGEVIEW_HIGH("Back(End) key");
		if (pMainView->ssHandle) {
			ivug_ss_stop(pMainView->ssHandle);
		}
	} else if (!strcmp(key_event->keyname, "XF86Home")) {
		MSG_IMAGEVIEW_HIGH("Home key");
	} else if (!strcmp(key_event->keyname, "XF86PowerOff")) {
		MSG_IMAGEVIEW_HIGH("Power key");
		if (pMainView->ssHandle) {
			ivug_ss_stop(pMainView->ssHandle);
		}
	} else if (!strcmp(key_event->keyname, "XF86Menu")) {
		MSG_IMAGEVIEW_HIGH("Menu key");
	}

	MSG_IMAGEVIEW_LOW("Key down : %s", key_event->keyname);

	return ECORE_CALLBACK_PASS_ON;
}

static Eina_Bool _on_exit_timer_expired(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	pMainView->exit_timer = NULL;

	DESTROY_ME();

	return ECORE_CALLBACK_CANCEL;
}

bool ivug_listpopup_context_move(Evas_Object *obj, int x, int y)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		evas_object_move(pListPopup->popup, x, y);
	}

	return true;
}

static void _on_layout_resize(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = static_cast<Ivug_MainView *>(data);
	IV_ASSERT(pMainView != NULL);

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);
	/**
	*  during slideshow, after rotation, resize cb of slideshow obj cannot be invoked,
	    so invoke it manually
	*/
	if (pMainView->ssHandle) {
		ivug_ss_resize(pMainView->ssHandle);
		// Need return????
	}

	int rot = gGetRotationDegree();
	MSG_MAIN_HIGH("MainView resized geomtery XYWH(%d,%d,%d,%d) Rotate=%d", x, y, w, h, rot);

	if (pMainView->pSliderNew) {
		MSG_HIGH("Inslide sn layout ");
		ivug_slider_new_change_view_size(pMainView->pSliderNew, w, h);
		Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
		if (rot == 270 || rot == 90) {
			MSG_HIGH("Rotation portrait");
			if (pMainView->mode == IVUG_MODE_SELECT) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_select_view_landscape", "glsurface");
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_landscape", "glsurface");
			}
		} else {
			MSG_HIGH("Rotation Landscape");
			if (pMainView->mode == IVUG_MODE_SELECT) {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_select_view_portrait", "glsurface");
			} else {
				edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_portrait", "glsurface");
			}
		}
	}
}

static bool _destory_slideshow_and_ug(Ivug_MainView *pMainView,
                                      bool bMmc_out)
{
	IV_ASSERT(pMainView != NULL);
	evas_object_smart_callback_del_full(ivug_ss_object_get(pMainView->ssHandle),
	                                    "slideshow,finished", _on_slideshow_finished, pMainView);

//	ivug_allow_lcd_off();
	/*from gallery ablum*/
	if (bMmc_out) {
		MSG_MAIN_HIGH("image viewer end cause slide show ended");
		ivug_ss_delete(pMainView->ssHandle);
		pMainView->ssHandle = NULL;

		DESTROY_ME();
		return true;
	}

	return false;
}


void _ivug_main_on_mmc_state_changed(void *data)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	if (NULL == pMainView) {
		return;
	}

	if (pMainView->ssHandle) {
		ivug_ss_set_stop(pMainView->ssHandle);
		_destory_slideshow_and_ug(pMainView, true);
	} else {
		if (pMainView->exit_timer == NULL) {
			pMainView->exit_timer = ecore_timer_add(0.2, _on_exit_timer_expired, data);
		}
	}
}

void _update_main_view(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (mdata == NULL) {
		MSG_MAIN_ERROR("mdata is NULL");
		return;
	}

	if (pMainView->bShowMenu == true) {
		_update_toolbuttons(pMainView);
	}

	if (pMainView->btn_favorite) {
		_update_favorite(pMainView);
	}

	return;
}
/*
static void
_on_slider_playburst_icon_clicked(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("Play Burst icon Clicked");
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	_on_play_continous_shot(pMainView);
}
*/
static void
_on_slider_playvideo_icon_clicked(void *data, Evas_Object *obj, const char *source, const char *emission)
{
	MSG_MAIN_HIGH("Play Video icon Clicked");
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (mdata == NULL) {
		MSG_MAIN_ERROR("slide data is NULL");
		return;
	}

	{
		MSG_MAIN_HIGH("Launching video player");
		ivug_ext_launch_videoplayer(mdata->filepath);
		//ivug_ext_launch_default(mdata->filepath, APP_CONTROL_OPERATION_VIEW, NULL, NULL, NULL);
	}

}

void
on_slider_clicked(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	MSG_MAIN_HIGH("On Slider Clicked. pMainView=0x%08x", pMainView);

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		if (ivug_thumblist_get_edit_mode(pMainView->thumbs) == EINA_TRUE) {
			MSG_WARN("Ignore click when Edit mode");
			return;
		}
	}
#endif
	if (pMainView->mode != IVUG_MODE_SELECT && pMainView->mode != IVUG_MODE_EMAIL) {
		if (pMainView->bShowMenu) {
			ivug_main_view_hide_menu_bar(pMainView);
		} else {
			ivug_main_view_show_menu_bar(pMainView);
		}
	}

}


static Eina_Bool _ivug_long_press_timer_expired(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	MSG_SETAS_HIGH("long press timer expired");

	pMainView->popup_timer = NULL;

	if (pMainView->bLongpressEnd == false) {	// still press
		if (pMainView->longpress_popup) {
			IVUG_DEBUG_WARNING("copy popup remove");
			evas_object_del(pMainView->longpress_popup);
			pMainView->longpress_popup = NULL;
		}
	}


	return ECORE_CALLBACK_CANCEL;
}

static void
_on_slider_long_press_start(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
	ivug_ret_if(!data || !p);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	int x = p->x;
	int y = p->y;

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if (mdata == NULL) {
		MSG_MAIN_ERROR("mdata is NULL");
		return;
	}

	/*if (ivug_mediadata_get_file_state(mdata) != DATA_STATE_LOADED)
	{
		MSG_MAIN_ERROR("Long pressed. but state is not ready");
		return;
	}*/

	if (mdata->slide_type != SLIDE_TYPE_IMAGE) {
		MSG_MAIN_ERROR("Long pressed. but it is not image");
		return;
	}

	bool bUseExtMenu = !(ivug_is_agif(pMainView,mdata->filepath));	//agif cannot use manual tag
	if (pMainView->mode == IVUG_MODE_DISPLAY) {
		bUseExtMenu = false;	//disable manualtag at display mode
	}

	if (pMainView->longpress_popup) {
		evas_object_del(pMainView->longpress_popup);
	}

	MSG_MAIN_HIGH("LongPressed. Show popup XY(%d,%d)", x, y);

	pMainView->longpress_popup = ivug_longpress_popup_show(pMainView->layout, x, y,
	                             bUseExtMenu, _on_longpress_popup_selected, pMainView);
	if (pMainView->longpress_popup == NULL) {
		IVUG_DEBUG_WARNING("long press popup create failed");
		return ;
	}

	pMainView->bLongpressEnd = false;

	if (pMainView->popup_timer) {
		ecore_timer_del(pMainView->popup_timer);
	}
	pMainView->popup_timer = ecore_timer_add(LONGTAP_TIMEOUT, _ivug_long_press_timer_expired, pMainView);
}

static void
_on_slider_long_press_end(void *data, Evas_Object *obj, void *event_info)
{
	//Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
	//ivug_ret_if(!data||!p);
	ivug_ret_if(!data);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	pMainView->bLongpressEnd = true;

	if (pMainView->popup_timer) {
		ecore_timer_del(pMainView->popup_timer);
		pMainView->popup_timer = NULL;
	}
	int *value = (int *)event_info ;
	if (*value == LONGTAP_CANCELED) {
		if (pMainView->longpress_popup) {
			IVUG_DEBUG_WARNING("copy popup remove");
			evas_object_del(pMainView->longpress_popup);
			pMainView->longpress_popup = NULL;
		}
	}
}

#ifdef USE_THUMBLIST
static void _on_receive_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	if (NULL == pMainView) {
		return;
	}

	/*if(pMainView->thumbs && ivug_thumblist_get_edit_mode(pMainView->thumbs) == EINA_TRUE)
	{
		return;
	}*/

	ivug_main_view_del_hide_timer(pMainView);

	MSG_MAIN_HIGH("Event layer clicked : %s %s Layer=%d", evas_object_name_get(obj), evas_object_type_get(obj), evas_object_layer_get(obj));
}

static void _on_receive_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	if (NULL == pMainView) {
		return;
	}

	/*if(pMainView->thumbs && ivug_thumblist_get_edit_mode(pMainView->thumbs) == EINA_TRUE)
	{
		return;
	}*/

	ivug_main_view_set_hide_timer(pMainView);
}
#endif

static void
_on_menu_state_changed(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Ivug_MainView *pMainView = static_cast<Ivug_MainView *>(data);
	if (NULL == pMainView) {
		return;
	}

	MSG_MAIN_HIGH("Receive %s %s", emission, source);

	if (strncmp(emission, "menu,show,finished", strlen(emission)) == 0) {
		//enable menu
	} else {
		//disable menu
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// External APIs
//

static char * _ivug_get_folder_name(char *filepath)
{
	char *folder = ivug_get_directory(filepath);
	if (folder == NULL) {
		return NULL;
	}

	char *name = NULL;
	char *result = NULL;

	name = strrchr(folder, '/');
	if ((name != NULL) && ((name + 1) != NULL)) {
		result = strdup(name + 1);
		free(folder);
		return result;
	}

	free(folder);

	return NULL;
}

static void
_on_slideshow_finished(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("_on_slideshow_finished");
	IV_ASSERT(data != NULL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	bool bDestoryed = false;
	bDestoryed = _destory_slideshow_and_ug(pMainView, false);
	if (bDestoryed) {
		return;
	}

	Media_Item * item = NULL;
	pMainView->isSliding = false;


	item = ivug_ss_item_get(pMainView->ssHandle);
	ivug_ss_delete(pMainView->ssHandle);
	pMainView->ssHandle = NULL;

	if (gGetDestroying() == true) {
		MSG_MAIN_WARN("ug is destroying");
		return;
	}

	if (item) {
		ivug_medialist_set_current_item(pMainView->mList, item);
		ivug_main_view_start(pMainView, NULL);
	}

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		Image_Object *img = NULL;
		if (item) {
			img = ivug_thumblist_find_item_by_data(pMainView->thumbs, item);
		}

		if (img == NULL) {
			MSG_MAIN_ERROR("Cannot find item");
		} else {
			MSG_MAIN_HIGH("pMainView->bSetThmByUser : %d", pMainView->bSetThmByUser);

			pMainView->bSetThmByUser = true;
			ivug_thumblist_select_item(pMainView->thumbs, img);
		}
	}
#endif

	ivug_slider_new_reload(pMainView->pSliderNew);
	ivug_main_view_show_menu_bar(pMainView);

}

#ifdef USE_THUMBLIST
static void _ivug_main_view_thumbnail_cb(media_handle media, const char *path, void *data)
{
	MSG_SDATA_ERROR("_ivug_main_view_thumbnail_cb, path = %s", path);

	Media_Data *mdata = (Media_Data *)data;

	mdata->thumbnail_path = strdup(path);

	ivug_db_cancel_thumbnail(mdata->thumb_handle);

	mdata->thumb_handle = NULL;
}

static void
_thumb_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	Media_Item *mitem = (Media_Item *)event_info;
	if (mitem == NULL) {
		MSG_SDATA_ERROR("data already removed");
		return;
	}
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	char *edj_default_path = DEFAULT_THUMBNAIL_PATH;

	if (strcmp(mdata->thumbnail_path, edj_default_path) == 0) {
		if (mdata->thumb_handle == NULL) {
			mdata->thumb_handle = ivug_db_create_thumbnail(mdata->m_handle, _ivug_main_view_thumbnail_cb, (void *)mdata);
		}
	}
	free(edj_default_path);
}

static void
_edit_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	if (ivug_thumblist_checked_item_is(pMainView->thumbs) == EINA_FALSE) {
		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,hide,toolbtn", "user");
	} else {
		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,show,toolbtn1", "user");
	}
}

static void
_thumb_mode_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	const char *mode = (const char *)event_info;

	if (strcmp(mode, "edit") == 0) {
		ivug_main_view_del_hide_timer(pMainView);

		if (ivug_thumblist_checked_item_is(pMainView->thumbs) == EINA_FALSE) {
			edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,hide,toolbtn", "user");
		}
	} else if (strcmp(mode, "list") == 0) {
		ivug_main_view_set_hide_timer(pMainView);

		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,show,toolbtn", "user");
	} else {
		MSG_MAIN_WARN("Unknown Mode. %s", mode);
	}

}
#endif

static Eina_Bool _on_hide_timer_expired(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	pMainView->hide_timer = NULL;

	ivug_main_view_hide_menu_bar(pMainView);

	return ECORE_CALLBACK_CANCEL;
}

static inline void
_naviframe_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("Popping Navivar");

	elm_naviframe_item_pop(obj);
}


Ivug_MainView *
ivug_main_view_create(Evas_Object* parent, ivug_parameter *param)
{
	IV_ASSERT(parent != NULL);
	IV_ASSERT(param != NULL);

	MSG_MAIN_HIGH("Creating main view.");

	PERF_CHECK_BEGIN(LVL2, "Create layout");

//create main view layout
	Ivug_MainView *pMainView = (Ivug_MainView *)calloc(1, sizeof(Ivug_MainView));
	IV_ASSERT(pMainView != NULL);

// Set default value
	pMainView->parent = parent;
	pMainView->mode = param->mode;
	pMainView->view_by = param->view_by;
	pMainView->bStandAlone = param->bStandalone;
	pMainView->bShowMenu = true;
	pMainView->hide_count = -1;
	pMainView->media_type = param->media_type;

	Evas_Object *layout = create_layout(parent, IVUG_MAIN_EDJ, "mainview,selected");
	if (layout == NULL) {	//if failed
		MSG_MAIN_ERROR("main layout create failed");
		free(pMainView);
		return NULL;
	}
	pMainView->layout = layout;
	evas_object_name_set(pMainView->layout, "Main Layout");

	PERF_CHECK_END(LVL2, "Create layout");

	edje_object_signal_callback_add(_EDJ(pMainView->layout),
	                                "menu,hide,finished",
	                                "edc",
	                                _on_menu_state_changed,
	                                (void *)pMainView);

	edje_object_signal_callback_add(_EDJ(pMainView->layout),
	                                "menu,show,finished",
	                                "edc",
	                                _on_menu_state_changed,
	                                (void *)pMainView);

// Navigation bar
	PERF_CHECK_BEGIN(LVL2, "elm_naviframe_add");

	pMainView->navi_bar = elm_naviframe_add(layout);
	if (pMainView->navi_bar == NULL) {
		MSG_MAIN_ERROR("navigation bar failed");
		ivug_main_view_destroy(pMainView);
		return NULL;
	}
	evas_object_name_set(pMainView->navi_bar, "Main naviframe");

	elm_naviframe_prev_btn_auto_pushed_set(pMainView->navi_bar, EINA_FALSE);

	eext_object_event_callback_add(pMainView->navi_bar, EEXT_CALLBACK_BACK, _naviframe_back_cb, pMainView);

	if (pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_SELECT && pMainView->mode != IVUG_MODE_HIDDEN) {
		eext_object_event_callback_add(pMainView->navi_bar, EEXT_CALLBACK_MORE, on_btn_more_clicked, pMainView);
	}

// Layout life cycle is controlled by application explictily.
// Because for ex, "Add tag" view is pushed, then previous view is removed. so app control life cycle directly.  --> Not true
//	elm_naviframe_content_preserve_on_pop_set(pMainView->navi_bar, EINA_TRUE);

	if (pMainView->mode != IVUG_MODE_SETAS) {
#if 1
#ifdef USE_CUSTOM_STYLE
		elm_object_theme_set(pMainView->navi_bar, gGetSystemTheme());
#endif
		const char *profile = elm_config_profile_get();
		if (!strcmp(profile, "mobile")) {
			elm_object_style_set(pMainView->navi_bar, "ivug-main/default");
		} else if (!strcmp(profile, "desktop")) {
			elm_object_style_set(pMainView->navi_bar, "ivug-main/default");
		} else {
			MSG_MAIN_ERROR("Unknown profile : %s", profile);
		}
#endif
	}
	elm_object_part_content_set(layout, "mainview.navibar", pMainView->navi_bar);	//swallow

	PERF_CHECK_END(LVL2, "elm_naviframe_add");

	PERF_CHECK_BEGIN(LVL2, "elm_layout_add");

	char *main_edj_path = IVUG_MAIN_EDJ;
	pMainView->lyContent = create_layout(layout, main_edj_path, "navi_content");
	free(main_edj_path);
	if (pMainView->lyContent == NULL) {
		IVUG_DEBUG_WARNING("layout create failed");
		ivug_main_view_destroy(pMainView);
		return NULL;
	}

	evas_object_name_set(pMainView->lyContent, "Navi content");
	PERF_CHECK_END(LVL2, "elm_layout_add");

#ifdef BACK_BTN
	PERF_CHECK_BEGIN(LVL2, "ivug_button_add");
	pMainView->back_btn = elm_button_add(pMainView->navi_bar);
	elm_object_style_set(pMainView->back_btn, "naviframe/end_btn/default");
	PERF_CHECK_END(LVL2, "ivug_button_add");
#endif

	PERF_CHECK_BEGIN(LVL2, "Init slider");

	pMainView->pSliderNew = ivug_slider_new_init(pMainView->layout, pMainView);
	if (pMainView->pSliderNew == NULL) {
		MSG_MAIN_ERROR("Unable to get Slider Layout");
		return NULL;
	}

	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
	if (pMainView->mode == IVUG_MODE_SELECT) {
		int rot = gGetRotationDegree();
		if (rot == 270 || rot == 90) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_select_view_landscape", "glsurface");
		} else {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_select_view_portrait", "glsurface");
		}
	}

	elm_object_part_content_set(pMainView->lyContent, "mainview.slider", sn_layout);	//swallow

	PERF_CHECK_END(LVL2, "Init slider");

	PERF_CHECK_BEGIN(LVL2, "elm_naviframe_item_push");
	if (pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_SELECT) {
		pMainView->navi_it = elm_naviframe_item_push(pMainView->navi_bar, NULL, NULL, NULL, pMainView->lyContent, NULL);

// Invisble title.
		elm_naviframe_item_title_enabled_set(pMainView->navi_it, EINA_FALSE, EINA_FALSE);
		elm_object_item_signal_emit(pMainView->navi_it, "elm,state,toolbar,instant_close", "");
	} else {
		pMainView->navi_it = elm_naviframe_item_push(pMainView->navi_bar, "Select", NULL, NULL, pMainView->lyContent, NULL);
		elm_naviframe_item_title_enabled_set(pMainView->navi_it, EINA_TRUE, EINA_FALSE);

		Evas_Object *pCancelbtn = elm_button_add(pMainView->navi_bar);
		elm_object_style_set(pCancelbtn, "naviframe/title_left");
		elm_object_domain_translatable_part_text_set(pCancelbtn, "default", textdomain(NULL), GET_STR(IDS_CANCEL_CAPS));
		elm_object_signal_callback_add(pCancelbtn, "elm,action,click", "", _on_cancel_btn_clicked, (void *)pMainView);
		elm_object_item_part_content_set(pMainView->navi_it, "title_left_btn", pCancelbtn);
		evas_object_show(pCancelbtn);

		Evas_Object *pSavebtn = elm_button_add(pMainView->navi_bar);
		elm_object_style_set(pSavebtn, "naviframe/title_right");
		elm_object_domain_translatable_part_text_set(pSavebtn, "default", textdomain(NULL), GET_STR(IDS_DONE_CAPS));
		elm_object_signal_callback_add(pSavebtn, "elm,action,click", "", _on_save_btn_clicked, (void *)pMainView);
		elm_object_item_part_content_set(pMainView->navi_it, "title_right_btn", pSavebtn);
		evas_object_show(pSavebtn);
	}

	elm_naviframe_item_pop_cb_set(pMainView->navi_it, _on_btn_back_clicked, pMainView); //pop�� ���� �ݹ� ���.

	PERF_CHECK_END(LVL2, "elm_naviframe_item_push");
	PERF_CHECK_BEGIN(LVL2, "add event handler");

//	PERF_CHECK_BEGIN(LVL2, "Register callback");
// 3ms
	if (pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_HIDDEN) {
		elm_object_signal_callback_add(sn_layout, "play", "elm", _on_slider_playvideo_icon_clicked, pMainView);
//		evas_object_smart_callback_add(sn_layout, "slider,playburst", _on_slider_playburst_icon_clicked, pMainView);

//		evas_object_smart_callback_add(sn_layout, "slider,clicked", on_slider_clicked, pMainView);
		evas_object_smart_callback_add(sn_layout, "slider,longpress,start", _on_slider_long_press_start, pMainView);
		evas_object_smart_callback_add(sn_layout, "slider,longpress,end", _on_slider_long_press_end, pMainView);

		evas_object_event_callback_add(sn_layout, EVAS_CALLBACK_MOUSE_DOWN,  _on_slider_mouse_down, pMainView);
		evas_object_event_callback_add(sn_layout, EVAS_CALLBACK_MOUSE_MOVE,  _on_slider_mouse_moved, pMainView);
		evas_object_event_callback_add(sn_layout, EVAS_CALLBACK_MOUSE_UP,  _on_slider_mouse_up, pMainView);
	}
//	PERF_CHECK_END(LVL2, "Register callback");

	if ((pMainView->view_by != IVUG_VIEW_BY_FILE)
	        && (pMainView->view_by != IVUG_VIEW_BY_INVAILD)) {
		PERF_CHECK_BEGIN(LVL2, "Create thumblist");

#ifdef USE_THUMBLIST
		pMainView->thumbs = ivug_thumblist_add(pMainView->layout);

		if (pMainView->thumbs == NULL) {
			MSG_MAIN_ERROR("Thumblist creation failed");
			ivug_main_view_destroy(pMainView);
			return NULL;
		}

		elm_object_theme_set(pMainView->thumbs, gGetSystemTheme());
		evas_object_name_set(pMainView->thumbs, "Thumblist");

//		elm_object_part_content_set(pMainView->lyContent, "thumblist", pMainView->thumbs);

		evas_object_event_callback_add(pMainView->thumbs, EVAS_CALLBACK_MOUSE_DOWN, _on_receive_mouse_down, pMainView);
		evas_object_event_callback_add(pMainView->thumbs, EVAS_CALLBACK_MOUSE_UP, _on_receive_mouse_up, pMainView);
		evas_object_smart_callback_add(pMainView->thumbs, "item,realized", _thumb_realized_cb, pMainView);
		evas_object_smart_callback_add(pMainView->thumbs, "changed,mode", _thumb_mode_changed_cb, pMainView);
		evas_object_smart_callback_add(pMainView->thumbs, "item,edit,selected", _edit_item_selected_cb, pMainView);

#endif

		PERF_CHECK_END(LVL2, "Create thumblist");
	}

	if (pMainView->mode == IVUG_MODE_DISPLAY) {
		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,enable,title_full", "user");
	}

	int rot = elm_win_rotation_get(gGetCurrentWindow());
	MSG_MAIN_HIGH("Current window rotation %d", rot);

	if (elm_win_wm_rotation_supported_get(gGetCurrentWindow())) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(gGetCurrentWindow(), (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(gGetCurrentWindow(), "wm,rotation,changed", _on_layout_resize, pMainView);

	pMainView->keydown_handler = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, _on_key_down, (void *)pMainView);

	{
		int bx, by, bw, bh;
		evas_object_geometry_get(pMainView->navi_bar, &bx, &by, &bw, &bh);

		MSG_MAIN_HIGH("Navibar(0x%08x) Created. (%d,%d,%d,%d)", pMainView->navi_bar, bx, by, bw, bh);
	}

	{
		int bx, by, bw, bh;
		evas_object_geometry_get(pMainView->lyContent, &bx, &by, &bw, &bh);

		MSG_MAIN_HIGH("Content layout(0x%08x) Created. (%d,%d,%d,%d)", pMainView->lyContent, bx, by, bw, bh);
	}

	int bx, by, bw, bh;
	evas_object_geometry_get(pMainView->layout, &bx, &by, &bw, &bh);

	MSG_MAIN_HIGH("MainView Layout(0x%08x) Created. (%d,%d,%d,%d)", pMainView->layout, bx, by, bw, bh);



	// creating photo cam
	ivug_create_new_photocam_image(pMainView, &pMainView->photocam, "imageview_area");
	ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam);
	edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_temp_invsible", "imageview_area_temp2");

	if ((pMainView->mode == IVUG_MODE_NORMAL || pMainView->mode == IVUG_MODE_CAMERA || pMainView->view_by == IVUG_VIEW_BY_FOLDER || pMainView->view_by == IVUG_VIEW_BY_ALL)
	        && pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_SELECT && pMainView->mode != IVUG_MODE_HIDDEN) {
		pMainView->btn_favorite = _create_favorite_button(pMainView->lyContent);
		evas_object_smart_callback_add(pMainView->btn_favorite, "clicked", _on_btn_favorite_cb, pMainView);
		elm_object_part_content_set(pMainView->lyContent, "elm.swallow.favorite", pMainView->btn_favorite); //swallow
	}
// For debugging.
//	DELETE_NOTIFY(pMainView->layout);

	PERF_CHECK_END(LVL2, "add event handler");

	return pMainView;

}


bool
ivug_main_view_set_list(Ivug_MainView *pMainView, ivug_parameter *ivug_param)
{
	MSG_MAIN_HIGH("Load media list. pMainView=0x%08x", pMainView);

	PERF_CHECK_BEGIN(LVL2, "create media list");
	if (NULL == pMainView) {
		return false;
	}

	Media_List *mlist = ivug_medialist_create();
	if (mlist == NULL) {
		MSG_MAIN_ERROR("Creating media list failed");
		return false;
	}
	PERF_CHECK_END(LVL2, "create media list");
	PERF_CHECK_BEGIN(LVL2, "create filter");

	Filter_struct *filter = ivug_param_create_filter(ivug_param);
	if (filter == NULL) {
		MSG_MAIN_ERROR("Creating filter failed");
		ivug_medialist_del(mlist);
		return false;
	}

	//do not use ivug_param->view_by after here

	PERF_CHECK_END(LVL2, "create filter");

	Media_Item *current = NULL;
	Media_Data *pData = NULL;
	current = ivug_medialist_load(mlist, filter);

	if (current == NULL) {
		MSG_MAIN_ERROR("Media list load failed");
		goto LOAD_LIST_FAILED;
	}

	pData = ivug_medialist_get_data(current);
	if (pData == NULL) {
		MSG_MAIN_ERROR("current data is NULL");
		goto LOAD_LIST_FAILED;
	}
	//ULC changes
	ivug_medialist_set_current_item(mlist, current);
	//ULC changes

	if (pData->fileurl == NULL) {
		MSG_MAIN_ERROR("current fileurl is NULL");
		goto LOAD_LIST_FAILED;
	}

#ifdef USE_RESCAN_FILE_PATH_AT_LIST
	{
		if (strncmp(pData->fileurl, ivug_param->filepath, strlen(pData->fileurl)) != 0) {
			current = ivug_medialist_find_item_by_filename(mlist, ivug_param->filepath);
			if (current == NULL) {
				MSG_MAIN_ERROR("Media list load failed, %s is not exist at list", ivug_param->filepath);
				goto LOAD_LIST_FAILED;
			}

			pData = ivug_medialist_get_data(current);
			if (pData == NULL) {
				MSG_MAIN_ERROR("current data is NULL");
				goto LOAD_LIST_FAILED;
			}
			//ULC changes
			ivug_medialist_set_current_item(mlist, current);
			//ULC changes

			if (pData->fileurl == NULL) {
				MSG_MAIN_ERROR("current fileurl is NULL");
				goto LOAD_LIST_FAILED;
			}
		}
	}
#endif

	if (filter->view_by == IVUG_VIEW_BY_ALL) {
		pMainView->album_name = strdup(IDS_ALL_ALBUMS);
	} else if (filter->view_by == IVUG_VIEW_BY_HIDDEN_ALL) {
		pMainView->album_name = strdup(IDS_HIDDEN);
	} else if (filter->view_by == IVUG_VIEW_BY_FOLDER) {
		/*
		media_handle m_handle = ivug_db_get_folder_handle(ivug_dir_get(ivug_param->filepath));
		if (m_handle == NULL)
		{
			MSG_IVUG_FATAL("View by Folder. but media handle is NULL" );
			return NULL;
		}

		pMainView->album_name = ivug_db_get_folder_name(m_handle);
		*/
		pMainView->album_name = _ivug_get_folder_name(ivug_param->filepath);
		if (pMainView->album_name == NULL) {
			pMainView->album_name = strdup(IDS_NO_NAME);
		}
	} else if (filter->view_by == IVUG_VIEW_BY_HIDDEN_FOLDER) {
		/*
		media_handle m_handle = ivug_db_get_folder_handle(ivug_dir_get(ivug_param->filepath));
		if (m_handle == NULL)
		{
			MSG_IVUG_FATAL("View by Folder. but media handle is NULL" );
			return NULL;
		}

		pMainView->album_name = ivug_db_get_folder_name(m_handle);
		*/
		pMainView->album_name = _ivug_get_folder_name(ivug_param->filepath);
		if (pMainView->album_name == NULL) {
			pMainView->album_name = strdup(IDS_NO_NAME);
		}
	}
	pMainView->mList = mlist;
	PERF_CHECK_BEGIN(LVL2, "media list set");

	if (ivug_medialist_get_count(mlist) == 1) {
		if (ivug_param->mode == IVUG_MODE_DISPLAY) {
			ivug_slider_new_set_mode(pMainView->pSliderNew, SLIDER_MODE_SINGLE);
		}
		// temp code, please remove later
		else if (ivug_param->view_by == IVUG_VIEW_BY_FILE) {
			ivug_slider_new_set_mode(pMainView->pSliderNew, SLIDER_MODE_SINGLE);
		}
	}

	if (ivug_param->mode == IVUG_MODE_SELECT) {
		int count = eina_list_count(ivug_param->selected_list);
		pMainView->selected_path_list = NULL;
		int i;
		char *temp = NULL;
		for (i = 0; i < count; i++) {
			temp = (char *)eina_list_nth(ivug_param->selected_list, i);
			if (temp) {
				Media_Item *mitem = ivug_medialist_find_item_by_filename(mlist, temp);
				if (mitem) {
					Media_Data *mdata = ivug_medialist_get_data(mitem);
					pMainView->selected_path_list = eina_list_append(pMainView->selected_path_list, mdata->filepath);
				} else {
					pMainView->selected_path_list = eina_list_append(pMainView->selected_path_list, temp);
				}
			}
		}
		pMainView->total_selected = ivug_param->total_selected ;
		pMainView->max_count = ivug_param->select_view_max_count;
		pMainView->limit_size = ivug_param->select_view_limit_size;
		pMainView->select_size = ivug_param->select_view_selected_size;
	}

	ivug_slider_new_set_list(pMainView->pSliderNew, mlist, current);

	PERF_CHECK_END(LVL2, "media list set");

	return true;

LOAD_LIST_FAILED:
	if (mlist) {
		ivug_medialist_del(mlist);
	}
	pMainView->mList = NULL;
	return false;
}


void ivug_main_view_start_slideshow(Ivug_MainView *pMainView, Eina_Bool bSlideFirst)
{
	IV_ASSERT(pMainView != NULL);

	Media_Item *current = NULL;

	// Stop animation & movie play before slideshow is started.
	current = ivug_medialist_get_current_item(pMainView->mList);

//	ivug_prohibit_lcd_off();
	//ivug_main_view_hide_menu_bar(pMainView);

	pMainView->ssHandle = ivug_ss_create(pMainView->layout);
	pMainView->isSliding = true;

// Register callback
	evas_object_smart_callback_add(ivug_ss_object_get(pMainView->ssHandle),  "slideshow,finished", _on_slideshow_finished, pMainView);


	ivug_main_view_hide_menu_bar(pMainView);
	ivug_ss_start(pMainView->ssHandle, current, pMainView->mList, bSlideFirst);
}


Evas_Object *
ivug_main_view_object_get(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	return pMainView->layout;
}

static void
_back_button_clicked(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	elm_exit();
}

void ivug_main_view_set_testmode(Ivug_MainView *pMainView, bool bTestMode)
{
	IV_ASSERT(pMainView != NULL);

	pMainView->bTestMode = bTestMode;
}

static void ivug_update_list(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);
	/*Recreate or refresh the list  "pMainView->mList"  */
	ivug_medialist_set_update_flag(pMainView->mList, true);
}

static Eina_Bool _ivug_db_update_idler(void *data)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);
	ivug_update_list(pMainView);
	if (pMainView->db_idler != NULL) {
		ecore_timer_del(pMainView->db_idler);
		pMainView->db_idler = NULL;
	}
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _ivug_db_update_timer_cb(void *data)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	if (pMainView->db_idler != NULL) {
		ecore_timer_del(pMainView->db_idler);
		pMainView->db_idler = NULL;
	}
	pMainView->db_idler = ecore_idler_add(_ivug_db_update_idler, data);
	return ECORE_CALLBACK_CANCEL;
}

static int _ivug_db_update_add_timer(void *data)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);
	_ivug_db_update_timer_cb(data);
	return 0;
}

static int
_ivug_db_update_op(media_content_error_e error, int pid,
                   media_content_db_update_item_type_e update_item,
                   media_content_db_update_type_e update_type,
                   media_content_type_e media_type, char *uuid,
                   char *path, char *mime_type, void *data)
{
	if (MEDIA_CONTENT_ERROR_NONE != error) {
		MSG_MAIN_ERROR("Update db error[%d]!", error);
		return -1;
	}
	if (update_item == MEDIA_ITEM_FILE &&
	        MEDIA_CONTENT_TYPE_IMAGE != media_type &&
	        MEDIA_CONTENT_TYPE_VIDEO != media_type) {
		MSG_MAIN_HIGH("Media type is wrong");
		return -1;
	}

	_ivug_db_update_add_timer(data);
	return 0;
}

static void
__ivug_db_update_cb(media_content_error_e error, int pid,
                    media_content_db_update_item_type_e update_item,
                    media_content_db_update_type_e update_type,
                    media_content_type_e media_type, char *uuid,
                    char *path, char *mime_type, void *data)
{
	MSG_MAIN_HIGH("update_item[%d], update_type[%d], media_type[%d]", update_item,
	              update_type, media_type);

	MSG_MAIN_HIGH("uuid[%s], path[%s]", uuid, path);
	_ivug_db_update_op(error, pid, update_item, update_type, media_type,
	                   uuid, path, mime_type, data);
}

static bool _ivug_db_update_reg_cb(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);
	int ret = -1;

	MSG_MAIN_HIGH("Set db updated callback");
	ret = media_content_set_db_updated_cb(__ivug_db_update_cb, pMainView);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		MSG_MAIN_ERROR("Set db updated cb failed[%d]!", ret);
	}
	return true;
}

/* Connect to media-content database */
static int _ivug_local_data_connect(void)
{
	int ret = -1;
	ret = media_content_connect();
	if (ret == MEDIA_CONTENT_ERROR_NONE) {
		MSG_MAIN_HIGH("DB connection is success");
		return 0;
	} else {
		MSG_MAIN_ERROR("DB connection is failed[%d]!", ret);
		return -1;
	}
}

static int _ivug_data_init(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);
	int err = _ivug_local_data_connect();
	if (err != 0) {
		MSG_MAIN_ERROR("Connect to media-content DB failed!");
	}
	return err;
}

static Eina_Bool
_ivug_main_view_reg_idler_cb(void *data)
{
	IV_ASSERT(data != NULL);
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);
	/* Initialize media-content */
	_ivug_data_init(pMainView);
	/* Register db monitor */
	_ivug_db_update_reg_cb(pMainView);

	if (pMainView->reg_idler != NULL) {
		ecore_timer_del(pMainView->reg_idler);
		pMainView->reg_idler = NULL;
	}

	return ECORE_CALLBACK_CANCEL;
}

int ivug_add_reg_idler(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);
	MSG_MAIN_HIGH("Register idler!");
	pMainView->reg_idler = ecore_timer_add(0.6f, _ivug_main_view_reg_idler_cb,
	                                       (void *)pMainView);
	return 0;
}

void
ivug_main_view_start(Ivug_MainView *pMainView, app_control_h service)
{
	IV_ASSERT(pMainView != NULL);
	MSG_MAIN_HIGH("ivug_main_view_start");

	Evas_Object *sn_layout = ivug_slider_new_get_layout(pMainView->pSliderNew);
	int count = -1;
	int currentindex = -1;
	Evas_Load_Error e = EVAS_LOAD_ERROR_NONE;
	char *default_thumbnail_edj_path = DEFAULT_THUMBNAIL_PATH;

	if (pMainView->mode != IVUG_MODE_HIDDEN) {
		Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
		if (!mitem) free(default_thumbnail_edj_path);
		ivug_ret_if(mitem == NULL);

		Media_Data *mdata = ivug_medialist_get_data(mitem);
		if (!mdata) free(default_thumbnail_edj_path);
		ivug_ret_if(mdata == NULL);

		pMainView->cur_mitem = mitem;
		pMainView->currentphotocam = PHOTOCAM_1;
		ivug_medialist_set_current_item(pMainView->mList, pMainView->cur_mitem);
		{
			ivug_main_view_set_hide_timer(pMainView);
		}

		MSG_MAIN_HIGH("mdata->filepath = %s", mdata->filepath);
		if (mdata->slide_type == SLIDE_TYPE_VIDEO) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
			pMainView->is_play_Icon = true;
			elm_photocam_file_set(pMainView->photocam, mdata->thumbnail_path);
			ivug_disable_gesture(pMainView->pSliderNew);
		} else {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
			pMainView->is_play_Icon = false;
			int bx = 0, by = 0, bw = 0, bh = 0;
			e = elm_photocam_file_set(pMainView->photocam, mdata->filepath);
			MSG_MAIN_HIGH("photocam_file_set Error = %d", e);

			if (EVAS_LOAD_ERROR_NONE != e) {
				MSG_HIGH("Loading default Thumbnail");
				elm_photocam_file_set(pMainView->photocam, default_thumbnail_edj_path);
			}
			evas_object_geometry_get(pMainView->photocam, &bx, &by, &bw, &bh);
		}

		evas_object_show(pMainView->photocam);

		e = evas_object_image_load_error_get(elm_image_object_get(pMainView->photocam));
		MSG_MAIN_HIGH("image_load_error = %d", e);

		ivug_slider_update_icon_layer(pMainView->pSliderNew);

		//create the Next and previous photocam images for smooth sliding
		pMainView->photocam0 = NULL;
		pMainView->photocam2 = NULL;

		if (pMainView->cur_mitem) {
			count = ivug_medialist_get_count(pMainView->mList);
			Media_Data *pData = ivug_medialist_get_data(pMainView->cur_mitem);
			currentindex =  pData->index;
			MSG_MAIN_HIGH("currentindex = %d count =%d", currentindex, count);
		}

		ivug_create_new_photocam_image(pMainView, &pMainView->photocam, "imageview_area");
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam);

		if (mdata->slide_type == SLIDE_TYPE_VIDEO) {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,icon", "video_play_icon");
			pMainView->is_play_Icon = true;
			elm_photocam_file_set(pMainView->photocam, mdata->thumbnail_path);
			ivug_disable_gesture(pMainView->pSliderNew);
		} else {
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");
			pMainView->is_play_Icon = false;
			e = elm_photocam_file_set(pMainView->photocam, mdata->filepath);

			if (EVAS_LOAD_ERROR_NONE != e) {
				MSG_HIGH("Loading default Thumbnail");
				elm_photocam_file_set(pMainView->photocam, default_thumbnail_edj_path);
			}
		}
	} else {
		ivug_create_new_photocam_image(pMainView, &pMainView->photocam, "imageview_area");
		ivug_slider_new_set_photocam(pMainView->pSliderNew, pMainView->photocam);
		edje_object_signal_emit(elm_layout_edje_get(sn_layout), "hide,icon", "video_play_icon");

		char *hidden_file_path = NULL;
		int ret = app_control_get_extra_data(service, "Path", &hidden_file_path);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_HIGH("app_control_get_extra_data failed");
		}

		e = elm_photocam_file_set(pMainView->photocam, hidden_file_path);

		if (EVAS_LOAD_ERROR_NONE != e) {
			MSG_HIGH("Loading default Thumbnail");
			elm_photocam_file_set(pMainView->photocam, default_thumbnail_edj_path);
		}
	}

	evas_object_show(pMainView->photocam);
	edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_default", "imageview_area");
	edje_object_signal_emit(elm_layout_edje_get(sn_layout), "show,back,btn", "user");

	if (pMainView->mode != IVUG_MODE_SETAS && pMainView->mode != IVUG_MODE_EMAIL && pMainView->mode != IVUG_MODE_HIDDEN) {

		// Update Main View.
		if (pMainView->bShowMenu == true) {
			PERF_CHECK_BEGIN(LVL5, "Update main view");
			_update_main_view(pMainView);
			PERF_CHECK_END(LVL5, "Update main view");
		}

		if (count != -1 && currentindex != -1 && currentindex + 1 < count) {
			Media_Item *next_mitem = ivug_medialist_get_next(pMainView->mList, pMainView->cur_mitem);
			Media_Data *pData = ivug_medialist_get_data(next_mitem);

			ivug_create_new_photocam_image(pMainView, &pMainView->photocam2, "imageview_area_temp2");
			if (pData->slide_type == SLIDE_TYPE_VIDEO) {
				elm_photocam_file_set(pMainView->photocam2, pData->thumbnail_path);
			} else {
				e = elm_photocam_file_set(pMainView->photocam2, pData->filepath);

				if (EVAS_LOAD_ERROR_NONE != e) {
					MSG_HIGH("Loading default Thumbnail");
					elm_photocam_file_set(pMainView->photocam2, default_thumbnail_edj_path);
				}
			}

			evas_object_show(pMainView->photocam2);
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_right", "imageview_area_temp2");
		}

		if (count != -1 && currentindex != -1 && currentindex - 1 >= 0) {
			Media_Item *prev_mitem = ivug_medialist_get_prev(pMainView->mList, pMainView->cur_mitem);
			Media_Data *pmData = ivug_medialist_get_data(prev_mitem);

			ivug_create_new_photocam_image(pMainView, &pMainView->photocam0, "imageview_area_temp0");
			if (pmData->slide_type == SLIDE_TYPE_VIDEO) {
				elm_photocam_file_set(pMainView->photocam0, pmData->thumbnail_path);
			} else {
				e = elm_photocam_file_set(pMainView->photocam0, pmData->filepath);
				if (EVAS_LOAD_ERROR_NONE != e) {
					MSG_HIGH("Loading default Thumbnail");
					elm_photocam_file_set(pMainView->photocam0, default_thumbnail_edj_path);
				}
			}

			evas_object_show(pMainView->photocam0);
			edje_object_signal_emit(elm_layout_edje_get(sn_layout), "set_left", "imageview_area_temp0");
		}
		edje_object_signal_callback_add(elm_layout_edje_get(sn_layout), "left_transit_done", "*",
		                                _ivug_main_view_left_transit_by_item_complete_cb, pMainView);
		edje_object_signal_callback_add(elm_layout_edje_get(sn_layout), "right_transit_done", "*",
		                                _ivug_main_view_right_transit_by_item_complete_cb, pMainView);

		edje_object_signal_callback_add(elm_layout_edje_get(pMainView->lyContent), "button_clicked", "elm", _back_button_clicked, pMainView);
	}
	if (pMainView->mode == IVUG_MODE_SELECT) {
		char *main_edj_path = IVUG_MAIN_EDJ;
		Evas_Object *sel_bar = create_layout(pMainView->layout, main_edj_path, "select_bar");
		free(main_edj_path);
		if (sel_bar == NULL) {
			IVUG_DEBUG_WARNING("layout create failed");
		}
		edje_object_signal_emit(elm_layout_edje_get(pMainView->lyContent), "hide,back,btn", "user");
		pMainView->select_bar = sel_bar;
		Evas_Object *check = elm_check_add(sel_bar);
		elm_check_state_set(check, EINA_FALSE);
		elm_object_style_set(check, "default");
		evas_object_color_set(check, 128, 138, 137, 255);
		elm_layout_content_set(sel_bar, "swallow.check", check); //swallow
		pMainView->check = check;
		evas_object_smart_callback_add(check, "changed", _check_changed_cb, (void *)pMainView);
		elm_object_part_content_set(pMainView->layout, "mainview.select_bar", sel_bar); //swallow

		char buf[64] = {0,};
		snprintf(buf, 64, GET_STR(IDS_PD_SELECTED), pMainView->total_selected);
		Eina_List *l = NULL;
		void *data = NULL;
		char **files = NULL;
		int i = 0 ;
		files = (char **)malloc(sizeof(char *) * pMainView->total_selected);
		if (files) {
			if (pMainView->selected_path_list) {
				EINA_LIST_FOREACH(pMainView->selected_path_list, l, data) {
					files[i] = strdup((char *)data);
					Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
					Media_Data *mdata = ivug_medialist_get_data(mitem);
					if (strcmp(mdata->filepath, files[i]) == 0) {
						evas_object_color_set(pMainView->check, 255, 255, 255, 255);
						elm_check_state_set(pMainView->check, EINA_TRUE);
					}
					i++;
				}
			}
		}
		i--;
		while(i >= 0) {
			free(files[i--]);
		}
		free(files);
		elm_object_part_text_set(pMainView->select_bar, "elm.text.title", buf);
		evas_object_show(check);
	} else if (pMainView->mode == IVUG_MODE_EMAIL) {
		edje_object_signal_callback_add(elm_layout_edje_get(pMainView->lyContent), "button_clicked", "elm", _back_button_clicked, pMainView);
	}/*else {
		edje_object_signal_emit(elm_layout_edje_get(pMainView->lyContent), "hide,back,btn", "user");
		edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,hide", "user");
		pMainView->bShowMenu = false ;
	}*/
	free(default_thumbnail_edj_path);
}


void
ivug_main_view_resume(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_HIGH("Main View Update");

	struct stat info = {0,};
	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	IV_ASSERT(mdata != NULL);

	if (ivug_slider_new_get_mode(pMainView->pSliderNew) != SLIDER_MODE_SINGLE) {
		ivug_medialist_del_update_callback(pMainView->mList);
	}

	if (mdata->slide_type == SLIDE_TYPE_IMAGE || mdata->slide_type == SLIDE_TYPE_VIDEO) {
		if (mdata->filepath && (stat(mdata->filepath, &info) == 0)) {
			MSG_MAIN_HIGH("Current filepath : %s", mdata->filepath);
		} else {
			MSG_MAIN_ERROR("Center file is not exist. stype=%d name=%s", mdata->slide_type, mdata->filepath);
			Media_Item *nxt_item = ivug_medialist_get_next(pMainView->mList, mitem);
			if (nxt_item) {
				ivug_medialist_set_current_item(pMainView->mList, nxt_item);
			} else {
				Media_Item *prv_item = ivug_medialist_get_prev(pMainView->mList, mitem);
				if (prv_item) {
					ivug_medialist_set_current_item(pMainView->mList, prv_item);
				} else {
					DESTROY_ME();
					return;
				}
			}
			mitem = ivug_medialist_get_current_item(pMainView->mList);
			mdata = ivug_medialist_get_data(mitem);
		}
	} else {
		MSG_MAIN_ERROR("Unhandled slide type : %d", mdata->slide_type);
	}

	if (ivug_medialist_need_update(pMainView->mList) == true) {
#ifdef USE_THUMBLIST
		if (pMainView->thumbs) {
			ivug_thumblist_clear_items(pMainView->thumbs);
		}
#endif
		mitem = ivug_medialist_reload(pMainView->mList, mitem);
		ivug_main_view_start(pMainView, NULL);
	}

	ivug_main_view_set_hide_timer(pMainView);

// When resume, menu bar will be appeared.
	if (!(pMainView->pNameView
	        || pMainView->pCropView) && pMainView->mode != IVUG_MODE_SELECT && pMainView->mode != IVUG_MODE_EMAIL) {	//if child view is not exist
		ivug_main_view_show_menu_bar(pMainView);
	}
}

void
ivug_main_view_pause(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	if (ivug_slider_new_get_mode(pMainView->pSliderNew) != SLIDER_MODE_SINGLE) {
		ivug_medialist_set_update_callback(pMainView->mList);
	}

	if (pMainView->ssHandle) {
		ivug_ss_stop(pMainView->ssHandle);
	}

	ivug_main_view_del_hide_timer(pMainView);

	// Stop photocam If AGIF
}

__attribute__((used)) void dump_obj(Evas_Object *obj, int lvl)
{
	Eina_List *list = evas_object_smart_members_get(obj);

	if (lvl == 0) {
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(obj, &mW, &mH);
		evas_object_size_hint_max_get(obj, &MW, &MH);

		MSG_HIGH("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);
		lvl++;
	}

	Evas_Object *data;
	Eina_List *l;

	for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l)) {
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(data, &mW, &mH);
		evas_object_size_hint_max_get(data, &MW, &MH);

		char *space = new char[lvl * 2 + 1];

		for (int i = 0; i < lvl * 2; i++) {
			space[i] = ' ';
		}

		space[lvl * 2] = '\0';

		MSG_HIGH("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", space, evas_object_name_get(data), evas_object_type_get(data), data, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);

		delete[] space;

		dump_obj(data, lvl + 1);

	}
}

void
ivug_main_view_destroy(Ivug_MainView *pMainView)
{
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_HIGH("ENTER : Main View Destroy. pMainView=0x%08x", pMainView);


	ivug_main_view_del_hide_timer(pMainView);
	if (pMainView->ssHandle) {
		MSG_MAIN_HIGH("image viewer end cause slide show ended");
		ivug_ss_delete(pMainView->ssHandle);
		pMainView->ssHandle = NULL;
	}

	if (pMainView->back_timer) {
		ecore_timer_del(pMainView->back_timer);
		pMainView->back_timer = NULL;
	}

	if (pMainView->popup_timer) {
		ecore_timer_del(pMainView->popup_timer);
		pMainView->popup_timer = NULL;
	}

	if (pMainView->delete_idler) {
		ecore_idler_del(pMainView->delete_idler);
		pMainView->delete_idler = NULL;
	}

	if (pMainView->tagbuddy_idler) {
		ecore_timer_del(pMainView->tagbuddy_idler);
		pMainView->tagbuddy_idler = NULL;
	}

	if (pMainView->ctx_popup) {
		evas_object_del(pMainView->ctx_popup);
		pMainView->ctx_popup = NULL;
	}

	if (pMainView->popup) {
		evas_object_del(pMainView->popup);
		pMainView->popup = NULL;
	}

	if (pMainView->access_popup) {
		evas_object_del(pMainView->access_popup);
		pMainView->access_popup = NULL;
	}

	if (pMainView->ext_svc) {
		app_control_send_terminate_request(pMainView->ext_svc);
		app_control_destroy(pMainView->ext_svc);
		pMainView->ext_svc = NULL;
	}

	if (pMainView->exit_timer) {
		ecore_timer_del(pMainView->exit_timer);
		pMainView->exit_timer = NULL;
	}

	MSG_MAIN_HIGH("Unregister system notifications");

	if (pMainView->keydown_handler) {
		ecore_event_handler_del(pMainView->keydown_handler);
		pMainView->keydown_handler = NULL;
	}

	if (pMainView->album_name) {
		free(pMainView->album_name);
		pMainView->album_name = NULL;
	}

// Who remove medialist?????
	_on_remove_main_view_ui(pMainView);

	dump_obj(pMainView->parent, 0);
	free(pMainView);

	MSG_MAIN_HIGH("LEAVE : Main View Destroy.");

	return ;
}

void ivug_main_view_reload(Ivug_MainView *pMainView)
{
	Media_Data *pData = NULL;
	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	pData = ivug_medialist_get_data(mitem);

	if (pData->slide_type == SLIDE_TYPE_VIDEO) {
		elm_photocam_file_set(pMainView->photocam, pData->thumbnail_path);
	} else {
		elm_photocam_file_set(pMainView->photocam, NULL);
		elm_photocam_file_set(pMainView->photocam, pData->filepath);
	}
	ivug_slider_update_icon_layer(pMainView->pSliderNew);

}

void _ivug_main_view_set_hide_timer(Ivug_MainView *pMainView, const char *func, int line)
{
	IV_ASSERT(pMainView != NULL);

	if (pMainView->mode == IVUG_MODE_DISPLAY
	        || pMainView->mode == IVUG_MODE_SAVE
			|| pMainView->mode == IVUG_MODE_SETAS
			|| pMainView->mode == IVUG_MODE_EMAIL) {

		return;
	}

	MSG_MAIN_HIGH("Set Menu Timer. Cnt=%d, %s(%d)", pMainView->hide_count, func, line);

	pMainView->hide_count++;

	if (pMainView->hide_count < 0) {
		// hide timer has stack structure, del > del > set(not really setted) > set (really setted)
		return ;
	}


	if (pMainView->bShowMenu == EINA_TRUE) {
		if (pMainView->hide_timer) {
			ecore_timer_del(pMainView->hide_timer);
			pMainView->hide_timer = NULL;
		}
		pMainView->hide_timer = ecore_timer_add(MENUBAR_TIMEOUT_SEC, _on_hide_timer_expired, (void *)pMainView);
	} else {
		MSG_MAIN_WARN("Called set_hide_timer. when Menu is invisible. %s(%d)", func, line);
	}
}

void _ivug_main_view_del_hide_timer(Ivug_MainView *pMainView, const char *func, int line)
{
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_HIGH("Remove Hide Timer. Cnt=%d, %s(%d)", pMainView->hide_count, func, line);

	pMainView->hide_count--;

	if (pMainView->hide_timer) {
		ecore_timer_del(pMainView->hide_timer);
		pMainView->hide_timer = NULL;
	}
}

void
ivug_main_view_show_menu_bar(Ivug_MainView *pMainView)
{
	MSG_MAIN_HIGH("Show Menu");
	if (NULL == pMainView) {
		MSG_MAIN_WARN("pMainView is NULL");
		return;
	}

	if (pMainView->bShowMenu == true) {
		MSG_MAIN_WARN("Menu is Already shown");
		return;
	}

	pMainView->bShowMenu = true;

	_update_main_view(pMainView);

#ifdef USE_THUMBLIST
	if (pMainView->thumbs) {
		Media_Item *mItem = ivug_medialist_get_current_item(pMainView->mList);

		Image_Object *img = NULL;
		img = ivug_thumblist_find_item_by_data(pMainView->thumbs, mItem);		// Find thumb item of new image,.

		if (img != NULL) {
			if (ivug_thumblist_get_selected_item(pMainView->thumbs) != img) { // Duplicate check but should check in here.
				pMainView->bSetThmByUser = true;
				ivug_thumblist_select_item(pMainView->thumbs, img);
				// If old item & new item is same, `selected item' callback is not called. then bSetThmByUser remains as true.
			}
		}
	}
#endif

	edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,show", "user");
	edje_object_signal_emit(elm_layout_edje_get(pMainView->lyContent), "show,back,btn", "user");

	ivug_main_view_set_hide_timer(pMainView);
}

void
ivug_main_view_hide_menu_bar(Ivug_MainView *pMainView)
{
	MSG_MAIN_HIGH("Hide Menu");

	if (NULL == pMainView) {
		MSG_MAIN_WARN("pMainView is NULL");
		return;
	}

	if (pMainView->bShowMenu == false) {
		MSG_MAIN_WARN("Menu is Already hidden");
		return;
	}

	ivug_main_view_del_hide_timer(pMainView);

	pMainView->bShowMenu = false;

	edje_object_signal_emit(_EDJ(pMainView->lyContent), "elm,state,hide", "user");
	edje_object_signal_emit(elm_layout_edje_get(pMainView->lyContent), "hide,back,btn", "user");
}

