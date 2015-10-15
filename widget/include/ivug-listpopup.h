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

#ifndef __IVUG_LISTPOPUP_H__
#define __IVUG_LISTPOPUP_H__

#include <stdbool.h>
#include "ivug-language-mgr.h"

typedef struct _ListPopupItem Ivug_ListPopup_Item;

#ifdef __cplusplus
extern "C" {
#endif

/*
	Signals
		"popup,selected"	event_info contains Ivug_ListPopup_Item which is used when append/prepend
		"popup,dismissed"	closed button pressed, in case of contextual, no dismised callback

		"popup,btn,selected" trigger when button is clicked. event_info has index. and index is order of button using in ivug_listpopup_button_set

*/

typedef enum {
	IVUG_POPUP_STYLE_LIST,		/* Default */
	IVUG_POPUP_STYLE_RADIO,
} ivug_popup_style;

Evas_Object *ivug_listpopup_add(Evas_Object *parent);
void ivug_listpopup_lang_set(Evas_Object *obj, language_handle_t hLang);

void ivug_listpopup_set_style(Evas_Object *obj, ivug_popup_style style);

Ivug_ListPopup_Item *ivug_listpopup_item_append(Evas_Object *obj,  const char *iconpath, const char *caption, void *data);
Ivug_ListPopup_Item *ivug_listpopup_item_prepend(Evas_Object *obj, const char *iconpath, const char *caption, void *data);


void ivug_listpopup_item_clear(Evas_Object *obj);


/*
	Eina list includes (Ivug_ListPopup_Item *) - (Ivug_ListPopup_Item *) - (Ivug_ListPopup_Item *) ....
*/
Eina_List /* Ivug_ListPopup_Item * */ *ivug_listpopup_items_get(Evas_Object *obj);

/*
	Valid only when genlist style popup
*/
void ivug_listpopup_button_set(Evas_Object *obj, const char* caption1, ...);
void ivug_listpopup_title_set(Evas_Object *obj, const char* title);


void *ivug_listpopup_item_get_data(Ivug_ListPopup_Item *item);
void ivug_listpopup_item_disabled_set(Ivug_ListPopup_Item *item);
void ivug_listpopup_item_enabled_set(Ivug_ListPopup_Item *item);
void ivug_listpopup_item_selected_set(Ivug_ListPopup_Item *item);

const char *ivug_listpopup_item_get_text(Ivug_ListPopup_Item *item);


Ivug_ListPopup_Item *ivug_listpopup_item_find_by_data(Evas_Object *obj, void *data);

void ivug_listpopup_max_showitem_count(Evas_Object *obj, unsigned int cnt);



/*
	Show Time!
*/
bool ivug_listpopup_popup_show(Evas_Object *obj);
bool ivug_listpopup_context_show(Evas_Object *obj, Evas_Object *hover, int x, int y);

// Valid only context popup
void ivug_listpopup_context_get_geometry(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h);

bool ivug_listpopup_context_set_rotate_enable(Evas_Object *obj, bool enable);
bool ivug_listpopup_context_get_rotate_enable(Evas_Object *obj);

bool ivug_listpopup_context_move(Evas_Object *obj, int x, int y);

#ifdef __cplusplus
}
#endif


#endif 	// __IVUG_LISTPOPUP_H__

