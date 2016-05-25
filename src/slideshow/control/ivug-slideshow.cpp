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

#include <stdlib.h>

#include <efl_extension.h>

#include "ivug-string.h"
#include "ivug-anim.h"
#include "ivug-slideshow.h"
#include "ivug-slideshow-priv.h"

#include "ivug-debug.h"

#include "ivug-config.h"
#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-uuid.h"
#include "ivug-util.h"
#include "ivug-context.h"

#include <Elementary.h>
#include <device/power.h>
#include <assert.h>

#include "statistics.h"

#define DEFAULT_THUMBNAIL		"/opt/usr/share/media/.thumb/thumb_default.png"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_MED

#undef LOG_CAT
#define LOG_CAT "IV-SLIDESHOW"

/*
	Basic functionality is implemented.
	but codes have many bugs. plz check this.

	TODO:
		Resize is not handled.
		Mouse routine is not handled.
		Decoding faied is not processed.

		Not test except slide effect.

*/
/*
static inline char * ivug_get_resource_path() {
	char * path = app_get_resource_path();
	return path;
}

static inline char* full_path(char *str1, char *str2) {
	char path[1024] = {};
	snprintf(path, 1024, "%s%s", str1, str2);
	char *full_path = strdup(path);
	return full_path;
}

*/
#define UG_RES_PATH 		ivug_get_resource_path()
#define EDJ_PATH				full_path(UG_RES_PATH, "edje")
#define IVUG_SS_LY_EDJ_PATH 	full_path(EDJ_PATH, "/ivug-ss-ly.edj")

#define IVUG_IMAGE_BETWEEN_MARGIN 	(30)
#define IVUG_IMAGE_MOVE_MARGIN 			((int)IVUG_IMAGE_BETWEEN_MARGIN*0.8)
#define IVUG_IMAGE_SCROLL_MARGIN 		(5)

#define _EDJ(o)			elm_layout_edje_get(o)

struct st_temp {
	int index;
	SlideShow *pSlideshow;
};

void ivug_ss_get_screen_size(Evas_Object *win, int *width, int *height)
{
	int rotation = elm_win_rotation_get(win);


	int screen_x = 0;
	int screen_y = 0;
	int screen_w = 0;
	int screen_h = 0;

	elm_win_screen_size_get(win, &screen_x, &screen_y, &screen_w, &screen_h);
	MSG_HIGH("screen_Size : Win(%d,%d,%d,%d)", screen_x, screen_y, screen_w, screen_h);

	if (rotation == 0 || rotation == 180) {
		*width = screen_w;
		*height = screen_h;
	} else if (rotation == 90 || rotation == 270) {
		*width = screen_h;
		*height = screen_w;
	}
}

int _ivug_ss_get_sort(int *val)
{
	return 0;
}

Media_Item *ivug_ss_get_next_item(Media_List *mList,
                                  Media_Item *header,
                                  Media_Item *current,
                                  slide_show_mode mode)
{
	Media_Item *item = NULL;
	int sort_value = 0;

	MSG_HIGH("Get Next Item : Header=0x%08x Current=0x%08x", header, current);

	MSG_HIGH("Get Next : Header=0x%08x Current=0x%08x",
	         header, current);
	switch (mode) {
	case SLIDE_SHOW_MODE_REPEAT:
		_ivug_ss_get_sort(&sort_value);
		if (sort_value == 1) {
			item = ivug_medialist_get_prev(mList, current);
		} else {
			item = ivug_medialist_get_next(mList, current);
		}
		if (item == NULL) {
			if (sort_value == 1) {
				item = ivug_medialist_get_last(mList);
			} else {
				item = ivug_medialist_get_first(mList);
			}
		}
		break;

	case SLIDE_SHOW_MODE_NORMAL:
		_ivug_ss_get_sort(&sort_value);
		if (sort_value == 1) {
			item = ivug_medialist_get_prev(mList, current);
		} else {
			item = ivug_medialist_get_next(mList, current);
		}
		break;
/*
	case SLIDE_SHOW_MODE_SHUFFLE_REPEAT:
		item = ivug_medialist_get_shuffle_item(mList, current);
		if (item == NULL) {
			MSG_ERROR("Never touch here");
		}
		break;

	case SLIDE_SHOW_MODE_SHUFFLE:
		item = ivug_medialist_get_shuffle_item(mList, current);
		if (item == header) {
			MSG_ERROR("Reach end");
			return NULL;
		}
		break;
*/
	default:
		MSG_ERROR("Unknown mode : %d", mode);
		item = NULL;
		break;
	}

	return item;

}


static void
_moved(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	SlideShow *pSlideShow = (SlideShow *)data;

	Evas_Coord ox, oy, ow, oh;
	evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

	MSG_HIGH("Moved (%d,%d,%d,%d)", ox, oy, ow, oh);

	evas_object_move(pSlideShow->event, ox, oy);
}

static void
_resized(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	SlideShow *pSlideShow = (SlideShow *)data;

	Evas_Coord ox, oy, ow, oh;
	evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

	MSG_HIGH("Resized (%d,%d,%d,%d)", ox, oy, ow, oh);

	evas_object_resize(pSlideShow->event, ow, oh);

	evas_object_resize(pSlideShow->sLayout[pSlideShow->sCurrent].layout, ow, oh);
	evas_object_resize(pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout, ow, oh);
}

static void
_shown(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("SlideShow is Shown");
}

static void
_hidden(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("SlideShow is Hidden");
}

static bool _ivug_ss_set_content(Slide_Layout *pSlide, Media_Item *item)
{
	Media_Data *mdata;
	int ret = EVAS_LOAD_ERROR_NONE;

	pSlide->mitem = item;

	if (pSlide->mitem == NULL) {
		MSG_ERROR("Mitem is NULL");
		return false;
	}

	mdata = ivug_medialist_get_data(item);
	IV_ASSERT(mdata != NULL);

	MSG_SEC("Start Loading : %s", mdata->filepath);

	if (mdata->slide_type == SLIDE_TYPE_VIDEO) {
		ret = elm_photocam_file_set(pSlide->photocam, mdata->thumbnail_path);

		if (EVAS_LOAD_ERROR_NONE != ret) {
			MSG_HIGH("elm_photocam_file_set failed");
			return false;
		}
	} else {
		MSG_SEC("Photocam Object is %p  and File is %s", pSlide->photocam, mdata->filepath);
		ret = elm_photocam_file_set(pSlide->photocam, mdata->filepath);
		MSG_SEC("elm_photocam_file_set API envoked");
		if (EVAS_LOAD_ERROR_NONE != ret) {
			MSG_HIGH("elm_photocam_file_set failed. Loading default Thumbnail");
			elm_photocam_file_set(pSlide->photocam, DEFAULT_THUMBNAIL);
			return false;
		}
		if (elm_image_file_set(pSlide->thumbnail, mdata->thumbnail_path, NULL)
		        == EINA_FALSE) {
			MSG_ERROR("Cannot load thumbnail : %s", mdata->thumbnail_path);
		} else {
			edje_object_signal_emit(_EDJ(pSlide->layout),
			                        "elm,state,show_thumbnail", "slideshow");
		}
	}
	MSG_SEC("Load : %s", mdata->filepath);
	elm_photocam_zoom_mode_set(pSlide->photocam,
	                           ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
	elm_photocam_paused_set(pSlide->photocam, true);
	evas_object_size_hint_weight_set(pSlide->photocam, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);

	return true;
}

static void _ivug_ss_video_icon(Evas_Object *layout, Media_Item *mitem)
{
	Media_Data *mdata = ivug_medialist_get_data(mitem);

	if (mdata->slide_type == SLIDE_TYPE_IMAGE) {
		elm_object_signal_emit(layout, "elm,state,hide", "");
	} else {
		elm_object_signal_emit(layout, "elm,state,show", "");
	}

	return;
}


static void _ivug_ss_update_pos(SlideShow *pSlideShow, Evas_Coord x, Evas_Coord y)
{
	IV_ASSERT(pSlideShow != NULL);

	MSG_HIGH("Update Pos(%d,%d) sCurrent=%d", x, y, pSlideShow->sCurrent);

//	x = x - 360;

	Slide_Layout *sLyCurrent = &pSlideShow->sLayout[pSlideShow->sCurrent];
	Slide_Layout *sLyNext = &pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2];

	evas_object_move(sLyCurrent->layout, x, y);
	evas_object_move(sLyNext->layout,
	                 x + pSlideShow->screen_w + IVUG_IMAGE_BETWEEN_MARGIN ,
	                 y);
}
static bool _ivug_ss_load_next_image(SlideShow *pSlideShow)
{
	ivug_retv_if(!pSlideShow, false);
	MSG_HIGH("");

	Slide_Layout* sLyCurrent = &pSlideShow->sLayout[pSlideShow->sCurrent];
	Slide_Layout* sLy = &pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2];

	Media_Item *next = NULL;
	Media_Item *current = sLyCurrent->mitem;
	do {
		next = ivug_ss_get_next_item(pSlideShow->media_list,
		                             pSlideShow->ss_Header,
		                             current,
		                             pSlideShow->ss_mode);

		if (next == NULL) {
			sLy->mitem = NULL;
			return false;
		}
		current = next;
	} while (!_ivug_ss_set_content(sLy, next));

	if (next) {
		evas_object_show(sLy->layout);
	}

	return true;
}

void _ivug_ss_effect_finished(void *data)
{
	ivug_ret_if(!data);
	SlideShow *pSlideShow = (SlideShow *) data;
	MSG_HIGH("slideshow Effect ended");

	if (pSlideShow->effect_engine) {
		MSG_HIGH("ivug_effect_finalize");
		ivug_effect_finalize(pSlideShow->effect_engine);
		pSlideShow->effect_engine = NULL;
	}

	/* Increse current index */
	pSlideShow->sCurrent = (pSlideShow->sCurrent + 1) % 2;
	_ivug_ss_update_pos(pSlideShow, 0, 0);		// Reset position

	Slide_Layout *sLyCurrent = &pSlideShow->sLayout[pSlideShow->sCurrent];
	Slide_Layout *sLyNext = &pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2];

	Media_Item *next = NULL;

	next = ivug_ss_get_next_item(pSlideShow->media_list,
	                             pSlideShow->ss_Header,
	                             sLyCurrent->mitem,
	                             pSlideShow->ss_mode);

	if (next == NULL) {
		sLyNext->mitem = NULL;
		MSG_ERROR("Cannot find next item");
		return;
	}

	_ivug_ss_set_content(sLyNext, next);

// Reset visibilaty because effect can hide layout
	evas_object_show(sLyCurrent->layout);
	evas_object_show(sLyNext->layout);

	MSG_HIGH("pSlideShow->bSS_StopFlag = %d", pSlideShow->bSS_StopFlag);

	if (pSlideShow->bSS_StopFlag == EINA_TRUE) {
		pSlideShow->state = SLIDE_SHOW_STOPPED;
		pSlideShow->ss_Header = NULL;
		evas_object_smart_callback_call(ivug_ss_object_get(pSlideShow), "slideshow,finished", (void *)SLIDE_SHOW_STOPPED);
//		ivug_ss_delete(pSlideShow);

		pSlideShow->bSS_StopFlag = EINA_FALSE;
	}
	//EFL::dump_obj(pSlideShow->obj, 0);
}

Evas_Object *_ivug_ss_create_layout(Evas_Object *parent, const char *edj_path, const char *group)
{
	MSG_ASSERT(parent != NULL);
	MSG_ASSERT(edj_path != NULL);
	MSG_ASSERT(group != NULL);

	Evas_Object *ly = NULL;
	ly = elm_layout_add(parent);

	ivug_retv_if(!ly, NULL);

	if (elm_layout_file_set(ly , edj_path, group) == EINA_FALSE) {
		MSG_ERROR("Cannot create layout. %s %s", edj_path, group);
		evas_object_del(ly);
		return NULL;
	}
	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ly, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(ly);
	return ly;
}

// Slide Show related functions.
static Eina_Bool _ivug_ss_on_slide_interval(void *data)
{
	MSG_ASSERT(data != NULL);

	SlideShow *pSlideShow = (SlideShow*)data;

	MSG_HIGH("On Slide Interval");
	/* stopped by other operation */
	if (pSlideShow->state == SLIDE_SHOW_STOPPED) {
		pSlideShow->ss_timer = NULL;
		MSG_ERROR("Slide show already stopped");
		return ECORE_CALLBACK_CANCEL;
	}

	Slide_Layout *sLyCurrent = &pSlideShow->sLayout[pSlideShow->sCurrent];
	Slide_Layout *sLyNext = &pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2];

	Media_Item *mitem = sLyCurrent->mitem;
	_ivug_ss_video_icon(pSlideShow->sLayout[pSlideShow->sCurrent].layout, mitem);

	/* Next item is NULL */
	if (sLyNext->mitem == NULL) {
		MSG_HIGH("Next item is NULL");
		pSlideShow->ss_timer = NULL;
		pSlideShow->ss_Header = NULL;
		/* exit slide show after whole animation is over */
		ivug_ss_stop(pSlideShow);

		return ECORE_CALLBACK_CANCEL;
	}

	mitem = sLyNext->mitem;
	_ivug_ss_video_icon(pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout, mitem);

	/* Slideshow Effect */
	Effect_Engine *eng = ivug_effect_add(pSlideShow->effect_type);
	MSG_ASSERT(eng != NULL);

	pSlideShow->effect_engine = eng;

	ivug_effect_init(pSlideShow->effect_engine, sLyCurrent->layout, sLyNext->layout);

	int rotation = elm_win_rotation_get(gGetCurrentWindow());
	ivug_ss_get_screen_size(gGetCurrentWindow(), &pSlideShow->screen_w, &pSlideShow->screen_h);

	if (ivug_effect_set_size(pSlideShow->effect_engine, pSlideShow->screen_w, pSlideShow->screen_h, rotation) == false) {
		pSlideShow->ss_timer = NULL;
		ivug_effect_finalize(pSlideShow->effect_engine);
		return ECORE_CALLBACK_CANCEL;
	}

//	EFL::dump_obj(pSlideShow->obj, 0);

	pSlideShow->cur_item = sLyNext->mitem;

	ivug_effect_start(pSlideShow->effect_engine, _ivug_ss_effect_finished, pSlideShow);

	return ECORE_CALLBACK_RENEW;
}



static Effect_Type _ivug_ss_get_trans_effect(ivug_effect_type type)
{
	MSG_MED("type %d", type);
	switch (type) {
	case IVUG_EFFECT_TYPE_SLIDE:
		return EFFECT_SLIDE;
	case IVUG_EFFECT_TYPE_DISSOLVE_FADE:
		return EFFECT_DISSOLVE_FADE;
	default:
		break;
	}
	return EFFECT_NONE;
}




void _ivug_ss_on_mouse_down(void *data, Evas *e,
                            Evas_Object *obj, void *event_info)
{
	SlideShow *pSlideShow = (SlideShow *) data;

	MSG_HIGH("_ivug_ss_on_mouse_down, state is %d", pSlideShow->state);

	if (pSlideShow->click_timer) {
		ecore_timer_del(pSlideShow->click_timer);
		pSlideShow->click_timer = NULL;
	}
}

void _ivug_ss_on_mouse_move(void *data, Evas *e,
                            Evas_Object *obj, void *event_info)
{
	MSG_LOW("_ivug_ss_on_mouse_move");
	return;
}

static Eina_Bool _ivug_ss_clicked_timer_cb(void *data)
{
	MSG_HIGH("-------------_ivug_ss_clicked_timer_cb--------------");
	SlideShow *pSlideShow = (SlideShow *)data;
	if (NULL == pSlideShow) {
		return ECORE_CALLBACK_CANCEL;
	}

	pSlideShow->click_timer = NULL;

	if ((pSlideShow->state == SLIDE_SHOW_RUNNING) && (pSlideShow->bPlayButton == false)) {
		ivug_ss_pause(pSlideShow);
		return ECORE_CALLBACK_CANCEL;
	}
	pSlideShow->bPlayButton = false;

	return ECORE_CALLBACK_CANCEL;
}

void _ivug_ss_on_mouse_up(void *data, Evas *e,
                          Evas_Object *obj, void *event_info)
{
	SlideShow *pSlideShow = (SlideShow *)data;
	if (NULL == pSlideShow) {
		return;
	}
	MSG_HIGH("_ivug_ss_on_mouse_up, pSlideShow->v is %d", pSlideShow->state);

	if (pSlideShow->bMouse_event) {
		if (pSlideShow->click_timer) {
			ecore_timer_del(pSlideShow->click_timer);
			pSlideShow->click_timer = NULL;
		}
		pSlideShow->click_timer = ecore_timer_add(0.03f, _ivug_ss_clicked_timer_cb, data);
	}
}

static Eina_Bool
_ivug_ss_auto_finish_timer_cb(void *data)
{
	ivug_retv_if(!data, ECORE_CALLBACK_CANCEL);

	SlideShow *pSlideShow = (SlideShow*)data;
	/* exit slide show after whole animation is over */
	pSlideShow->ss_timer = NULL;

	ivug_ss_stop(pSlideShow);
	return ECORE_CALLBACK_CANCEL;
}


static void
_ivug_ss_photocam_loaded_cb(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Slide_Layout* sLy = static_cast<Slide_Layout*>(data);
	SlideShow *pSlideShow = (SlideShow *)sLy->pSlideshow;

	if (sLy->mitem == NULL) {
		MSG_ERROR("Debug Me! Data item is NULL.");
		return;
	}

	Media_Data* mdata = ivug_medialist_get_data(sLy->mitem);
	if (mdata == NULL) {
		MSG_ERROR("ivug_medialist_get_data failed.");
		return;
	}

//	Evas_Load_Error error = static_cast<Evas_Load_Error>(reinterpret_cast<int >(event_info));//check
	int *error = (int *)event_info;
	if (*error != EVAS_LOAD_ERROR_NONE) {
		MSG_SEC("Image loading failed. Error=%d File=%s",
		        *error, mdata->filepath);
		_ivug_ss_load_next_image((SlideShow *)sLy->pSlideshow);
		return;
	}
	edje_object_signal_emit(_EDJ(pSlideShow->sLayout[pSlideShow->sCurrent].layout), "elm,state,hide_thumbnail", "slideshow");

	MSG_SEC("Photocam Pre-loaded. File=%s",
	        ivug_get_filename(mdata->filepath));

	return;
}

static bool _ivug_ss_create_image_layout(Evas_Object *parent, Slide_Layout *sLayout)
{
	/* Create Layout for the current item */
	char *edj_path = IVUG_SS_LY_EDJ_PATH;
	sLayout->layout = _ivug_ss_create_layout(parent, edj_path, "slayout");
	free(edj_path);

	if (sLayout->layout == NULL) {
		MSG_ERROR("Cannot create current layout");
		return false;
	}

	sLayout->photocam = elm_photocam_add(sLayout->layout);

	evas_object_size_hint_expand_set(sLayout->photocam, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(sLayout->photocam, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_part_content_set(sLayout->layout, "content", sLayout->photocam);

	elm_photocam_zoom_mode_set(sLayout->photocam, ELM_PHOTOCAM_ZOOM_MODE_AUTO_FIT);
	elm_photocam_paused_set(sLayout->photocam, EINA_TRUE);

	evas_object_smart_callback_add(sLayout->photocam,
	                               "loaded",
	                               _ivug_ss_photocam_loaded_cb,
	                               sLayout);

	evas_object_show(sLayout->photocam);
	evas_object_show(sLayout->layout);

	return true;
}

SlideShow *ivug_ss_create(Evas_Object *parent)
{
	SlideShow *pSlideShow = NULL;
	MSG_HIGH("ivug_ss_create");

	pSlideShow = (SlideShow*)calloc(1, sizeof(SlideShow));
	IV_ASSERT(pSlideShow != NULL);

	pSlideShow->state = SLIDE_SHOW_STOPPED;
	pSlideShow->bMouse_event = false;
	ivug_effect_type ivug_effect = IVUG_EFFECT_TYPE_SLIDE;

	ivug_config_get_slideshow_setting(&(pSlideShow->ss_mode),
	                                  &(pSlideShow->ss_interval_time), &ivug_effect);

	if (ivug_effect == IVUG_EFFECT_TYPE_UNKNOWN) {
		free(pSlideShow);
		pSlideShow = NULL;
		return NULL;
	}

	pSlideShow->effect_type = _ivug_ss_get_trans_effect(ivug_effect);

// If Non-DALI slideshow.
	char *edj_path = IVUG_SS_LY_EDJ_PATH;
	pSlideShow->obj = _ivug_ss_create_layout(parent, edj_path, "view.slideshow");
	free(edj_path);
	MSG_ASSERT(pSlideShow->obj != NULL);
	evas_object_name_set(pSlideShow->obj, "slideshow");

	/* Create Layout for the current item */
	if (_ivug_ss_create_image_layout(pSlideShow->obj, &pSlideShow->sLayout[0]) == false) {
		MSG_ERROR("Cannot create current layout");
		free(pSlideShow);
		return NULL;
	}
	pSlideShow->sLayout[0].pSlideshow = pSlideShow;
	evas_object_name_set(pSlideShow->sLayout[0].layout, "Layout 0");

	/* Create Layout for the next item */
	if (_ivug_ss_create_image_layout(pSlideShow->obj, &pSlideShow->sLayout[1]) == false) {
		MSG_ERROR("Cannot create current layout");
		elm_object_part_content_unset(pSlideShow->sLayout[0].layout, "content");
		evas_object_del(pSlideShow->sLayout[0].photocam);
		evas_object_del(pSlideShow->sLayout[0].layout);
		free(pSlideShow);
		return NULL;
	}

	pSlideShow->sLayout[1].pSlideshow = pSlideShow;
	evas_object_name_set(pSlideShow->sLayout[1].layout, "Layout 1");

	/* Event rect */
	pSlideShow->event = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_name_set(pSlideShow->event, "ss_event");
	evas_object_color_set(pSlideShow->event, 0, 0, 0, 0);

	evas_object_show(pSlideShow->event);
	evas_object_repeat_events_set(pSlideShow->event, EINA_TRUE);

	//EFL::dump_obj(pSlideShow->obj, 0);

	evas_object_event_callback_add(pSlideShow->obj, EVAS_CALLBACK_MOVE, _moved, pSlideShow);
	evas_object_event_callback_add(pSlideShow->obj, EVAS_CALLBACK_RESIZE, _resized, pSlideShow);
	evas_object_event_callback_add(pSlideShow->obj, EVAS_CALLBACK_SHOW, _shown, pSlideShow);
	evas_object_event_callback_add(pSlideShow->obj, EVAS_CALLBACK_HIDE, _hidden, pSlideShow);

// Event
	evas_object_event_callback_add(pSlideShow->event, EVAS_CALLBACK_MOUSE_DOWN, _ivug_ss_on_mouse_down, pSlideShow);
	evas_object_event_callback_add(pSlideShow->event, EVAS_CALLBACK_MOUSE_MOVE, _ivug_ss_on_mouse_move, pSlideShow);
	evas_object_event_callback_add(pSlideShow->event, EVAS_CALLBACK_MOUSE_UP, _ivug_ss_on_mouse_up, pSlideShow);

	return pSlideShow;
}

bool ivug_ss_start(SlideShow *pSlideShow , Media_Item *current, Media_List *list, Eina_Bool bSlideFirst)
{
	MSG_ASSERT(pSlideShow != NULL);

	ivug_ss_get_screen_size(gGetCurrentWindow(), &pSlideShow->screen_w, &pSlideShow->screen_h);

	if (pSlideShow->ss_interval_time < 0) {
		MSG_ERROR("slide show interval time is invalid !!!");
		return false;
	}

	if (pSlideShow->state == SLIDE_SHOW_RUNNING) {
		MSG_ERROR("Debug me!!! Slide show is running. remove previous one.");
		ivug_ss_stop(pSlideShow);
	}

	if (pSlideShow->effect_engine != NULL) {
		MSG_FATAL("Debug Me!!!");
		return false;
	}

	int ret = DEVICE_ERROR_NONE;
	ret = device_power_request_lock(POWER_LOCK_DISPLAY, 0);

	if (ret != DEVICE_ERROR_NONE) {
		MSG_ERROR("Display Request could not be processed.");
	}

	pSlideShow->sCurrent = 0;
	pSlideShow->bSS_StopFlag = EINA_FALSE;


	pSlideShow->media_list = list;
	pSlideShow->ss_Header = current;
	pSlideShow->cur_item = current;

	MSG_ASSERT(pSlideShow->obj != NULL);
	pSlideShow->bMouse_event = true;
	evas_object_move(pSlideShow->obj, 0, 0);
	evas_object_resize(pSlideShow->obj, pSlideShow->screen_w, pSlideShow->screen_h);

	_ivug_ss_update_pos(pSlideShow, 0, 0);

	Slide_Layout *sLyCurrent = &pSlideShow->sLayout[pSlideShow->sCurrent];
	Slide_Layout *sLyNext = &pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2];

	_ivug_ss_set_content(sLyCurrent, current);

	Media_Item *mitem = sLyCurrent->mitem;
	_ivug_ss_video_icon(pSlideShow->sLayout[pSlideShow->sCurrent].layout, mitem);

	Media_Item *next = NULL;

	next = ivug_ss_get_next_item(pSlideShow->media_list,
	                             pSlideShow->ss_Header,
	                             sLyCurrent->mitem,
	                             pSlideShow->ss_mode);

	if (next == NULL) {
		/* if last image is tapped, then after some time, back to main view directly */
		pSlideShow->ss_timer = NULL;
		pSlideShow->ss_Header = NULL;
		pSlideShow->state = SLIDE_SHOW_RUNNING;

		pSlideShow->ss_timer = ecore_timer_add(pSlideShow->ss_interval_time, _ivug_ss_auto_finish_timer_cb, pSlideShow);
		return false;
	}

	_ivug_ss_set_content(sLyNext, next);

	mitem = sLyNext->mitem;
	_ivug_ss_video_icon(pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout, mitem);

	pSlideShow->state = SLIDE_SHOW_RUNNING;

//	EFL::dump_obj(pSlideShow->obj, 0);

	if (pSlideShow->ss_timer) {
		ecore_timer_del(pSlideShow->ss_timer);
	}
	pSlideShow->ss_timer = ecore_timer_add(pSlideShow->ss_interval_time, _ivug_ss_on_slide_interval, pSlideShow);

	MSG_HIGH("Slide show started!");

	return true;
}


bool ivug_ss_stop(SlideShow *pSlideShow)
{
	MSG_ASSERT(pSlideShow != NULL);

	MSG_HIGH("Slide show stopping");

	pSlideShow->bSS_StopFlag = EINA_TRUE;

	int ret = DEVICE_ERROR_NONE;
	ret = device_power_release_lock(POWER_LOCK_DISPLAY);

	if (ret != DEVICE_ERROR_NONE) {
		MSG_ERROR("Display Release could not be processed.");
	}

	//delete timmer
	if (pSlideShow->ss_timer) {
		ecore_timer_del(pSlideShow->ss_timer);
		pSlideShow->ss_timer = NULL;
	}

	if (pSlideShow->click_timer) {
		ecore_timer_del(pSlideShow->click_timer);
		pSlideShow->click_timer = NULL;
	}
	if (pSlideShow->event) {
		evas_object_del(pSlideShow->event);
		pSlideShow->event = NULL;
	}
	if (pSlideShow->pauseLayout) {
		evas_object_del(pSlideShow->pauseLayout);
		pSlideShow->pauseLayout = NULL;
	}
	if (pSlideShow->pauseLayout2) {
		evas_object_del(pSlideShow->pauseLayout2);
		pSlideShow->pauseLayout2 = NULL;
	}

	/*when it is 3D slideshow.it should call finish operation to end slideshow*/
	if (pSlideShow->effect_engine == NULL) {
		MSG_HIGH("Slide effect_engine = NULL");
		pSlideShow->state = SLIDE_SHOW_STOPPED;
		pSlideShow->ss_Header = NULL;
		// TODO : Mis-implementation
		evas_object_smart_callback_call(ivug_ss_object_get(pSlideShow), "slideshow,finished", (void *)SLIDE_SHOW_STOPPED);
		pSlideShow->bSS_StopFlag = EINA_FALSE;
	}
	//MSG_HIGH("Stop slide show. but not run state");

	return true;
}

bool ivug_ss_resume(SlideShow *pSlideShow)
{
	MSG_HIGH("Slide show Resume. pSlideShow=0x%08x", pSlideShow);
	MSG_ASSERT(pSlideShow != NULL);

	if (pSlideShow->pauseLayout != NULL) {
		evas_object_del(pSlideShow->pauseLayout);
		pSlideShow->pauseLayout = NULL;
	}

	if (pSlideShow->pauseLayout2 != NULL) {
		evas_object_del(pSlideShow->pauseLayout2);
		pSlideShow->pauseLayout2 = NULL;
	}

	if (pSlideShow->ss_timer) {
		ecore_timer_thaw(pSlideShow->ss_timer);
	}

	pSlideShow->state = SLIDE_SHOW_RUNNING;

	return true;
}

static void _play_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	IV_ASSERT(data != NULL);

	SlideShow *pSlideShow = (SlideShow *)data;
	pSlideShow->bPlayButton = true;

	if (pSlideShow->state == SLIDE_SHOW_PAUSE) {
		elm_object_signal_emit(pSlideShow->pauseLayout, "elm,state,hide", "");
		ivug_ss_resume(pSlideShow);

		elm_object_signal_emit(pSlideShow->pauseLayout2, "elm,state,hide", "");
		ivug_ss_resume(pSlideShow);
	}
}

static void _stop_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	IV_ASSERT(data != NULL);

	SlideShow *pSlideShow = (SlideShow *)data;

	ivug_ss_stop(pSlideShow);
}

static char *
_gl_text_get_effect_type(void *data, Evas_Object *obj, const char *part)
{
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	char buf[255] = {0, };

	switch (index) {
	case 0:
		snprintf(buf, sizeof(buf), "%s", GET_STR(IDS_SLIDESHOW_FLOW_EFFECT));
		break;
	case 1:
		snprintf(buf, sizeof(buf), "%s", GET_STR(IDS_SLIDESHOW_FADE_EFFECT));
		break;
	default:
		snprintf(buf, sizeof(buf), "%s", GET_STR(IDS_SLIDESHOW_FADE_EFFECT));
	}

	return strdup(buf);
}

static char *
_gl_text_get_slide_interval(void *data, Evas_Object *obj, const char *part)
{
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	char buf[50] = {0, };
	char *str = NULL;

	switch (index) {
	case 0:
		str = GET_STR(IDS_SLIDE_SECOND);
		snprintf(buf, sizeof(buf), "%s", str);
		break;
	case 1:
		str = GET_STR(IDS_SLIDE_SECONDS);
		snprintf(buf, sizeof(buf), str, 3);
		break;
	case 2:
		str = GET_STR(IDS_SLIDE_SECONDS);
		snprintf(buf, sizeof(buf), str, 5);
		break;
	default:
		str = GET_STR(IDS_SLIDE_SECOND);
		snprintf(buf, sizeof(buf), "%s", str);
	}

	MSG_ERROR("buffer %s", buf);
	return strdup(buf);
}

static void
gl_slide_interval_radio_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*) event_info;
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	Evas_Object *radio;
	elm_genlist_item_selected_set(it, EINA_FALSE);
	radio = elm_object_item_part_content_get(it, "elm.swallow.end");
	elm_radio_value_set(radio, index + 1);
	MSG_ERROR("Index is %d", index + 1);

	ivug_config_set_interval_time(index + 1);
	ob->pSlideshow->ss_interval_time = (double) ivug_config_get_slideshow_interval_time();

	ecore_timer_del(ob->pSlideshow->ss_timer);
	ob->pSlideshow->ss_timer = ecore_timer_add(ob->pSlideshow->ss_interval_time, _ivug_ss_on_slide_interval, ob->pSlideshow);

	if (ob->pSlideshow->popup) {
		evas_object_del(ob->pSlideshow->popup);
		ob->pSlideshow->popup = NULL;
	}
}

static void
gl_effect_radio_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item*) event_info;
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	Evas_Object *radio;
	elm_genlist_item_selected_set(it, EINA_FALSE);
	radio = elm_object_item_part_content_get(it, "elm.swallow.end");
	elm_radio_value_set(radio, index + 1);
	MSG_ERROR("Index is %d", index + 1);

	ivug_config_set_transition_effect(index + 1);

	char *effect = ivug_config_get_slideshow_effect_type();
	ivug_effect_type effect_type = ivug_config_get_effect_type_by_string(effect);
	ob->pSlideshow->effect_type = _ivug_ss_get_trans_effect(effect_type);


	if (ob->pSlideshow->popup) {
		evas_object_del(ob->pSlideshow->popup);
		ob->pSlideshow->popup = NULL;
	}
}

static Evas_Object*
gl_radio_slide_interval_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *radio;
		Evas_Object *radio_main = (Evas_Object*)evas_object_data_get(obj, "radio");
		radio = elm_radio_add(obj);
		elm_radio_group_add(radio, radio_main);
		elm_radio_state_value_set(radio, index);

		int radio_index = 0;
		int interval_time = ivug_config_get_slideshow_interval_time();
		if (interval_time == 3) {
			radio_index = 1;
		} else if (interval_time == 5) {
			radio_index = 2;
		}

		elm_radio_value_set(radio, radio_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_TRUE);
		return radio;
	}
	return NULL;
}

static Evas_Object*
gl_radio_effects_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
	st_temp *ob = (st_temp *)data;
	int index = ob->index;
	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *radio;
		Evas_Object *radio_main = (Evas_Object*)evas_object_data_get(obj, "radio");
		radio = elm_radio_add(obj);
		elm_radio_group_add(radio, radio_main);
		elm_radio_state_value_set(radio, index);

		int radio_index = 0;
		char *effect = ivug_config_get_slideshow_effect_type();
		if (!strcmp(effect, "DissolveFade")) {
			radio_index = 1;
		}

		elm_radio_value_set(radio, radio_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_TRUE);
		return radio;
	}
	return NULL;
}

static void
popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

Evas_Object *ivug_list_popoup_show(const char *title_id, void *data)
{
	SlideShow *pSlideshow = (SlideShow *)data;

	static Elm_Genlist_Item_Class itc;
	Evas_Object *popup;
	Evas_Object *genlist;
	Evas_Object *radio;
	int i;

	popup = elm_popup_add(gGetCurrentWindow());
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, pSlideshow);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(popup, "block,clicked", popup_block_clicked_cb, gGetCurrentWindow());

	/* genlist */
	genlist = elm_genlist_add(popup);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(genlist, "popup");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);

	/* radio */
	radio = elm_radio_add(genlist);
	elm_radio_state_value_set(radio, 0);
	elm_radio_value_set(radio, 0);
	evas_object_data_set(genlist, "radio", radio);

	if (strcmp(title_id, GET_STR(IDS_SLIDESHOW_EFFECT))) {
		itc.item_style = "default_style";
		itc.func.text_get = _gl_text_get_slide_interval;
		itc.func.content_get = gl_radio_slide_interval_content_get_cb;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		for (i = 0; i < 3; i++) {
			st_temp *ob = (st_temp *) calloc(1, sizeof(st_temp));
			if (ob != NULL) {
				ob->index = i;
				ob->pSlideshow = pSlideshow;

				elm_genlist_item_append(genlist, &itc, (void *) ob, NULL, ELM_GENLIST_ITEM_NONE, gl_slide_interval_radio_sel_cb, (void *) ob);
			}
		}
	} else {
		itc.item_style = "default_style";
		itc.func.text_get = _gl_text_get_effect_type;
		itc.func.content_get = gl_radio_effects_content_get_cb;
		itc.func.state_get = NULL;
		itc.func.del = NULL;

		for (i = 0; i < 2; i++) {
			st_temp *ob = (st_temp *) calloc(1, sizeof(st_temp));
			if (ob) {
				ob->index = i;
				ob->pSlideshow = pSlideshow;

				elm_genlist_item_append(genlist, &itc, (void *) ob, NULL, ELM_GENLIST_ITEM_NONE, gl_effect_radio_sel_cb, (void *) ob);
			}
		}
	}

	evas_object_show(genlist);
	elm_object_content_set(popup, genlist);
	evas_object_show(popup);

	return popup;
}

static void _effect_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	IV_ASSERT(data != NULL);

	SlideShow *pSlideShow = (SlideShow *)data;
	pSlideShow->popup = ivug_list_popoup_show(GET_STR(IDS_SLIDESHOW_EFFECT), pSlideShow);

	return;
}

static void _setting_icon_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	IV_ASSERT(data != NULL);

	SlideShow *pSlideShow = (SlideShow *)data;
	pSlideShow->popup = ivug_list_popoup_show(GET_STR(IDS_SLIDESHOW_SLIDE_INTERVAL), pSlideShow);

	return;
}

static void ivug_show_pause_state_layout(SlideShow *pSlideShow)
{
	IV_ASSERT(pSlideShow != NULL);

//Current Layout
	char *edj_path = IVUG_SS_LY_EDJ_PATH;
	pSlideShow->pauseLayout = elm_layout_add(pSlideShow->sLayout[0].layout);
	evas_object_size_hint_expand_set(pSlideShow->pauseLayout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	Eina_Bool ret = elm_layout_file_set(pSlideShow->pauseLayout, edj_path, "slideshow_overlay");

	if (ret == EINA_FALSE) {
		MSG_ERROR("Layout file set failed! slideshow_overlay in %s", edj_path);
	}

	/*text*/
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout, "slide_interval_setting.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_SLIDE_INTERVAL));
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout, "effect.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_EFFECT));
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout, "stop.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_STOP));

	/*register callback*/
	elm_object_signal_callback_add(pSlideShow->pauseLayout, "mouse,clicked,1", "play.icon", _play_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout, "mouse,clicked,1", "stop.icon.click", _stop_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout, "mouse,clicked,1", "slide_interval_setting.icon.click", _setting_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout, "mouse,clicked,1", "effect.icon.click", _effect_icon_cb, (void *)pSlideShow);

	elm_object_part_content_set(pSlideShow->sLayout[0].layout, "elm.swallow.overlay", pSlideShow->pauseLayout);

	evas_object_show(pSlideShow->pauseLayout);

//Next Layout
	pSlideShow->pauseLayout2 = elm_layout_add(pSlideShow->sLayout[1].layout);
	evas_object_size_hint_expand_set(pSlideShow->pauseLayout2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ret = elm_layout_file_set(pSlideShow->pauseLayout2, edj_path, "slideshow_overlay");

	if (ret == EINA_FALSE) {
		MSG_ERROR("Layout file set failed! slideshow_overlay in %s", edj_path);
	}
	free(edj_path);

	/*text*/
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout2, "slide_interval_setting.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_SLIDE_INTERVAL));
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout2, "effect.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_EFFECT));
	elm_object_domain_translatable_part_text_set(pSlideShow->pauseLayout2, "stop.icon.text", textdomain(NULL), GET_STR(IDS_SLIDESHOW_STOP));

	/*register callback*/
	elm_object_signal_callback_add(pSlideShow->pauseLayout2, "mouse,clicked,1", "play.icon", _play_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout2, "mouse,clicked,1", "stop.icon.click", _stop_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout2, "mouse,clicked,1", "slide_interval_setting.icon.click", _setting_icon_cb, (void *)pSlideShow);
	elm_object_signal_callback_add(pSlideShow->pauseLayout2, "mouse,clicked,1", "effect.icon.click", _effect_icon_cb, (void *)pSlideShow);

	elm_object_part_content_set(pSlideShow->sLayout[1].layout, "elm.swallow.overlay", pSlideShow->pauseLayout2);

	evas_object_show(pSlideShow->pauseLayout2);


}

bool ivug_ss_pause(SlideShow *pSlideShow)
{
	MSG_HIGH("Slide show Pause. pSlideShow=0x%08x", pSlideShow);

	MSG_ASSERT(pSlideShow != NULL);

	if (pSlideShow->ss_timer) {
		ecore_timer_freeze(pSlideShow->ss_timer);
	}

	pSlideShow->state = SLIDE_SHOW_PAUSE;

	ivug_show_pause_state_layout(pSlideShow);

	return true;
}

Media_Item * ivug_ss_item_get(SlideShow *pSlideShow)
{
	MSG_ASSERT(pSlideShow != NULL);

	return pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].mitem;
}

void ivug_ss_delete(SlideShow *pSlideShow)
{
	MSG_HIGH("Slide show Delete");

	if (!pSlideShow) {
		MSG_ERROR("Already deleted!!");
		return;
	}

	int ret = DEVICE_ERROR_NONE;
	ret = device_power_release_lock(POWER_LOCK_DISPLAY);

	if (ret != DEVICE_ERROR_NONE) {
		MSG_ERROR("Display Release could not be processed.");
	}

	if (pSlideShow->focus_in_handler) {
		ecore_event_handler_del(pSlideShow->focus_in_handler);
		pSlideShow->focus_in_handler = NULL;
	}

	if (pSlideShow->focus_out_handler) {
		ecore_event_handler_del(pSlideShow->focus_out_handler);
		pSlideShow->focus_out_handler = NULL;
	}

	if (pSlideShow->visibility_handler) {
		ecore_event_handler_del(pSlideShow->visibility_handler);
		pSlideShow->visibility_handler = NULL;
	}

	evas_object_event_callback_del(pSlideShow->event, EVAS_CALLBACK_MOUSE_DOWN, _ivug_ss_on_mouse_down);
	evas_object_event_callback_del(pSlideShow->event, EVAS_CALLBACK_MOUSE_MOVE, _ivug_ss_on_mouse_move);
	evas_object_event_callback_del(pSlideShow->event, EVAS_CALLBACK_MOUSE_UP, _ivug_ss_on_mouse_up);

	if (pSlideShow->effect_engine) {
		ivug_effect_finalize(pSlideShow->effect_engine);
		pSlideShow->effect_engine = NULL;
	}

	if (pSlideShow->ss_timer) {
		ecore_timer_del(pSlideShow->ss_timer);
		pSlideShow->ss_timer = NULL;
	}

	if (pSlideShow->click_timer) {
		ecore_timer_del(pSlideShow->click_timer);
		pSlideShow->click_timer = NULL;
	}

	for (int i = 0 ; i < 2; i++) {
		if (pSlideShow->sLayout[i].photocam) {
			evas_object_del(pSlideShow->sLayout[i].photocam);
			pSlideShow->sLayout[i].photocam = NULL;
		}

		if (pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].layout) {
			pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].photocam =
			    elm_object_part_content_unset(pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].layout,
			                                  "content");
		}

		if (pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].photocam) {
			evas_object_del(pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].photocam);
			pSlideShow->sLayout[(pSlideShow->sCurrent) % 2].photocam = NULL;
		}

		if (pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout) {
			pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].photocam =
			    elm_object_part_content_unset(pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout,
			                                  "content");
		}

		if (pSlideShow->sLayout[i].layout) {
			evas_object_del(pSlideShow->sLayout[i].layout);
			pSlideShow->sLayout[i].layout = NULL;
		}
	}

	if (pSlideShow->event) {
		evas_object_del(pSlideShow->event);
		pSlideShow->event = NULL;
	}

	if (pSlideShow->obj) {
		evas_object_del(pSlideShow->obj);
		pSlideShow->obj = NULL;
	}

	if (pSlideShow) {
		free(pSlideShow);
		pSlideShow = NULL;
	}
	return;
}

void ivug_ss_resize(SlideShow *pSlideShow)
{
	IVUG_FUNC_ENTER();
	/*This interface only used for 2d slideshow,
	  *So disable it when current is dali-slideshow.
	  *resize slide show layout and move them to right pos after rotation
	  */
	if (!pSlideShow) {
		return;
	}

	if ((pSlideShow->obj) && (pSlideShow->event)) {
		ivug_ss_get_screen_size(gGetCurrentWindow(), &pSlideShow->screen_w, &pSlideShow->screen_h);
		evas_object_resize(pSlideShow->obj, pSlideShow->screen_w, pSlideShow->screen_h);
		Evas_Coord ox, oy, ow, oh;
		evas_object_geometry_get(pSlideShow->obj, &ox, &oy, &ow, &oh);

		//MSG_HIGH("Moved (%d,%d,%d,%d)", ox, oy, ow, oh);
		if (pSlideShow->event) {
			evas_object_move(pSlideShow->event, ox, oy);
		}

		if (pSlideShow->sLayout[pSlideShow->sCurrent].layout) {
			evas_object_move(pSlideShow->sLayout[pSlideShow->sCurrent].layout, ox, oy);
		}
		if (pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout) {
			evas_object_move(pSlideShow->sLayout[(pSlideShow->sCurrent + 1) % 2].layout, ox + pSlideShow->screen_w + IVUG_IMAGE_BETWEEN_MARGIN, oy);
		}
		//MSG_HIGH("_ivug_ss_resize_obj, Moved (%d,%d,%d,%d)", ox, oy, ow, oh);
	}
	IVUG_FUNC_LEAVE();
}



Evas_Object *ivug_ss_object_get(SlideShow *pSlideShow)
{
	MSG_ASSERT(pSlideShow != NULL);

	return pSlideShow->obj;

}

void
ivug_ss_set_stop(SlideShow *pSlideShow)
{
	MSG_ASSERT(pSlideShow != NULL);

	pSlideShow->state = SLIDE_SHOW_STOPPED;
}



