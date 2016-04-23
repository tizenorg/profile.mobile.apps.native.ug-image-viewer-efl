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

#ifndef __IVUG_WIDGET_H__
#define __IVUG_WIDGET_H__

#include "ivug-language-mgr.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	Internal use only... use macro DELETE_LOG instead
*/
void ivug_on_obj_deleted(Evas_Object* obj, const char *msg, const char *func, int line);

void ivug_on_obj_deleted_cancel(Evas_Object* obj);

#define DELETE_NOTIFY(obj)	\
	ivug_on_obj_deleted(obj, #obj, __func__, __LINE__)

#define DELETE_NOTIFY_CANCEL(obj)	\
	ivug_on_obj_deleted_cancel(obj)

/*
	Create elm_bg with color - r,g,b
*/
Evas_Object *
ivug_bg_add(Evas_Object* parent, int r, int g, int b);
#if 0 //Chandan
Evas_Object *
ivug_layout_add(Evas_Object *win, const char *edjname, const char *groupname);
#endif
Evas_Object *
ivug_layout_add2(Evas_Object *parent, const char *edj, const char *group);

Evas_Object*
ivug_default_layout_add( Evas_Object *win);

Evas_Object *
ivug_button_add(Evas_Object *parent, const char *style, language_handle_t hLang, const char *caption, Evas_Object *icon, Evas_Smart_Cb pFunc, const void *data );

Evas_Object *
ivug_icon_add(Evas_Object *parent, const char *edjname, const char *groupname);

Evas_Object *
ivug_toolbar_add(Evas_Object *parent, const char *style);

Elm_Object_Item *
ivug_toolbar_item_append(Evas_Object *toolbar, const char *text_id, Evas_Smart_Cb callback, void *data);


/*
	Set indicator mode.
		bOverlap = true, app draws below indicator.
		if not, app region & indicator region is split

*/

void _ivug_set_indicator_overlap_mode(const char *func, int line, bool bOverlap);

#define ivug_set_indicator_overlap_mode(bOverlap) _ivug_set_indicator_overlap_mode(__FUNCTION__, __LINE__, bOverlap)



/*
	Set indicator visibility.
*/

void _ivug_set_indicator_visibility(const char *func, int line, Evas_Object *win, bool bShow);

#define ivug_set_indicator_visibility(win,bShow) _ivug_set_indicator_visibility(__FUNCTION__, __LINE__, win,bShow)




#ifdef __cplusplus
}
#endif


#endif // __IVUG_WIDGET_H__

