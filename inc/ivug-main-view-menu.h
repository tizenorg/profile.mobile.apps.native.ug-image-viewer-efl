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

#define MAX_BUTTON (3)
#define ONE_ITEM_HEIGHT (114)
#define GET_POPUP_HEIGHT(icnt)	((ONE_ITEM_HEIGHT*(icnt)))
#define GET_POPUP_WIDTH(icnt)	(618)

#ifdef __cplusplus
extern "C" {
#endif

struct Ivug_ListPopup_Item;

typedef enum {
	IVUG_POPUP_STYLE_LIST,		/* Default */
	IVUG_POPUP_STYLE_RADIO,
} ivug_popup_style;

typedef enum {
	IVUG_LISTPOPUP_TYPE_HIDDEN,
	IVUG_LISTPOPUP_TYPE_POPUP,
	IVUG_LISTPOPUP_TYPE_CTX_POPUP,
} Ivug_ListPopup_Type;

typedef enum {
	IVUG_LISTPOPUP_STATE_NONE,
	IVUG_LISTPOPUP_STATE_DISMISSED,
	IVUG_LISTPOPUP_STATE_MAX,
} Ivug_ListPopup_State;

typedef struct {
	Evas_Object *obj;		// Dummy Box object

	Evas_Object *parent;

	Evas_Object *popup;
	Evas_Object *box;		// Internal box object

	Evas_Object *genlist;

	Evas_Object *rgroup;		// Radio group

	Eina_List *list;

	unsigned int show_count;
	ivug_popup_style style;

	const char *title;

	const char *btn_caption[MAX_BUTTON];

//	int selected_index;
	Ivug_ListPopup_Type eStatus; 		// Popup is currently displaying?
	Ivug_ListPopup_State state;

	Ivug_ListPopup_Item *item_selected;

	bool bIconUsed;
	bool bRotationEnable;
	language_handle_t hLang;
} ListPopup;

struct Ivug_ListPopup_Item {
	Evas_Object *obj;		// This object

	Elm_Object_Item *item;		// Gengrid item

	const char *iconpath;
	const char *caption_id;
	void *data;

	bool bDisabled;

	int index;		// Checkbox index
	ListPopup *pListPopup;
};

#define IV_LISTPOPUP(obj) \
		(ListPopup *)(evas_object_data_get((obj), "LISTPOPUP"))

void ivug_notification_create(const char* text);

/*
	Button handlers for main view
*/

void on_btn_slideshow_clicked(Ivug_MainView *pMainView);

Evas_Object* ivug_ctx_popup_create(Ivug_MainView *pMainView);

bool ivug_listpopup_context_set_rotate_enable(Evas_Object *obj, bool enable);

bool ivug_listpopup_context_get_rotate_enable(Evas_Object *obj);

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

