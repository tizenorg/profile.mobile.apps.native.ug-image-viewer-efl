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

#ifndef __DECOLAYER_H__
#define __DECOLAYER_H__

#include <Elementary.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
	IVUG_DECO_NONE 		=  0,
	IVUG_DECO_BESTPIC	= (1 << 0),
	IVUG_DECO_PANORAMA	= (1 << 1),
	IVUG_DECO_SOUNDPIC	= (1 << 2),
	IVUG_DECO_BURST		= (1 << 3),
	IVUG_DECO_VIDEO 	= (1 << 4),
} Ivug_Deco;

typedef enum {
	IVUG_DECO_ICON_NONE,
	IVUG_DECO_ICON_VIDEO,
	IVUG_DECO_ICON_SOUNDPIC,
	IVUG_DECO_ICON_BURST_PLAYSPEED,
	IVUG_DECO_ICON_BURST_PLAY,
} ivug_deco_icon_e;

/*
	{"start,blink", ""},
	{"stop,blink", ""},
	{"clicked,icon", ""},
*/
Evas_Object *ivug_decolayer_add(Evas_Object *parent);

void ivug_decolayer_set_type(Evas_Object *obj, Ivug_Deco deco);

void ivug_decolayer_start_blinking(Evas_Object *obj);

void ivug_decolayer_stop_blinking(Evas_Object *obj);

#ifdef DEPRECATED
bool ivug_decolayer_check_video_icon(Evas_Object *obj, int cx, int cy);

bool ivug_decolayer_check_sound_icon(Evas_Object *obj, int cx, int cy);
#endif 		// DEPRECATED

ivug_deco_icon_e ivug_decolayer_check_icon(Evas_Object *obj, int cx, int cy);


void ivug_decolayer_update_icon(Evas_Object *obj);

void ivug_decolayer_hide_play_icon(Evas_Object *obj);

#ifdef __cplusplus
}
#endif

#endif // __IVUG_BESTPIC_H__


