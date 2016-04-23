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

//#include <ui-gadget-module.h>		// ug_destroy_me
#include <algorithm>    // std::swap

#include <pthread.h>

#include "ivug-common.h"
#include "ivug-slider-new.h"

#include "ivug-parameter.h"
#include "ivug-define.h"
#include "ivug-config.h"

#include "ivug-util.h"
//#include "ivug-medialist.h"
#include "ivug-media.h"

#include "ivug-decolayer.h"

#include "ivug-main-view.h"
#include "../view/ivug-main-view-priv.h"

#define ICON_PLAY_SIZE 100
#define LOGNTAP_THRESHOLD	(100)		// allow moving distance(pixel)
#define TILT_THRESHOLD	(3)		// allow tilt value

#undef 	LOG_LVL
#define LOG_LVL (DBG_MSG_LVL_HIGH)

#undef 	LOG_CAT
#define LOG_CAT "IV-QSLIDER"

#define SLIDER_NEW_EDJ_FILE EDJ_PATH"/ivug-slider-new.edj"
#define SLIDER_NEW_HD_EDJ_FILE EDJ_PATH"/ivug-slider-new-hd.edj"
typedef struct _Ivug_SliderNew {
	Evas_Object *parent;
	Evas_Object *layout;
	Evas_Object *gesture;
	Evas_Object *photocam;

	int prev_x;
	int prev_y;

	int curpc;
	bool curpcm_state;//current photocam move state;
	bool pcreset; //photocam reset
	bool bLongtapEnable;
	bool bMomentumStarted;

	int longtap_count;
	int longtap_prev_x;
	int longtap_prev_y;

	double  zoom_level;
	int  zoom_internal_level;
	int max_zoom_level;

	bool bZooming;
	bool bSliding;

	int c_x;	// zoom center x
	int c_y;	// zoom center x

	double zoom_factor_prev;

	slider_mode_e mode;

// Have same size as Imange internal's
	Evas_Object *icon_layer;		// Decoration layer. VideoIcon/BestPicIcon/Sound&Pic icon

	Media_List *mList;

	callback_t changed_cb;
	void *changed_data;

	callback_t loaded_cb;
	void *loaded_data;

	location_callback_t location_cb;
	void *location_data;

	pthread_t mainTID;
	bool bDeleting;

	int orientation;
	Ivug_MainView *pMainView;
} Ivug_SliderNew;


#if (1)

static void _on_obj_resize(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

#if 1
	if (w == 0 || h == 0) {
		MSG_WARN("Not ready for update XYWH(%d,%d,%d,%d)", x, y, w, h);
		return;
	}

	if (slider_new->icon_layer) {
		evas_object_show(slider_new->icon_layer);
	}
#endif

	MSG_HIGH("QSlider(0x%08x) resized geomtery XYWH(%d,%d,%d,%d) angle=%d", obj, x, y, w, h, elm_win_rotation_get(slider_new->pMainView->window));
}

static void _on_obj_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;

	Evas_Coord x, y, w, h;
	evas_object_geometry_get(obj, &x, &y, &w, &h);

#if 1
	if (w == 0 || h == 0) {
		MSG_WARN("Not ready for update XYWH(%d,%d,%d,%d)", x, y, w, h);
		return;
	}

	if (slider_new->icon_layer) {
		evas_object_show(slider_new->icon_layer);
	}
#endif

	MSG_HIGH("QSlider(0x%08x) moved geomtery XYWH(%d,%d,%d,%d) angle=%d", obj, x, y, w, h, elm_win_rotation_get(slider_new->pMainView->window));
}


static void _on_obj_show(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("QSlider(0x%08x) layout show", obj);
}

static void _on_obj_hide(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("QSlider(0x%08x) layout hide", obj);
}
#endif

void ivug_slider_update_icon_layer(Ivug_SliderNew *slider_new)
{

	int x, y, w, h;
	evas_object_geometry_get(slider_new->photocam, &x, &y, &w, &h);
	evas_object_move(slider_new->icon_layer, (w / 2 - ICON_PLAY_SIZE / 2) , (h / 2 - ICON_PLAY_SIZE / 2));
	evas_object_resize(slider_new->icon_layer, ICON_PLAY_SIZE, ICON_PLAY_SIZE);
	Media_Item *mitem = ivug_medialist_get_current_item(slider_new->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (mdata->slide_type == SLIDE_TYPE_VIDEO
	   ) {
		ivug_decolayer_set_type(slider_new->icon_layer, IVUG_DECO_VIDEO);
	} else if (mdata->iType == MIMAGE_TYPE_BESTSHOT) {
		ivug_decolayer_set_type(slider_new->icon_layer, IVUG_DECO_BESTPIC);
	} else if (mdata->iType == MIMAGE_TYPE_SOUNDSCENE) {
		ivug_decolayer_set_type(slider_new->icon_layer, IVUG_DECO_SOUNDPIC);
	} else if (mdata->m_handle && ivug_db_is_burstshot(mdata->m_handle) == true) {
		ivug_decolayer_set_type(slider_new->icon_layer, IVUG_DECO_BURST);
	} else {
		ivug_decolayer_set_type(slider_new->icon_layer, IVUG_DECO_NONE);
	}

}


typedef struct _SliderCB {
	Ivug_SliderNew *slider_new;
	int x, y, w, h;		// Location CB
	Media_Item *mItem;	// Changed CB

	void *handle;
} SliderCB;

// Item changed.
static void _ivug_slider_new_changed_cb(void *handle, Media_Item *mItem, void *data)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;

	if (slider_new->changed_cb) {
		MSG_LOW("START : Item Changed");
		slider_new->changed_cb(NULL, mItem, slider_new->changed_data);
		MSG_LOW("END   : Item Changed");
	}

	if (mItem) {
		ivug_slider_update_icon_layer(slider_new);
	}

	MSG_HIGH("Slider Item Changed. mItem=0x%08x", mItem);

	return;
}

static Evas_Event_Flags _momentum_start(void *data , void *event_info)
{
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _momentum_move(void *data , void *event_info)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;

	MSG_LOW("No of Fingers = %d", p->n);

	if (p->n >= 2) {
		slider_new->pMainView->bmultitouchsliding = true;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _momentum_end(void *data , void *event_info)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;

	if (p->n >= 2) {
		slider_new->pMainView->bmultitouchsliding = false;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _momentum_abort(void *data , void *event_info)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	Elm_Gesture_Momentum_Info *p = (Elm_Gesture_Momentum_Info *) event_info;

	if (p->n >= 2) {
		slider_new->pMainView->bmultitouchsliding = false;
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags n_finger_tap_end(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

	MSG_MED("Finger tab end. Time=%d", p->timestamp);

	if (p->n != 1) {
		return EVAS_EVENT_FLAG_NONE;
	}

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	ivug_deco_icon_e icon;

	icon = ivug_decolayer_check_icon(slider_new->icon_layer, p->x, p->y);

	if (icon == IVUG_DECO_ICON_VIDEO) {
		evas_object_smart_callback_call(slider_new->layout, "slider,playvideo", slider_new); // Clicked video icon
	} else if (icon == IVUG_DECO_ICON_SOUNDPIC) {
		if (slider_new->icon_layer) {
			ivug_decolayer_start_blinking(slider_new->icon_layer);
		}
	} else if (icon == IVUG_DECO_ICON_BURST_PLAY) {
		MSG_HIGH("Burst play!!!!");
		evas_object_smart_callback_call(slider_new->layout, "slider,playburst", slider_new); // Clicked video icon
	}
	/*	else if (icon == IVUG_DECO_ICON_BURST_PLAYSPEED)
		{
			MSG_HIGH("Burst play speed");
			Evas_Object *popup = ivug_playspeed_popup_add(slider_new->layout);

			evas_object_smart_callback_add(popup, "playspeed,changed", _on_playspeed_changed, slider_new);
		}*/
	else {
		evas_object_smart_callback_call(slider_new->layout, "slider,clicked", slider_new);
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _dbl_click_start(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	MSG_HIGH("Double Tap Start :: No of fingers: %d", p->n);
	if (p->n != 1) {
		return EVAS_EVENT_FLAG_NONE;
	}
	slider_new->bSliding = false;
	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _dbl_click_end(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

	MSG_HIGH("Double Tap End :: No of fingers: %d, CenterPointXY(%d,%d)", p->n, p->x, p->y);
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	if (!slider_new->bZooming && slider_new->photocam) {
		slider_new->bZooming = true;
		slider_new->bSliding = false;
		slider_new->zoom_level = elm_photocam_zoom_get(slider_new->photocam);
		elm_photocam_zoom_mode_set(slider_new->photocam,  ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
		elm_photocam_zoom_set(slider_new->photocam, slider_new->zoom_level - 0.4);
	} else {

		slider_new->bSliding = true ;
		ivug_reset_zoom(slider_new);
	}

	return EVAS_EVENT_FLAG_NONE;
}

static Evas_Event_Flags _dbl_click_abort(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
	MSG_HIGH("Double Tap Abort :: No of fingers: %d", p->n);

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	if (!slider_new->bZooming) {
		slider_new->bSliding = true ;
	}

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags n_long_tap_start(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

	MSG_HIGH("Long tab start, x=%d, y=%d", p->x, p->y);

	if (p->n != 1) {
		return EVAS_EVENT_FLAG_NONE;
	}

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);

	slider_new->longtap_count = 0;

	return EVAS_EVENT_FLAG_NONE;
}

void ivug_slider_set_current_Photocam(Ivug_SliderNew *slider_new, int pc)
{
	slider_new->curpc = pc;
}
void ivug_slider_set_Photocam_moved(Ivug_SliderNew *slider_new, bool pcm)
{
	slider_new->curpcm_state = pcm;
}

Evas_Event_Flags _zoom_start(void *data, void *event_info)
{
	Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
	MSG_LOW("zoom start <%d,%d> <%f>", p->x, p->y, p->zoom);

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);
	MSG_ASSERT(slider_new->pMainView != NULL);
	if (slider_new->pMainView->slide_state) {
		MSG_MAIN_HIGH("Sliding, no Zoom should happen");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (p->zoom == 1.000000) {
		slider_new->curpcm_state = false;
	}

	if (slider_new->curpcm_state == true) {
		MSG_MAIN_HIGH("_zoom_start");
		slider_new->bSliding = false ;
		slider_new->curpcm_state = false;
		slider_new->pcreset = true;

		if (slider_new->curpc  == 0) {
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_stop", "imageview_area_temp0");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_right", "imageview_area");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_left", "imageview_area_temp2");
		} else if (slider_new->curpc == 1) {
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_stop", "imageview_area");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_right", "imageview_area_temp2");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_left", "imageview_area_temp0");
		} else if (slider_new->curpc == 2) {
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_stop", "imageview_area_temp2");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_right", "imageview_area_temp0");
			edje_object_signal_emit(elm_layout_edje_get(slider_new->layout), "set_left", "imageview_area");
		}
	}

	if (!slider_new->bZooming) {
		//elm_photocam_zoom_mode_set(slider_new->photocam,  ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
		//lider_new->bZooming = true;
		//slider_new->zoom_level = elm_photocam_zoom_get(slider_new->photocam);
		int width, height;
		elm_photocam_image_size_get(slider_new->photocam, &width, &height);
		slider_new->zoom_internal_level = 0;
		MSG_MAIN_HIGH(" width= %d  height = %d", width, height);
		if (width <= 50 && height <= 50) {
			slider_new->max_zoom_level = 3;
		} else if (width <= 200 && height <= 200) {
			slider_new->max_zoom_level = 5;
		} else if (width <= 500 && height <= 500) {
			slider_new->max_zoom_level = 8;
		} else if (width <= 1000 && height <= 1000) {
			slider_new->max_zoom_level = 12;
		} else if (width > 1000 && height > 1000) {
			slider_new->max_zoom_level = 20;
		} else {
			slider_new->max_zoom_level = 5;
		}
	}

	slider_new->c_x = p->x;
	slider_new->c_y = p->y;
	slider_new->zoom_factor_prev =  p->zoom ;

	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags _zoom_move(void *data, void *event_info)
{
	Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
	MSG_LOW("zoom move <%d,%d> <%f>", p->x, p->y, p->zoom);
	MSG_MAIN_HIGH(" _zoom_move");
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	double zoom_level = elm_photocam_zoom_get(slider_new->photocam);
	MSG_ASSERT(slider_new != NULL);
	MSG_ASSERT(slider_new->pMainView != NULL);
	if (slider_new->pMainView->slide_state) {
		MSG_MAIN_HIGH("Sliding, no Zoom should happen");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (zoom_level <= 1.0) {
		elm_photocam_gesture_enabled_set(slider_new->photocam, EINA_TRUE);

		if (!slider_new->bZooming) {
			slider_new->bZooming = true;
			elm_photocam_zoom_mode_set(slider_new->photocam,  ELM_PHOTOCAM_ZOOM_MODE_MANUAL);
			slider_new->zoom_level = elm_photocam_zoom_get(slider_new->photocam);
		}
		zoom_level = elm_photocam_zoom_get(slider_new->photocam);
		double zoomfactordiff = (p->zoom - slider_new->zoom_factor_prev);

		if (zoomfactordiff >= 0.1) {
			if (slider_new->zoom_internal_level <= slider_new->max_zoom_level) {
				slider_new->zoom_factor_prev = p->zoom;
				slider_new->zoom_internal_level++;
				elm_photocam_zoom_set(slider_new->photocam, zoom_level - 0.2);
			}
		} else if (zoomfactordiff <= -0.1) {
			elm_photocam_zoom_mode_set(slider_new->photocam,  ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
			if (slider_new->zoom_internal_level > 0 &&  zoom_level  !=  slider_new->zoom_level) {
				slider_new->zoom_factor_prev = p->zoom;
				slider_new->zoom_internal_level--;
				elm_photocam_zoom_set(slider_new->photocam, zoom_level + 0.2);
			}
		}
	}

	slider_new->bSliding = false ;
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags _zoom_end(void *data, void *event_info)
{
	Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
	MSG_MAIN_HIGH("zoom end <%d,%d> <%f>", p->x, p->y, p->zoom);

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);
	MSG_ASSERT(slider_new->pMainView != NULL);
	if (slider_new->pMainView->slide_state /*|| slider_new->pMainView->is_moved*/) {
		MSG_MAIN_HIGH("Sliding, no Zoom should happen");
		return EVAS_EVENT_FLAG_NONE;
	}

	double zoom_level = elm_photocam_zoom_get(slider_new->photocam);
	if (zoom_level >= 1.0) {
		slider_new->bSliding = true;
	}

	if (slider_new->zoom_internal_level == 0 || zoom_level == slider_new->zoom_level) {
		ivug_reset_zoom(slider_new);

	}
	return EVAS_EVENT_FLAG_NONE;
}

Evas_Event_Flags _zoom_abort(void *data, void *event_info)
{
	Elm_Gesture_Zoom_Info *p = (Elm_Gesture_Zoom_Info *) event_info;
	MSG_LOW("zoom abort <%d,%d> <%f>", p->x, p->y, p->zoom);

	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)data;
	MSG_ASSERT(slider_new != NULL);
	MSG_ASSERT(slider_new->pMainView != NULL);
	if (slider_new->pMainView->slide_state) {
		MSG_MAIN_HIGH("Sliding, no Zoom should happen");
		return EVAS_EVENT_FLAG_NONE;
	}
	return EVAS_EVENT_FLAG_NONE;
}

void ivug_disable_gesture(Ivug_SliderNew *slider_new)
{
	MSG_ASSERT(slider_new != NULL);

	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam0, EINA_FALSE);
	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam, EINA_FALSE);
	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam2, EINA_FALSE);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_START, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_MOVE, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT, NULL, slider_new);


	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, NULL, slider_new);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_START, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_END, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_ABORT, NULL, slider_new);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, NULL, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT, NULL, slider_new);

}

void ivug_enable_gesture(Ivug_SliderNew *slider_new)
{
	MSG_ASSERT(slider_new != NULL);

	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam0, EINA_TRUE);
	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam, EINA_TRUE);
	elm_photocam_gesture_enabled_set(slider_new->pMainView->photocam2, EINA_TRUE);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_START, _momentum_start, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_MOVE, _momentum_move, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END, _momentum_end, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT, _momentum_abort, slider_new);


	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, n_finger_tap_end, slider_new);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_START, _dbl_click_start, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_END, _dbl_click_end, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_ABORT, _dbl_click_abort, slider_new);

	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, _zoom_start, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, _zoom_move, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, _zoom_end, slider_new);
	elm_gesture_layer_cb_set(slider_new->gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT, _zoom_abort, slider_new);
}

Ivug_SliderNew * ivug_slider_new_init(Evas_Object *parent, void *pMainView)
{
	Ivug_SliderNew *slider_new = (Ivug_SliderNew *)calloc(1, sizeof(Ivug_SliderNew));
	if (slider_new == NULL) {
		MSG_ERROR("Cannot allocate memory");
		return NULL;
	}

	PERF_CHECK_BEGIN(LVL3, "create slider layout");

	slider_new->parent = parent;
	slider_new->mainTID = pthread_self();
	MSG_WARN("main tid = 0x%08x", slider_new->mainTID);
	slider_new->bDeleting = false;
	slider_new->pMainView = (Ivug_MainView *)pMainView;
	slider_new->bSliding = true;


	Evas_Object *win = slider_new->pMainView->window;
	int wx, wy, ww, wh;

	evas_object_geometry_get(win, &wx, &wy, &ww, &wh);
	if (ww == 720 && wh == 1280) {
		MSG_WARN("HD Loading");
		slider_new->layout = ivug_layout_add2(parent, SLIDER_NEW_HD_EDJ_FILE, "slider_new");
	} else {
		MSG_WARN("WVGA Loading");
		slider_new->layout = ivug_layout_add2(parent, SLIDER_NEW_EDJ_FILE, "slider_new");
	}
	if (slider_new->layout == NULL) {
		MSG_WARN("layout sawllow failed");
		free(slider_new);
		return NULL;
	}

	evas_object_name_set(slider_new->layout, "Slider new");
//	evas_object_propagate_events_set(slider_new->layout, EINA_FALSE);

	Evas_Object *event = const_cast<Evas_Object *>(edje_object_part_object_get(_EDJ(slider_new->layout), "slider.event"));
	MSG_ASSERT(event != NULL);

	//evas_object_event_callback_add(event, EVAS_CALLBACK_MOUSE_DOWN, _on_event_mouse_down, (void *)slider_new);
	//evas_object_event_callback_add(event, EVAS_CALLBACK_MOUSE_UP, _on_event_mouse_up, (void *)slider_new);

	slider_new->bLongtapEnable = true;

	Evas_Object *gesture = elm_gesture_layer_add(slider_new->layout);
	elm_gesture_layer_hold_events_set(gesture, EINA_FALSE);

	if (elm_gesture_layer_attach(gesture, event) == EINA_FALSE) {
		MSG_ERROR("Cannot attach event rect");
	}

	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_START, _momentum_start, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_MOMENTUM , ELM_GESTURE_STATE_MOVE, _momentum_move, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_END, _momentum_end, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_MOMENTUM, ELM_GESTURE_STATE_ABORT, _momentum_abort, slider_new);


	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, n_finger_tap_end, slider_new);

	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_START, _dbl_click_start, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_END, _dbl_click_end, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_ABORT, _dbl_click_abort, slider_new);
//	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_MOVE, _dbl_click_end, slider_new);


/*
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_START, n_long_tap_start, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_MOVE, n_long_tap_move, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_END, n_long_tap_end, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_N_LONG_TAPS, ELM_GESTURE_STATE_ABORT, n_long_tap_abort, slider_new);
*/
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_START, _zoom_start, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_MOVE, _zoom_move, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_END, _zoom_end, slider_new);
	elm_gesture_layer_cb_set(gesture, ELM_GESTURE_ZOOM, ELM_GESTURE_STATE_ABORT, _zoom_abort, slider_new);

	slider_new->gesture = gesture;

	{
		slider_new->icon_layer = ivug_decolayer_add(slider_new->layout);
		evas_object_name_set(slider_new->icon_layer, "Decolayer");

		evas_object_pass_events_set(slider_new->icon_layer, EINA_FALSE);		// For pressed icon display
		evas_object_repeat_events_set(slider_new->icon_layer, EINA_TRUE);

		evas_object_smart_member_add(slider_new->icon_layer, _EDJ(slider_new->layout));
	}

#if 1
	int x, y, w, h;
	evas_object_geometry_get(slider_new->pMainView->window, &x, &y, &w, &h);
#else
// \B0\A1\B7η\CE \BD\C3\C0\DB\C7ϴ\C2 \B0\E6\BF\EC \B5\BF\C0\DB \C0̻\F3\C7\D4.
	int w, h;
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h); 	// Portrait size.
#endif

	PERF_CHECK_END(LVL3, "create slider layout");

	PERF_CHECK_BEGIN(LVL3, "QPhotoAPI::create()");

	//Qphoto_init();

#if (1)
	evas_object_event_callback_add(slider_new->layout, EVAS_CALLBACK_RESIZE, _on_obj_resize, slider_new);
	evas_object_event_callback_add(slider_new->layout, EVAS_CALLBACK_MOVE, _on_obj_move, slider_new);

	evas_object_event_callback_add(slider_new->layout, EVAS_CALLBACK_SHOW, _on_obj_show, NULL);
	evas_object_event_callback_add(slider_new->layout, EVAS_CALLBACK_HIDE, _on_obj_hide, NULL);
#endif

	return slider_new;
}

void ivug_slider_new_destroy(Ivug_SliderNew * slider_new)
{
	IV_ASSERT(slider_new != NULL);

	slider_new->bDeleting = true;

	if (slider_new->gesture) {
		evas_object_del(slider_new->gesture);
		slider_new->gesture = NULL;
	}

	if (slider_new->icon_layer) {
		evas_object_del(slider_new->icon_layer);
		slider_new->icon_layer = NULL;
	}

	MSG_HIGH("Qphoto destroy. [END]");

	if (slider_new->layout) {
		evas_object_del(slider_new->layout);
		slider_new->layout = NULL;
	}

	free(slider_new);
}

void ivug_slider_new_set_mode(Ivug_SliderNew * slider_new, slider_mode_e mode)
{
	slider_new->mode = mode;
}

slider_mode_e ivug_slider_new_get_mode(Ivug_SliderNew * slider_new)
{
	return slider_new->mode;
}

void ivug_slider_new_set_list(Ivug_SliderNew * slider_new, Media_List *mList, Media_Item *current)
{
	slider_new->mList = mList;
}

void ivug_slider_new_set_photocam(Ivug_SliderNew *slider_new, Evas_Object * photocam)
{
	slider_new->photocam = photocam;
}

bool ivug_slider_new_is_zoomed(Ivug_SliderNew *slider_new)
{
	return FALSE;
}

void ivug_slider_new_enable_motion_pan(Ivug_SliderNew *slider_new, bool pan_state)
{
	// set motion pan
}

Evas_Object * ivug_slider_new_get_layout(Ivug_SliderNew *slider_new)
{
	return slider_new->layout;
}

Evas_Object * ivug_slider_new_get_gesture(Ivug_SliderNew *slider_new)
{
	return slider_new->gesture;
}

void  ivug_reset_zoom(Ivug_SliderNew *slider_new)
{
	slider_new->bZooming = false;
	elm_photocam_zoom_mode_set(slider_new->photocam,  ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);

}

bool  ivug_iszoom_enabled(Ivug_SliderNew *slider_new)
{
	return slider_new->bZooming;
}

bool  ivug_isslide_enabled(Ivug_SliderNew *slider_new)
{
	return slider_new->bSliding;
}

bool  ivug_isphotocam_reset(Ivug_SliderNew *slider_new)
{
	return slider_new->pcreset;
}

void  ivug_set_photocam_reset(Ivug_SliderNew *slider_new)
{
	slider_new->pcreset = false;
}

void ivug_slider_new_move_item(Ivug_SliderNew *slider_new, Media_Item *item)
{

}

void ivug_slider_new_reload(Ivug_SliderNew *slider_new)
{
	MSG_HIGH("QSlider(0x%08x) Reload", slider_new);
}

void ivug_slider_new_region_get(Ivug_SliderNew *slider_new, int *x, int *y, int *w, int *h)
{

}

void ivug_slider_new_region_image_get(Ivug_SliderNew *slider_new, int x, int y, int w, int h)
{
	MSG_ASSERT(0);

//	Bitmap *bitmap = NULL;
//	bitmap = slider_new->qphoto->Qphoto_region_image_get(x, y, w, h);		// Current display coord.
}

int ivug_slider_new_get_cur_index(Ivug_SliderNew *slider_new)
{
	return 0;
}

void ivug_slider_new_change_view_size(Ivug_SliderNew *slider_new, int w, int h)
{

	int wx, wy, ww, wh;
	evas_object_geometry_get(slider_new->pMainView->window, &wx, &wy, &ww, &wh);

	int rot = elm_win_rotation_get(slider_new->pMainView->window);

	int screen_x = 0;
	int screen_y = 0;
	int screen_w = 0;
	int screen_h = 0;

	elm_win_screen_size_get(slider_new->pMainView->window, &screen_x, &screen_y, &screen_w, &screen_h);
	MSG_HIGH("screen_Size : Win(%d,%d,%d,%d)", screen_x, screen_y, screen_w, screen_h);

	if ((rot % 180) != 0) {
		std::swap(screen_w, screen_h);
	}

// Landscape Mode -> SIP visible --> Window geometry is 1280x729....
// Rotate in E-mail.. WH=1225x1280

	MSG_HIGH("Set new Size : Win(%d,%d,%d,%d) WH(%d,%d)", wx, wy, ww, wh, screen_w, screen_h);
}

void ivug_slider_new_delete_cur_image(Ivug_SliderNew *slider_new)
{

}

void ivug_slider_set_changed_callback(Ivug_SliderNew *slider_new, callback_t callback, void *data)
{
	MSG_HIGH("Slider(0x%08x) Set changed callback. CB=0x%08x", slider_new, callback);
	slider_new->changed_cb = callback;
	slider_new->changed_data = data;
}

void ivug_slider_call_changed_callback(Ivug_SliderNew *slider_new, Media_Item *cur_mItem)
{
	MSG_HIGH("Slider(0x%08x) call changed callback", slider_new);

	_ivug_slider_new_changed_cb(NULL, cur_mItem, slider_new);
}

void ivug_slider_set_loaded_callback(Ivug_SliderNew *slider_new, callback_t callback, void *data)
{
	MSG_HIGH("Slider(0x%08x) Set loaded callback. CB=0x%08x", slider_new, callback);
	slider_new->loaded_cb = callback;
	slider_new->loaded_data = data;
}

void ivug_slider_set_location_callback(Ivug_SliderNew *slider_new, location_callback_t callback, void *data)
{

}

void ivug_slider_new_image_size_get(Ivug_SliderNew *slider_new, int *w, int *h)
{
	MSG_HIGH("ivug_slider_new_image_size_get w=%d, h=%d", *w, *h);
}

void ivug_slider_new_update_list(Ivug_SliderNew *slider_new, Media_List *mList)
{
	ivug_media_list_free(slider_new->mList);

	slider_new->mList = mList;
}

void ivug_slider_new_set_orientation(Ivug_SliderNew *slider_new, int orientation)
{
	MSG_HIGH("ivug_slider_new_set_orientation orientation=%d", orientation);
	slider_new->orientation = orientation;
}

int ivug_slider_new_get_orientation(Ivug_SliderNew *slider_new)
{
	MSG_HIGH("ivug_slider_new_get_orientation orientation=%d", slider_new->orientation);
	return slider_new->orientation;
}

void ivug_slider_new_agif_enable(Ivug_SliderNew *slider_new, bool enable)
{

}

void ivug_slider_new_mouse_enable(Ivug_SliderNew *slider_new, bool enable)
{
	MSG_HIGH("Slider Mouse Enabled=%d", enable);
}

void ivug_slider_new_rotate(Ivug_SliderNew *slider_new, int degree)
{

}

float ivug_slider_new_get_zoom(Ivug_SliderNew *slider_new)
{
	float zoom = 0.0;

	MSG_ERROR("ivug_slider_new_get_zoom zoom=%f", zoom);
	return zoom;
}
void  ivug_slider_new_hide_play_icon(Ivug_SliderNew *slider_new)
{
	MSG_HIGH("Hiding play icon");
	ivug_decolayer_hide_play_icon(slider_new->icon_layer);
}

