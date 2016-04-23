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

//#include "ivug-common.h"
#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-uuid.h"

#include <Elementary.h>
#include <assert.h>
#include <system_settings.h>
#include <app_preference.h>

#include "statistics.h"
#include "ivug-debug.h"
#include "ivug-config.h"

#define PREFERENCE_SLIDESHOW_INTERVAL_TIME "interval_time"
#define PREFERENCE_SLIDESHOW_SHUFFLE_STATE "shuffle_state"
#define PREFERENCE_SLIDESHOW_REPEAT_STATE "repeat_state"
#define PREFERENCE_SLIDESHOW_TRANSITION_EFFECT "effect"

enum { STATE_FALSE = 0, STATE_TRUE = 1, };

/*
	Set lock screen with given image.

	CAUTION : does not check filepath integrity
*/
bool ivug_config_set_lockscreen_image(const char* filepath)
{
	if (filepath == NULL)
	{
		MSG_IMAGEVIEW_ERROR("Lock screen path is NULL");
		return FALSE;
	}

	if (system_settings_set_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_LOCK_SCREEN, filepath) != SYSTEM_SETTINGS_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("Lockscreen set Error : %s", filepath);
		return FALSE;
	}

	MSG_IMAGEVIEW_HIGH("Set Lockscreen filepath = %s", filepath);

	return TRUE;
}

/*
	Set home screen with given image

	CAUTION : does not check filepath integrity
*/
bool ivug_config_set_homescreen_image(const char* filepath)
{
	if (filepath == NULL)
	{
		MSG_IMAGEVIEW_ERROR("Home screen path is NULL");
		return FALSE;
	}

	if (system_settings_set_value_string(SYSTEM_SETTINGS_KEY_WALLPAPER_HOME_SCREEN, filepath) != SYSTEM_SETTINGS_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("Homescreen set Error : %s", filepath);
		return FALSE;
	}

	TODO("Need to check file existence?????")

	MSG_IMAGEVIEW_HIGH("Set Homescreen filepath = %s", filepath);
	return TRUE;
}

static bool
_ivug_config_get_slideshow_repeat_state(void)
{
	bool repeat_state = true;
	bool existing = false;

	int ret = preference_is_existing(PREFERENCE_SLIDESHOW_REPEAT_STATE, &existing);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Preference Fails for Repeat State");
		repeat_state = true;
	}

	MSG_IMAGEVIEW_ERROR("Preference for Repeat State %s", (existing == false) ? "doesn't exist !!!!" : "exists");

	if (existing == false) {
		ret = preference_set_boolean(PREFERENCE_SLIDESHOW_REPEAT_STATE, true);
		if (ret != PREFERENCE_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("Unable to set preference for Repeat State ERROR(%d)", ret);
		}
	}

	ret = preference_get_boolean(PREFERENCE_SLIDESHOW_REPEAT_STATE, &repeat_state);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to get preference for Repeat State ERROR(%d)",ret);
	}

	MSG_IMAGEVIEW_HIGH("Repeat State is: %s", (repeat_state == true) ? "true" : "false");
	return repeat_state;
}

static bool
_ivug_config_get_slideshow_shuffle_state(void)
{
	bool shuffle_state = false;
	bool existing = false;

	int ret = preference_is_existing(PREFERENCE_SLIDESHOW_SHUFFLE_STATE, &existing);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Preference Fails for Shuffle State");
		shuffle_state = false;
	}

	MSG_IMAGEVIEW_ERROR("Preference for Shuffle State %s", (existing == false) ? "doesn't exist !!!!" : "exists");

	if (existing == false) {
		ret = preference_set_boolean(PREFERENCE_SLIDESHOW_SHUFFLE_STATE, false);
		if (ret != PREFERENCE_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("Unable to set preference for Shuffle State ERROR(%d)", ret);
		}
	}

	ret = preference_get_boolean(PREFERENCE_SLIDESHOW_SHUFFLE_STATE, &shuffle_state);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to get preference for Shuffle State ERROR(%d)",ret);
	}

	MSG_IMAGEVIEW_HIGH("Shuffle State is: %s", (shuffle_state == true) ? "true" : "false");
	return shuffle_state;
}

void
ivug_config_set_interval_time(int index)
{
	int interval = 0;

	switch (index) {
	case 1:
		interval = 1;
		break;
	case 2:
		interval = 3;
		break;
	case 3:
		interval = 5;
		break;
	}

	int ret = preference_set_int(PREFERENCE_SLIDESHOW_INTERVAL_TIME, interval);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to set preference for Interval Time ERROR(%d)", ret);
	}

	return;
}

int
ivug_config_get_slideshow_interval_time(void)
{
	int interval_time = 1;
	bool existing = false;

	int ret = preference_is_existing(PREFERENCE_SLIDESHOW_INTERVAL_TIME, &existing);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Preference Fails for Interval Time");
		interval_time = -1;
	}

	MSG_IMAGEVIEW_ERROR("Preference for Interval Time %s", (existing == false) ? "doesn't exist !!!!" : "exists");

	if (existing == false) {
		ret = preference_set_int(PREFERENCE_SLIDESHOW_INTERVAL_TIME, 1);
		if (ret != PREFERENCE_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("Unable to set preference for Interval Time ERROR(%d)", ret);
		}
	}

	ret = preference_get_int(PREFERENCE_SLIDESHOW_INTERVAL_TIME, &interval_time);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to get preference for Interval Time ERROR(%d)",ret);
	}

	MSG_IMAGEVIEW_HIGH("interval time is: %d", interval_time);
	return interval_time;
}

void
ivug_config_set_transition_effect(int index)
{
	char *effect = NULL;

	if (index == 1) {
		effect = "Slide";
	} else {
		effect = "DissolveFade";
	}

	int ret = preference_set_string(PREFERENCE_SLIDESHOW_TRANSITION_EFFECT, effect);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to set preference for Transition Effect ERROR(%d)", ret);
	}

	return;
}

char *
ivug_config_get_slideshow_effect_type(void)
{
	TODO("Free returned string??")

	char *effect_str = "DissolveFade";
	bool existing = false;

	int ret = preference_is_existing(PREFERENCE_SLIDESHOW_TRANSITION_EFFECT, &existing);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Preference Fails for Transition Effect");
		effect_str = "DissolveFade";
	}

	MSG_IMAGEVIEW_ERROR("Preference for Transition Effect %s", (existing == false) ? "doesn't exist !!!!" : "exists");

	if (existing == false) {
		ret = preference_set_string(PREFERENCE_SLIDESHOW_TRANSITION_EFFECT, "DissolveFade");
		if (ret != PREFERENCE_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("Unable to set preference for Transition Effect ERROR(%d)", ret);
		}
	}

	ret = preference_get_string(PREFERENCE_SLIDESHOW_TRANSITION_EFFECT, &effect_str);
	if (ret != PREFERENCE_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("Unable to get preference for Transition Effect ERROR(%d)",ret);
	}

	MSG_IMAGEVIEW_HIGH("Transition Effect is: %s", effect_str);
	return effect_str;
}

ivug_effect_type
ivug_config_get_effect_type_by_string(char *effect_str)
{
	IV_ASSERT(effect_str != NULL);

	ivug_effect_type type = IVUG_EFFECT_TYPE_SLIDE;

	if (!strncmp(effect_str, "Slide", strlen(effect_str))) {
		type = IVUG_EFFECT_TYPE_SLIDE;
	} else if (!strncmp(effect_str, "DissolveFade", strlen(effect_str))) {
		type = IVUG_EFFECT_TYPE_DISSOLVE_FADE;
	} else {										//Set all other cases as default NONE
		MSG_SLIDER_WARN("Invalid type : %s", effect_str);
		type = IVUG_EFFECT_TYPE_UNKNOWN;
	}

	MSG_IMAGEVIEW_HIGH("effect_str = %s, type = %d", effect_str, type);

	return type;
}

void
ivug_config_get_slideshow_setting(slide_show_mode *mode,
							double *interval_time,
							ivug_effect_type *effect_type)
{
	*mode = SLIDE_SHOW_MODE_NORMAL;

	bool state;

	state = _ivug_config_get_slideshow_repeat_state();
	if (state == true) {
		*mode |= SLIDE_SHOW_MODE_REPEAT;
	}

	state = _ivug_config_get_slideshow_shuffle_state();
	if (state == true) {
		*mode |= SLIDE_SHOW_MODE_SHUFFLE;
	}

	*interval_time = (double) ivug_config_get_slideshow_interval_time();

	/* EFFECT_NONE, EFFECT_SLIDE, EFFECT_DISSOLVE_FADE */
	char *effect = ivug_config_get_slideshow_effect_type();
	*effect_type = ivug_config_get_effect_type_by_string(effect);

	if (effect) {
		free(effect);
		effect = NULL;
	}
}

bool ivug_config_get_can_rotate(void)
{
	bool state = false;
	if (system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO, &state) != SYSTEM_SETTINGS_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("system_settings_get_value_bool, set as default: false");
		return false;
	}

	MSG_IMAGEVIEW_HIGH("rotate state is: %d", state);
	return (state == STATE_TRUE ? true : false);
}

