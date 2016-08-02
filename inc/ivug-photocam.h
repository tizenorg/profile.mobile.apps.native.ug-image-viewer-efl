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

#ifndef __IVUG_PHOTOCAM_H__
#define __IVUG_PHOTOCAM_H__

#include "ivug-common.h"
#include "ivug-parameter.h"
#include "ivug-medialist.h"

#include "ivug-thumblist.h"
#include "Evas.h"
enum
{
    PC_POSITION_LEFT = 0,
    PC_POSITION_CENTER ,
    PC_POSITION_RIGHT ,
};	
#ifdef __cplusplus
extern "C" {
#endif

/*
	Photocam images
*/
void _on_slider_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info);
 void _on_slider_mouse_down (void *data, Evas *e, Evas_Object *obj, void *event_info);
 void _on_slider_mouse_moved (void *data, Evas *e, Evas_Object *obj, void *event_info);
 Eina_Bool _ivug_left_move_interval(void *data);
 Eina_Bool _ivug_right_move_interval(void *data);
 void ivug_update_favourite_button(Ivug_MainView *pMainView);
 bool _main_view_object_move_ (Ivug_MainView *pMainView, Evas_Object *obj, int photocampos);
 void _ivug_main_view_left_transit_by_item_complete_cb(void *data, Evas_Object * obj, const char *emission, const char *source);
void _ivug_main_view_right_transit_by_item_complete_cb(void *data, Evas_Object * obj, const char *emission, const char *source);
void ivug_create_new_photocam_image(void *data, Evas_Object **cur_pc, const char *cur_iva);
void update_check(Ivug_MainView *pMainView);
void ivug_set_prev_next_photocam_images(void *data, Evas_Object **prev_pc,Evas_Object **next_pc, const char *prev_iva,const char *next_iva);

#ifdef __cplusplus
}
#endif

#endif //__IVUG_PHOTOCAM_H__
