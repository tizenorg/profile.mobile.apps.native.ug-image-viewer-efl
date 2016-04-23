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

#ifndef __IVUG_BASE_H__
#define __IVUG_BASE_H__

#include <Elementary.h>
#include "ivug-parameter.h"
#include "ivug-slideshow-view.h"
#include "ivug-crop-ug.h"
#include "ivug-main-view.h"

typedef struct _ug_data ug_data;

struct _ug_data {
	Evas_Object *base;			// UG layout

// View Data;
	struct _Ivug_MainView *main_view;
	IvugCropUG *crop_ug;

	Ivug_SlideShowView *ss_view;

	ivug_parameter* ivug_param;

	bool bError;
	char *bErrMsg;

	Evas_Object *icon;
	Ecore_Timer *exit_timer;

	Evas_Object *navi_bar;
	Evas_Object *window;
	Elm_Object_Item *navi_it;
};


#ifdef __cplusplus
extern "C" {
#endif

void FreeUGData(ug_data *ug);


//void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h service, void *priv);
bool on_create(void *priv);
#if 0//Chandan
void on_start(ui_gadget_h ug, app_control_h service, void *priv);
#endif
void on_pause(void *priv);

void on_resume(void *priv);


/*

*/
#if 0//Chandan
void on_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv);
#endif

/*

*/
#if 0//Chandan
void on_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv);
#endif
/*

*/
void _language_changed_cb(void *user_data);

/*

*/
#if 0//Chandan
void on_destroying(app_control_h service, void *priv);
#endif

/*

*/
void on_destroy(void *priv);


#ifdef __cplusplus
}
#endif

#endif //__IVUG_BASE_H__

