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

#ifndef __IVUG_BUDDY_VIEW_H__
#define __IVUG_BUDDY_VIEW_H__

#include <Elementary.h>


/* SIGNAL
	"destroyed"
	"clicked"
*/

typedef enum {
	IVUG_BUDDY_NORMAL,	// launched from mainview
	IVUG_BUDDY_UG,		// ug mode
	IVUG_BUDDY_APPSVC	// appsvc mode
} ivug_buddy_mode;

typedef struct _Ivug_BuddyView{
	Evas_Object *parent;
	Evas_Object *layout;

	Elm_Object_Item *btn_send;

	Evas_Object *event_obj;
	Evas_Object *gesture;

	Evas_Object *photocam;

	Evas_Object *navi_bar;
	Elm_Object_Item *navi_it;

	char* filepath;
	char* mediaID;

	ivug_buddy_mode mode;

	bool bShowMenu;
}Ivug_BuddyView;

#ifdef __cplusplus
extern "C" {
#endif

Ivug_BuddyView * ivug_buddy_view_create(Evas_Object *parent);
bool ivug_buddy_view_destroy(Ivug_BuddyView *pBuddyView);
Evas_Object * ivug_buddy_view_get_layout(Ivug_BuddyView *pBuddyView);
void ivug_buddy_view_create_menu(Ivug_BuddyView *pBuddyView, Evas_Object *navi_bar);
bool ivug_buddy_view_load_file(Ivug_BuddyView *pBuddyView, const char *filepath, const char *mediaID);

#ifdef __cplusplus
}
#endif

#endif //__IVUG_BUDDY_VIEW_H__

