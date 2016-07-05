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

void FreeUGData(ug_data *ug)
{
	IV_ASSERT(ug != NULL);

	free(ug);
}

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

static void _on_mmc_state_changed(int storage_id, storage_dev_e dev, storage_state_e state,
								  const char *fstype, const char *fsuuid, const char *mountpath,
								  bool primary, int flags, void *user_data)
{
	ug_data *ugd = (ug_data *)user_data;
	IV_ASSERT(ugd != NULL);

	if (state == STORAGE_STATE_REMOVED) {
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
	} else if (state == STORAGE_STATE_MOUNTED){
		MSG_IMAGEVIEW_WARN("MMC Inserted!");
	}
}

static void _on_base_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	MSG_IMAGEVIEW_ERROR("_on_base_deleted");
}

Evas_Object *create_layout(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);

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

	error_code = storage_set_changed_cb(STORAGE_TYPE_EXTERNAL, _on_mmc_state_changed, ugd);
	if (error_code != STORAGE_ERROR_NONE) {
		MSG_MAIN_ERROR("storage_set_state_changed_cb() failed!!");
	}

ON_CREATE_ERROR:

	if (ugd->base == NULL) {
		ugd->base = elm_layout_add(win);
		elm_layout_theme_set(ugd->base, "layout", "application", "default");
	}

	return true;
}

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
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s(0x%08x) data=0x%08x", __func__, on_destroy, priv);

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
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_MOUSE_DOWN, _on_receive_mouse_down);
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_RESIZE, _on_base_resize);
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_MOVE, _on_base_move);
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_SHOW, _on_base_show);
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_HIDE, _on_base_hide);
		evas_object_event_callback_del(ugd->base, EVAS_CALLBACK_DEL, _on_base_deleted);

		evas_object_del(ugd->base);
		ugd->base = NULL;
		PERF_CHECK_END(LVL1, "Base layout");
	}

	MSG_IMAGEVIEW_HIGH("Destroyed ug");

	PERF_CHECK_END(LVL0, "On Destroy");
}

void _language_changed_cb(void *user_data)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer : %s", __func__);

}
