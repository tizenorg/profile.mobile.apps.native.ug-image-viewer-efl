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

#include "ivug-common.h"
#include "ivug-parameter.h"

#ifndef	__IVUG_SMART_EVENT_BOX_H_
#define	__IVUG_SMART_EVENT_BOX_H_

typedef enum{
	IVUG_EVENT_CLICK,
	IVUG_EVENT_LEFT,
	IVUG_EVENT_RIGHT,
	IVUG_EVENT_MAX,
}IvugEventCallback_e;

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *ivug_smart_event_box_add(Evas_Object* parent);
void ivug_smart_event_box_callback_add(Evas_Object *event_box, IvugEventCallback_e event, void (*event_cb)(void *), void *user_data);
#ifdef __cplusplus
}
#endif

#endif //__IVUG_SMART_EVENT_BOX_H_
