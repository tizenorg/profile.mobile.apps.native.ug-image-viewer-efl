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
#include <ui-gadget.h>

typedef struct _ug_data ug_data;


#ifdef __cplusplus
extern "C" {
#endif

ug_data *AllocUGData();
void FreeUGData(ug_data *ug);


void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h service, void *priv);

void on_start(ui_gadget_h ug, app_control_h service, void *priv);

void on_pause(ui_gadget_h ug, app_control_h service, void *priv);

void on_resume(ui_gadget_h ug, app_control_h service, void *priv);


/*

*/
void on_message(ui_gadget_h ug, app_control_h msg, app_control_h service, void *priv);

/*

*/
void on_event(ui_gadget_h ug, enum ug_event event, app_control_h service, void *priv);

/*

*/
void on_destroying(ui_gadget_h ug, app_control_h service, void *priv);


/*

*/
void on_destroy(ui_gadget_h ug, app_control_h service, void *priv);


#ifdef __cplusplus
}
#endif

#endif //__IVUG_BASE_H__

