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

#ifndef __IVUG_CONFIG_H__
#define __IVUG_CONFIG_H__

#include "ivug-define.h"

typedef enum {
	SLIDE_SHOW_MODE_NORMAL 			= 0x00,
	SLIDE_SHOW_MODE_REPEAT			= 0x01,
	SLIDE_SHOW_MODE_SHUFFLE			= 0x02,
	SLIDE_SHOW_MODE_SHUFFLE_REPEAT 	= 0x03,
} slide_show_mode;

typedef enum {
	IVUG_EFFECT_TYPE_UNKNOWN = -1,
	IVUG_EFFECT_TYPE_SLIDE=0x00,
	IVUG_EFFECT_TYPE_DISSOLVE_FADE
} ivug_effect_type;


typedef enum {
	PLAYSPEED_UNDEFINED,
	PLAYSPEED_1_5TH,
	PLAYSPEED_1_2TH,
	PLAYSPEED_1,
	PLAYSPEED_2,
} eConfigPlaySpeed;

#ifdef __cplusplus
extern "C" {
#endif

bool ivug_config_set_lockscreen_image(const char* filepath);

bool ivug_config_set_homescreen_image(const char* filepath);

void ivug_config_set_interval_time(int index);

ivug_effect_type
ivug_config_get_effect_type_by_string(char *effect_str);

char* ivug_config_get_slideshow_effect_type(void);

int ivug_config_get_slideshow_interval_time(void);

void ivug_config_set_transition_effect(int index);

void ivug_config_get_slideshow_setting(slide_show_mode * /* OUT */ mode,
							double * /* OUT */ interval_time,
							ivug_effect_type * /* OUT */ effect_type);

bool ivug_config_get_can_rotate(void);

bool ivug_config_set_playspeed(eConfigPlaySpeed speed);
//bool ivug_config_get_playspeed(eConfigPlaySpeed *speed);

#ifdef __cplusplus
}
#endif


#endif // __IVUG_CONFIG_H__
