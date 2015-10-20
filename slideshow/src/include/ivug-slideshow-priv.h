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

#pragma once

// Slide Show

/*
	Start slide show.

	if bSlideFirst == EINA_TRUE, slide and then wait interval time
	if bSlideFirst == EINA_FALSE, wait interval time and then slide
*/
#include <Elementary.h>
#include <player.h>
#include <glib.h>

#include "ivug-define.h"
#include "ivug-medialist.h"
#include "ivug-config.h"
#include "ivug-effect.h"
#include "ivug-player.h"

using namespace std;

#define DATA_PATH 						DATADIR"/"PACKAGE

typedef struct
{
	void *pSlideshow; //SlideShow *
	Evas_Object *layout;
	Evas_Object *photocam;
	Evas_Object *thumbnail;

	Media_Item *mitem;

	int x;	// Need??
	int y;
} Slide_Layout;

typedef enum{
	EVASPLUGIN_RESUMED = 0x00,
	EVASPLUGIN_PAUSED,
} EvasPluginState_t;


typedef struct _SlideShow
{
	Evas_Object *obj;		// Slide show view.
	Evas_Object *event;		// Mouse event

	/* slide show setting */
	double ss_interval_time;
	Effect_Engine* effect_engine;

	Ecore_Timer *ss_timer;	//slide show timer
	Ecore_Timer *click_timer;
	void* ss_user_data;

	Media_List *media_list;
	Media_Item *ss_Header;		// Start point

	int sCurrent;		// 0 or 1
	int screen_w;
	int screen_h;

	Ecore_Idler *load_idler;
	Ecore_X_Window xwin;
	Ecore_Event_Handler *focus_in_handler;
	Ecore_Event_Handler *focus_out_handler;
	Ecore_Event_Handler *visibility_handler;

	Media_Item *cur_item;
	GList *face_record_list;
	void *dali_viewer_handle;	/*3D slidehshow image view handle*/
	Media_Item *downloading_item;	/* only used for web image*/
	int xPixmapId;
	player_h vmpHandle;			/*Used for video and music*/

	Slide_Layout sLayout[2];
	Evas_Object *pauseLayout;
	Evas_Object *pauseLayout2;
	Evas_Object *popup;
	slide_show_mode ss_mode;
	/* slide show effect*/
	Effect_Type effect_type;
	EvasPluginState_t evas_plugin_state;
	slideshow_state_t state;
	Eina_Bool bSS_StopFlag; /* request slide show stop flag */
	bool bMouse_event;
	bool bPlayButton;
}SlideShow;

