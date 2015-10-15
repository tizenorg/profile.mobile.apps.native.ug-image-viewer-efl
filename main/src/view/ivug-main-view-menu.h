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

#ifndef __IVUG_MAINVIEW_MENU_H__
#define __IVUG_MAINVIEW_MENU_H__

#include "ivug-define.h"
#include "ivug-thumblist.h"
#include "ivug-main-view.h"

#ifdef __cplusplus
extern "C" {
#endif

void ivug_notification_create(const char* text);

/*
	Button handlers for main view
*/

void on_btn_slideshow_clicked(void *data, Evas_Object *obj, void *event_info);

void on_btn_more_clicked(void *data, Evas_Object *obj, void *event_info);

void _on_remove_main_view_ui(Ivug_MainView *pMainView);


/*
	Save current contents to disk
*/
void _on_mainview_save(Ivug_MainView *pMainView);
void _on_mainview_share(Ivug_MainView *pMainView);
void _on_mainview_delete(Ivug_MainView *pMainView);


/*
	Edit Image or Video
*/
void _on_mainview_edit(Ivug_MainView *pMainView);
void _on_play_continous_shot(Ivug_MainView *pMainView);
void _set_thumblist_mode(Ivug_MainView *pMainView, Media_Data *mdata, Image_Object *image_obj);


/*
	Destroy detail view
*/
void _delete_details_view(Ivug_MainView *pMainView );

#ifdef __cplusplus
}
#endif


#endif 		// __IVUG_MAINVIEW_MENU_H__

