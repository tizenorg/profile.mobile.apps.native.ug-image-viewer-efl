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

#ifndef __IVUG_SLIDER_NEW_H__
#define __IVUG_SLIDER_NEW_H__

#include "ivug-define.h"

#include <Elementary.h>
//#include "ivug-medialist.h"
#include "ivug-media.h"
#include "ivug-motion.h"

typedef void (*callback_t)(void *handle, Media_Item *mItem, void *data);
typedef void (*location_callback_t )(void *handle, int x, int y, int w, int h, void *data);
typedef void (*status_callback_t)(void *handle, Media_Item *mItem, void *data);

typedef enum {
	LONGTAP_ENDED___,
	LONGTAP_CANCELED___,
}longtap_state_new;

typedef enum {
	SLIDER_MODE_MULTIPLE,
	SLIDER_MODE_SINGLE,
	SLIDER_MODE_MAX
}slider_mode_e;

typedef struct _Ivug_SliderNew Ivug_SliderNew;

#ifdef __cplusplus
extern "C" {
#endif

Ivug_SliderNew * ivug_slider_new_init(Evas_Object *parent, void *data);

void ivug_slider_new_destroy(Ivug_SliderNew * slider_new);

//do this befor ivug_slider_new_set_list
void ivug_slider_new_set_mode(Ivug_SliderNew * slider_new, slider_mode_e mode);

slider_mode_e ivug_slider_new_get_mode(Ivug_SliderNew * slider_new);

void ivug_slider_new_set_list(Ivug_SliderNew * slider_new, Media_List *mList,Media_Item *current);

void ivug_slider_new_set_photocam(Ivug_SliderNew *slider_new,Evas_Object * photocam);
Evas_Object * ivug_slider_new_get_photocam(Ivug_SliderNew *slider_new);
void ivug_slider_set_current_Photocam(Ivug_SliderNew *slider_new,int pc);
void ivug_slider_set_Photocam_moved(Ivug_SliderNew *slider_new,bool pcm);
void  ivug_set_photocam_reset(Ivug_SliderNew *slider_new);
bool  ivug_isphotocam_reset(Ivug_SliderNew *slider_new);
Evas_Event_Flags _zoom_start(void *data, void *event_info);
Evas_Event_Flags _zoom_move(void *data, void *event_info);
Evas_Event_Flags _zoom_end(void *data, void *event_info);
void ivug_disable_gesture(Ivug_SliderNew *slider_new);
void ivug_enable_gesture(Ivug_SliderNew *slider_new);
Evas_Event_Flags _zoom_abort(void *data, void *event_info);

Evas_Object * ivug_slider_new_get_layout(Ivug_SliderNew *slider_new);

Evas_Object * ivug_slider_new_get_gesture(Ivug_SliderNew *slider_new);

bool  ivug_iszoom_enabled(Ivug_SliderNew *slider_new);
bool  ivug_isslide_enabled(Ivug_SliderNew *slider_new);

void ivug_slider_new_move_item(Ivug_SliderNew *slider_new, Media_Item *item);


void ivug_slider_new_region_get(Ivug_SliderNew *slider_new, int *x, int *y, int *w, int *h);

int ivug_slider_new_get_cur_index(Ivug_SliderNew *slider_new);

void ivug_slider_new_change_view_size(Ivug_SliderNew *slider_new, int w, int h);

void ivug_slider_new_delete_cur_image(Ivug_SliderNew *slider_new);

void ivug_slider_call_changed_callback(Ivug_SliderNew *slider_new, Media_Item *cur_mItem);

void ivug_slider_set_changed_callback(Ivug_SliderNew *slider_new, callback_t callback, void *data);
void ivug_slider_set_loaded_callback(Ivug_SliderNew *slider_new, callback_t callback, void *data);
void ivug_slider_set_location_callback(Ivug_SliderNew *slider_new, location_callback_t callback, void *data);

void ivug_slider_new_image_size_get(Ivug_SliderNew *slider_new, int *w, int *h);

//update current window
void ivug_slider_new_reload(Ivug_SliderNew *slider_new);

//update whole list
void ivug_slider_new_update_list(Ivug_SliderNew *slider_new, Media_List *mList);


void ivug_slider_new_set_orientation(Ivug_SliderNew *slider_new, int orientation);

int ivug_slider_new_get_orientation(Ivug_SliderNew *slider_new);



void ivug_slider_new_agif_enable(Ivug_SliderNew *slider_new, bool enable);

/*
	double tab & zoom is disabled/enabled
*/
void ivug_slider_new_mouse_enable(Ivug_SliderNew *slider_new, bool enable);

void ivug_slider_new_rotate(Ivug_SliderNew *slider_new, int degree);

float ivug_slider_new_get_zoom(Ivug_SliderNew *slider_new);

bool ivug_slider_new_is_zoomed(Ivug_SliderNew *slider_new);

void ivug_slider_new_enable_motion_pan(Ivug_SliderNew *slider_new, bool pan_state);

void ivug_slider_update_icon_layer(Ivug_SliderNew *slider_new);

void  ivug_slider_new_hide_play_icon(Ivug_SliderNew *slider_new);

void ivug_reset_zoom(Ivug_SliderNew *slider_new);


/*
	Below API will be deprecated!
*/



#ifdef __cplusplus
}
#endif


#endif	// __IVUG_SLIDER_NEW_H__

