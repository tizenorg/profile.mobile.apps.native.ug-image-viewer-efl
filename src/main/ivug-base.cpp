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

#include <Elementary.h>
#include <app.h>
#include <storage.h>
#include <efl_extension.h>

#include "ivug-common.h"
#include "ivug-util.h"
#include "ivug-popup.h"

#include "ivug-callback.h"
#include "ivug-context.h"
#include "ivug-language-mgr.h"
#include "ivug-base.h"

static int __externalStorageId = 0;

bool getSupportedStorages_cb(int storageId, storage_type_e type, storage_state_e state, const char *
                             path, void *userData)
{
	MSG_NOTI_ERROR("");
	if (type == STORAGE_TYPE_EXTERNAL) {
		__externalStorageId = storageId;
		return false;
	}
	return true;
}

void FreeUGData(ug_data *ug)
{
	IV_ASSERT(ug != NULL);

	free(ug);
}

#if 0
static void _send_result_to_caller(ui_gadget_h ug)
{
	int ret = 0;
	app_control_h service = NULL;
	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_HIGH("app_control_create failed");
		return;
	}

	ret = app_control_add_extra_data(service, "ivug.status", "started");
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_HIGH("app_control_add_extra_data failed");
	}
	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);

	MSG_IMAGEVIEW_HIGH("Send load started event to caller");
}
#endif

#ifdef TRACE_WINDOW
static void _on_win_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_IMAGEVIEW_HIGH("Parent win(0x%08x) resized geomtery XYWH(%d,%d,%d,%d) angle=%d", obj, x, y, w, h, elm_win_rotation_get((Evas_Object *)ug_get_window()));
}

static void _on_win_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_IMAGEVIEW_HIGH("Parent win(0x%08x) moved geomtery XYWH(%d,%d,%d,%d) angle=%d", obj, x, y, w, h, elm_win_rotation_get((Evas_Object *)ug_get_window()));
}
#endif


static void _on_base_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_IMAGEVIEW_HIGH("Base layout(0x%08x) resized geomtery XYWH(%d,%d,%d,%d)", obj, x, y, w, h);
}

static void _on_base_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_IMAGEVIEW_HIGH("Base layout(0x%08x) moved geomtery XYWH(%d,%d,%d,%d)", obj, x, y, w, h);
}


static void _on_base_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MSG_IMAGEVIEW_HIGH("Base(0x%08x) layout show", obj);
}

static void _on_base_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MSG_IMAGEVIEW_HIGH("Base(0x%08x) layout hide", obj);
}

static void _on_receive_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MSG_MAIN_HIGH("Base layout(0x%08x) clicked : %s Layer=%d", obj, evas_object_name_get(obj), evas_object_layer_get(obj));
}

static Eina_Bool _on_exit_timer_expired(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	ug_data *ugd = (ug_data *)data;

	ugd->exit_timer = NULL;

	DESTROY_ME();

	return ECORE_CALLBACK_CANCEL;
}

static bool _is_mmc_inserted(void)
{
	int error = storage_foreach_device_supported(getSupportedStorages_cb, NULL);

	if (error == STORAGE_ERROR_NONE) {
		storage_state_e mmc_state;
		int ret = storage_get_state(__externalStorageId, &mmc_state);
		if (ret != STORAGE_ERROR_NONE) {
			MSG_NOTI_ERROR("storage_get_state %d is failed", mmc_state);
			return false;
		}

		if (mmc_state == STORAGE_STATE_MOUNTED) {
			return true;
		}
	}

	return false;
}

static void _on_mmc_state_changed(int storage_id, storage_state_e state, void *user_data)
{
	ug_data *ugd = (ug_data *)user_data;
	IV_ASSERT(ugd != NULL);

	if (_is_mmc_inserted() == false) {
		MSG_IMAGEVIEW_WARN("MMC Removed!");
		if (strncmp(ugd->ivug_param->filepath, PATH_SDCARD, strlen(PATH_SDCARD)) != 0
		        && ugd->ivug_param->view_by != IVUG_VIEW_BY_ALL
		        && ugd->ivug_param->view_by != IVUG_VIEW_BY_HIDDEN_ALL) {
			return;
		}

		MSG_IMAGEVIEW_WARN("Request destroy UG");

		if (ugd->main_view) {
			_ivug_main_on_mmc_state_changed(ugd->main_view);
		} else if (ugd->ss_view) {
			_ivug_slideshow_view_on_mmc_state_changed(ugd->ss_view);
		} else {
			if (ugd->exit_timer == NULL) {
				ugd->exit_timer = ecore_timer_add(0.2, _on_exit_timer_expired, ugd);
			}
		}
	} else {
		MSG_IMAGEVIEW_WARN("MMC Inserted!");
	}
}

static void _on_base_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	MSG_IMAGEVIEW_ERROR("_on_base_deleted");
}
#if 0
static void _destroy_manually(void *priv)
{
	ug_data *ugd = (ug_data *)priv;

	MSG_IMAGEVIEW_HIGH("_destroy_manually ugd=0x%08x", ugd);

	int error_code = storage_unset_state_changed_cb(__externalStorageId, _on_mmc_state_changed);
	if (error_code != STORAGE_ERROR_NONE) {
		MSG_MAIN_ERROR("storage_unset_state_changed_cb() failed!!");
	}

	if (ugd->bErrMsg) {
		free(ugd->bErrMsg);
		ugd->bErrMsg = NULL;
	}

	if (ugd->crop_ug) {
		ivug_crop_ug_destroy(ugd->crop_ug);
		ugd->crop_ug = NULL;
	}

	//destroy main view.
	if (ugd->main_view) {
		PERF_CHECK_BEGIN(LVL1, "MainView");
		ivug_main_view_destroy(ugd->main_view);
		ugd->main_view = NULL;
		PERF_CHECK_END(LVL1, "MainView");
	}

	if (ugd->ss_view) {
		PERF_CHECK_BEGIN(LVL1, "SlideShowView");
		ivug_slideshow_view_destroy(ugd->ss_view);
		ugd->ss_view = NULL;
		PERF_CHECK_END(LVL1, "SlideShowView");
	}

	//delete param.
	if (ugd->ivug_param) {
		ivug_param_delete(ugd->ivug_param);
		ugd->ivug_param = NULL;
	}

	if (ugd->base) {
		PERF_CHECK_BEGIN(LVL1, "Base layout");
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_DEL, _on_base_deleted);
		evas_object_del(ugd->base);
		ugd->base = NULL;
		PERF_CHECK_END(LVL1, "Base layout");
	}
}
#endif
Evas_Object *create_layout(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);
//	MSG_ASSERT(parent != NULL);

	Evas_Object *layout;
	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_IMAGEVIEW_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_IMAGEVIEW_ERROR("Layout file set failed! %s in %s", group, edj);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	return layout;
}

static Evas_Object *create_fullview(Evas_Object *win)
{
	Evas_Object *base;

	/* Create Full view */
	char *edj_path = full_path(EDJ_PATH, "/ivug-base.edj");
	base = create_layout(win, edj_path, "ivug_base");
	if (base == NULL) {
		MSG_IMAGEVIEW_HIGH("Cannot set layout. EDJ=%s Group=%s", edj_path, "ivug_base");
		free(edj_path);
		free(EDJ_PATH);
		return NULL;
	}
	free(EDJ_PATH);
	free(edj_path);

	evas_object_name_set(base, "Base layout");

	evas_object_size_hint_expand_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	return base;
}
#if 0
static Evas_Object *create_frameview(Evas_Object *parent, ug_data *ugd)
{
	Evas_Object *base = NULL;

	/* Create Frame view */
	base = elm_layout_add(parent);
	evas_object_name_set(base, "Base layout frame");
	elm_layout_theme_set(base, "layout", "application", "default");

	return base;
}
#endif
bool on_create( void *priv)
{
	ug_data *ugd;

	MSG_MED("on_create.");

	/*Evas_Object *conform = (Evas_Object *)ug_get_conformant();
	IV_ASSERT(conform != NULL);*/

	PERF_CHECK_END(LVL0, "UG_MODULE_INIT -> On Create");

	PERF_CHECK_BEGIN(LVL0, "On Create");

	MSG_MED("on_create. IV & PRIV");

	if (!priv) {
		MSG_ERROR("Error. priv=0x%08x", priv);
		return false;
	}

	ugd = (ug_data *)priv;

	MSG_MED("Image Viewer : %s  data=0x%08x", __func__, ugd);

	Evas_Object *win = elm_win_util_standard_add("ug-image-viewer-efl", "ug-image-viewer-efl");
	ugd->window = win;


	int wx, wy, ww, wh;
	int error_code = -1;

	char *edj_path = full_path(EDJ_PATH, "/ivug-base.edj");
	elm_theme_extension_add(NULL, edj_path);
	free(edj_path);

	evas_object_geometry_get(win, &wx, &wy, &ww, &wh);

	MSG_MED("Parent Info. Win(0x%08x) Size(%d,%d,%d,%d) rotation=%d",
					   win, wx, wy, ww, wh,  elm_win_rotation_get(win));

	Evas_Object *conform = elm_conformant_add(win);
	if (!conform)
		return false;
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(conform, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_win_resize_object_add(win, conform);
	evas_object_show(conform);
	evas_object_show(win);
	elm_win_conformant_set(win, EINA_TRUE);

	PERF_CHECK_BEGIN(LVL1, "init context");
	if (!ivug_context_init(win, conform)) {
		MSG_ERROR("ivug_main_init error");
		return false;
	}
	PERF_CHECK_END(LVL1, "init context");
	PERF_CHECK_BEGIN(LVL1, "creating base");

	ugd->base = create_fullview(conform);
	elm_object_content_set(conform, ugd->base);

	PERF_CHECK_END(LVL1, "creating base");

	if (ugd->base == NULL) {
		MSG_ERROR("Cannot create base view");
		ugd->bError = true;
		ugd->bErrMsg = strdup(IDS_UNABLE_TO_OPEN_FILE);
		goto ON_CREATE_ERROR;
	}

	int ux1, uy1, uw1, uh1;

	evas_object_geometry_get(ugd->base, &ux1, &uy1, &uw1, &uh1);

	MSG_MAIN_HIGH("UG base created : 0x%08x (%d,%d,%d,%d)", ugd->base, ux1, uy1, uw1, uh1);

	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_MOUSE_DOWN, _on_receive_mouse_down, NULL);

	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_RESIZE, _on_base_resize, ugd);
	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_MOVE, _on_base_move, NULL);

	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_SHOW, _on_base_show, NULL);
	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_HIDE, _on_base_hide, NULL);

#ifdef TRACE_WINDOW
	evas_object_event_callback_add(win, EVAS_CALLBACK_RESIZE, _on_win_resize, NULL);
	evas_object_event_callback_add(win, EVAS_CALLBACK_MOVE, _on_win_move, NULL);
#endif

	error_code = storage_set_state_changed_cb(__externalStorageId, _on_mmc_state_changed, ugd);
	if (error_code != STORAGE_ERROR_NONE) {
		MSG_MAIN_ERROR("storage_set_state_changed_cb() failed!!");
	}
#if 0
	if (ugd->ivug_param->mode == IVUG_MODE_SETAS && ugd->ivug_param->setas_type != IVUG_SET_AS_UG_TYPE_CALLER_ID && ugd->ivug_param->setas_type != IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP) {
#ifdef TEST_FOR_CROP
		ugd->ivug_param->setas_type = IVUG_SET_AS_UG_TYPE_CROP;
		ugd->ivug_param->width = 450;
		ugd->ivug_param->height = 300;
		ugd->ivug_param->bRatioFix = true;
#endif
		MSG_IMAGEVIEW_HIGH("UG types=%d", ugd->ivug_param->setas_type);

		switch (ugd->ivug_param->setas_type) {
		case IVUG_SET_AS_UG_TYPE_CALLER_ID:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_CALLER_ID");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_LOCKSCREEN:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_LOCKSCREEN");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP");
			return NULL;
			break;

		case IVUG_SET_AS_UG_TYPE_CROP:
			MSG_IMAGEVIEW_HIGH("IVUG_SET_AS_UG_TYPE_CROP");
			return NULL;
			break;
		default:
			MSG_IMAGEVIEW_ERROR("Invalid SetAS UG Type:%d", ugd->ivug_param->setas_type);
			return NULL;

		}
	} else {
		PERF_CHECK_BEGIN(LVL1, "main_view_create");

		ivug_set_indicator_visibility(false);		// Set indicator visibility false.
		ivug_set_indicator_overlap_mode(true);				// Set comformant as no indicator mode

		ugd->main_view = ivug_main_view_create(ugd->base, ugd->ivug_param);

		PERF_CHECK_END(LVL1, "main_view_create");

		if (ugd->main_view == NULL) {	//set main view.
			MSG_IMAGEVIEW_ERROR("Main View Layout Loading Fail");
			ugd->bError = true;
			ugd->bErrMsg = strdup("Layout Loading Fail");
			goto ON_CREATE_ERROR;
		}

		if (ugd->ivug_param->bTestMode == true) {
			ivug_main_view_set_testmode(ugd->main_view, true);
		}

// Load list.
		PERF_CHECK_BEGIN(LVL1, "main_view_set_list");

		if (ivug_main_view_set_list(ugd->main_view, ugd->ivug_param) == false) {
			MSG_IMAGEVIEW_ERROR("Cannot load media list.");
			// Need popup?
			if (ugd->ivug_param->mode != IVUG_MODE_HIDDEN) {
				ugd->bError = true;
				ugd->bErrMsg = strdup(IDS_UNABLE_TO_OPEN_FILE);
				goto ON_CREATE_ERROR;
			}
		}

		Evas_Object *layout = ivug_main_view_object_get(ugd->main_view);
		elm_object_part_content_set(ugd->base, "elm.swallow.content", layout);	//swallow

		PERF_CHECK_END(LVL1, "main_view_set_list");
	}

	PERF_CHECK_END(LVL0, "On Create");

//	struct ug_data *ugd = (struct ug_data *)priv;

	PERF_CHECK_BEGIN(LVL0, "On Create -> On Start");

	evas_object_size_hint_weight_set(ugd->base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	int ux, uy, uw, uh;

	evas_object_geometry_get(ugd->base, &ux, &uy, &uw, &uh);

	MSG_IMAGEVIEW_HIGH("Image Viewer : %s Base(0x%08x) Geometry(%d,%d,%d,%d)", __func__, ugd->base, ux, uy, uw, uh);

	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_DEL, _on_base_deleted, ugd);

	return ugd->base;
#endif
ON_CREATE_ERROR:

	if (ugd->base == NULL) {
		ugd->base = elm_layout_add(win);
		elm_layout_theme_set(ugd->base, "layout", "application", "default");
	}

	return true;
}
#if 0
void on_start(ui_gadget_h ug, app_control_h service, void *priv)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer BEGIN %s, ug=0x%08x, data=0x%08x", __func__, ug, priv);

	PERF_CHECK_END(LVL0, "On Create -> On Start");

	PERF_CHECK_BEGIN(LVL0, "On Start");

	if (!ug || !priv) {
		MSG_IMAGEVIEW_ERROR("Invalid UG. UG=0x%08x Priv=0x%08x", ug, priv);
		return ;
	}

	ug_data *ugd = (ug_data *)priv;

	int ux, uy, uw, uh;

	evas_object_geometry_get(ugd->base, &ux, &uy, &uw, &uh);

	MSG_IMAGEVIEW_HIGH("Image Viewer : %s BaseGeometry(%d,%d,%d,%d)", __func__, ux, uy, uw, uh);

	if (ugd->bError == true) {
		PERF_CHECK_END(LVL0, "On Start");
		MSG_IMAGEVIEW_ERROR("UG create has ERROR");
		notification_status_message_post(GET_STR(IDS_UNABLE_TO_OPEN_FILE));
//		ug_destroy_me(gGetUGHandle());
		return;
	}

	if (ugd->main_view) {
		PERF_CHECK_BEGIN(LVL1, "main_view_start");
		ivug_main_view_start(ugd->main_view, service);
		PERF_CHECK_END(LVL1, "main_view_start");
	} else if (ugd->crop_ug) {
		ivug_crop_ug_start(ugd->crop_ug);
	}

	ivug_add_reg_idler(ugd->main_view);

	PERF_CHECK_END(LVL0, "On Start");

	MSG_IMAGEVIEW_HIGH("Image Viewer END:%s, ug=0x%08x, data=0x%08x", __func__, ug, priv);

	if (ugd->ivug_param->bStandalone == true) {
		_send_result_to_caller(ug);
	}
}
#endif
void on_pause( void *priv)
{
	MSG_MAIN_HIGH("Image Viewer : %s, data=0x%08x", __func__, priv);

	if (!priv) {
		MSG_MAIN_HIGH("Invalid UG. Priv=0x%08x", priv);
		return ;
	}

	if (gGetDestroying() == true) {
		MSG_MAIN_HIGH("Image Viewer is destroying");
		return;
	}

	ug_data *ugd = (ug_data *)priv;

	if (ugd->ivug_param == NULL) {
		MSG_MAIN_HIGH("UG is destroying");
		return;
	}

	if (ugd->main_view) {
		ivug_main_view_pause(ugd->main_view);
	} else {
		MSG_MAIN_HIGH("don't need to pause");
	}
}

void on_resume(void *priv)
{
	MSG_MAIN_HIGH("Image Viewer : %s, data=0x%08x", __func__, priv);

	if (!priv) {
		MSG_MAIN_SEC("Invalid UG. Priv=0x%08x",priv);
		return ;
	}

	ug_data *ugd = (ug_data *)priv;

	if (ugd->main_view) {
		ivug_main_view_resume(ugd->main_view);
	} else {
		MSG_MAIN_HIGH("don't need to resume");
	}

}

void on_destroy(void *priv)
{
	MSG_MAIN_HIGH("Image Viewer : %s(0x%08x) data=0x%08x", __func__, on_destroy, priv);

	PERF_CHECK_BEGIN(LVL0, "On Destroy");

	if (!priv) {
		MSG_MAIN_ERROR("Invalid UG. Priv=0x%08x",priv);
		return ;
	}

	ug_data *ugd = (ug_data *)priv;

	if (ugd->bErrMsg) {
		free(ugd->bErrMsg);
		ugd->bErrMsg = NULL;
	}

	if (ugd->crop_ug) {
		ivug_crop_ug_destroy(ugd->crop_ug);
		ugd->crop_ug = NULL;
	}

	//destroy main view.
	if (ugd->main_view) {
		PERF_CHECK_BEGIN(LVL1, "MainView");
		ivug_main_view_destroy(ugd->main_view);
		ugd->main_view = NULL;
		PERF_CHECK_END(LVL1, "MainView");
	}

	if (ugd->ss_view) {
		PERF_CHECK_BEGIN(LVL1, "SlideShowView");
		ivug_slideshow_view_destroy(ugd->ss_view);
		ugd->ss_view = NULL;
		PERF_CHECK_END(LVL1, "SlideShowView");
	}

	//delete param.
	if (ugd->ivug_param) {
		ivug_param_delete(ugd->ivug_param);
		ugd->ivug_param = NULL;
	}

	//finalize data
	PERF_CHECK_BEGIN(LVL1, "Context");
	if (!ivug_context_deinit()) {
		MSG_IMAGEVIEW_ERROR("ivug_main_deinit failed");
	}
	PERF_CHECK_END(LVL1, "Context");

	if (ugd->base) {
		PERF_CHECK_BEGIN(LVL1, "Base layout");
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_DEL, _on_base_deleted);
		evas_object_del(ugd->base);
		ugd->base = NULL;
		PERF_CHECK_END(LVL1, "Base layout");
	}

	MSG_MAIN_HIGH("Destroyed ug");

	PERF_CHECK_END(LVL0, "On Destroy");
}
#if 0
static bool _data_print(app_control_h service, const char *key, void *user_data)
{
	char *value;

	app_control_get_extra_data(service, key, &value);

	MSG_IVUG_HIGH("  %s : %s", key, value);

	free(value);

	return true;
}

void on_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s UG=0x%08x", __func__, ug);	//on message

	if (!ug || !priv) {
		MSG_IMAGEVIEW_ERROR("Invalid UG. UG=0x%08x Priv=0x%08x", ug, priv);
		return;
	}

	int ret = app_control_foreach_extra_data(msg, _data_print, NULL);

	if (APP_CONTROL_ERROR_NONE != ret) {
		MSG_IVUG_ERROR("app_control_foreach_extra_data ERROR");
	}

	ug_data *ugd = (ug_data *)priv;

	//ivug_msg_type msg_type = IVUG_MSG_NONE;

	if (ugd->main_view == NULL) {
		MSG_IMAGEVIEW_ERROR("main view is NULL");
		return;
	}

	char *value = NULL;
	app_control_get_extra_data(msg, "Destroy", &value);
	if (value != NULL) {
		MSG_IMAGEVIEW_HIGH("_destroy_manually msg received");
		_destroy_manually(priv);
		free(value);
		return;
	}

	ivug_callback_call(gGetCallbackHandle(), msg, NULL, NULL);

	return;
}

void on_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv)
{
	if (!ug || !priv) {
		MSG_IMAGEVIEW_ERROR("Invalid UG. UG=0x%08x Priv=0x%08x", ug, priv);
		return;
	}
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s UG=0x%08x", __func__, ug);	//on message

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		MSG_IMAGEVIEW_HIGH("Get Event : Low Memory");
		break;
	case UG_EVENT_LOW_BATTERY:
		MSG_IMAGEVIEW_HIGH("Get Event : Low battery");
		break;
	case UG_EVENT_LANG_CHANGE:
		MSG_IMAGEVIEW_HIGH("Get Event : Language changed");
		ivug_language_mgr_update(gGetLanguageHandle());
		break;

		// Rotate event is not used now.. plz, use only resized callback.
	case UG_EVENT_ROTATE_PORTRAIT:
		MSG_IMAGEVIEW_HIGH("Get Event : rotate portrait");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		MSG_IMAGEVIEW_HIGH("Get Event : rotate landscape");
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		MSG_IMAGEVIEW_HIGH("Get Event : rotate portrait upsidedown");
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		MSG_IMAGEVIEW_HIGH("Get Event : rotate landscape upsidedown");
		break;

	default:
		MSG_IMAGEVIEW_ERROR("Unknown event type : %d", event);
		break;
	}
}

void on_destroying(app_control_h service, void *priv)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s", __func__);

	if (!priv) {
		MSG_IMAGEVIEW_ERROR("Invalid UG. Priv=0x%08x", priv);
		return;
	}

	int error_code = storage_unset_state_changed_cb(__externalStorageId, _on_mmc_state_changed);
	if (error_code != STORAGE_ERROR_NONE) {
		MSG_MAIN_ERROR("storage_unset_state_changed_cb() failed!!");
	}

	gSetDestroying(true);
}
#endif

void _language_changed_cb(void *user_data)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s", __func__);

}
