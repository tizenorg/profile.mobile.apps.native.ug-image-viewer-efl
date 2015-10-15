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

#include "ivug-debug.h"

#include <Elementary.h>
#include <stdarg.h>
#include <efl_extension.h>

#include "ivug-listpopup.h"
#include "ivug-language-mgr.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-POPUP"

#define MAX_BUTTON (3)

#define ONE_ITEM_HEIGHT (114)
#define GET_POPUP_HEIGHT(icnt)	((ONE_ITEM_HEIGHT*(icnt)))
#define GET_POPUP_WIDTH(icnt)	(618)

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


struct _ListPopupItem {
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

static char *
_on_label_set(void *data, Evas_Object *obj, const char *part)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;

	MSG_LOW("_on_label_set Part %s", part);

	return strdup(GET_STR(item->caption_id)); //dump
}


static Evas_Object                  *
_on_content_set(void *data, Evas_Object *obj, const char *part)
{
	IV_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)data;
	ListPopup *pListPopup = item->pListPopup;

	Evas_Object *radio;

	if (strcmp(part, "elm.icon.2") == 0)
	{
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, item->index);

		elm_radio_group_add(radio, pListPopup->rgroup);

		if (pListPopup->item_selected == item)
		{
			elm_radio_value_set(radio, item->index);
		}

		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return radio;
	}
	else if (strcmp(part, "elm.icon.1") == 0)
	{
		Evas_Object *icon;

		icon = elm_icon_add(obj);
		elm_image_file_set(icon, PREFIX"/res/edje/"PACKAGE"/ivug-icons.edj"	, item->iconpath);

		return icon;
	}

	MSG_HIGH("_on_content_set Part %s", part);

	return NULL;
}



static void
_on_genlist_selected(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = data;
	ListPopup *pListPopup = item->pListPopup;

	int index;

	index = item->index;
	MSG_HIGH("Selected Index = %d", index);

//	pListPopup->selected_index = index;
	pListPopup->item_selected = item;

	MSG_SEC("Genlist item selected. item=0x%08x %s", item, item->caption_id);

	elm_genlist_item_update(item->item);

	evas_object_smart_callback_call(item->obj, "popup,selected", item);

	elm_genlist_item_selected_set(item->item, EINA_FALSE);
}


static void
_on_ctxpopup_selected(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

//
//	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	Ivug_ListPopup_Item *item = data;
//	ListPopup *pListPopup = IV_LISTPOPUP(item->obj);

	MSG_SEC("Ctxpopup item selected. item=0x%08x %s", item, item->caption_id);

	evas_object_smart_callback_call(item->obj, "popup,selected", item);
}


static void _on_item_del(void *data, Evas_Object *obj)
{
	MSG_ASSERT(data != NULL);

	Ivug_ListPopup_Item *item = data;

	MSG_MED("Remove item. obj=0x%08x", obj);

	if (item->caption_id)
		eina_stringshare_del(item->caption_id);
	item->caption_id = NULL;

	if (item->iconpath)
		eina_stringshare_del(item->iconpath);
	item->iconpath = NULL;

	free(item);
}

static void
_on_popup_btn_click(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

	ListPopup *pListPopup = data;

	int nIndex = (int)evas_object_data_get(obj, "_index");
	MSG_HIGH("Genlist Popup Btn click. pListPopup=0x%08x Index=%d", pListPopup, nIndex);

	evas_object_smart_callback_call(pListPopup->obj, "popup,btn,selected", (void *)nIndex);
}

static void _on_listpopup_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	ListPopup *pListPopup = data;

	MSG_HIGH("Remove genlist popup. pListPopup=0x%08x", pListPopup);

	if (pListPopup->state != IVUG_LISTPOPUP_STATE_DISMISSED)
	{
		pListPopup->state = IVUG_LISTPOPUP_STATE_DISMISSED;
		evas_object_smart_callback_call(pListPopup->obj, "popup,dismissed", NULL);
	}

	if (pListPopup->title)
	{
		eina_stringshare_del(pListPopup->title);
		pListPopup->title = NULL;
	}

	int i = 0;

	for (i = 0; i < MAX_BUTTON; i++)
	{
		if (pListPopup->btn_caption[i])
		{
			eina_stringshare_del(pListPopup->btn_caption[i]);
		}
	}

	if (pListPopup->genlist)
	{
		evas_object_hide(pListPopup->genlist);

// After item is cleaed. selected callback is comming. so not removing genlist
//		elm_genlist_clear(pListPopup->genlist);
		evas_object_del(pListPopup->genlist);
	}

	if (pListPopup->box)
	{
		evas_object_hide(pListPopup->box);
		evas_object_del(pListPopup->box);
	}

	if (pListPopup->popup)
	{
		evas_object_hide(pListPopup->popup);
		evas_object_del(pListPopup->popup);
	}

	free(pListPopup);

}


static void _on_ctxpopup_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	ListPopup *pListPopup = data;

	MSG_HIGH("Remove ctx popup. pListPopup=0x%08x", pListPopup);

	if (pListPopup->title)
		eina_stringshare_del(pListPopup->title);

	pListPopup->title = NULL;

	int i = 0;

	for (i = 0; i < MAX_BUTTON; i++)
	{
		if (pListPopup->btn_caption[i])
		{
			eina_stringshare_del(pListPopup->btn_caption[i]);
		}
	}

	void *item;

	EINA_LIST_FREE(pListPopup->list, item)
	{
		_on_item_del(item, NULL);
	}

	pListPopup->list = NULL;

	if (pListPopup->popup)
	{
		evas_object_del(pListPopup->popup);
	}

	free(pListPopup);
}

static void _on_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	ListPopup *pListPopup = data;

	MSG_HIGH("Ctx Popup Dismissed. pListPopup=0x%08x", pListPopup);

	pListPopup->state = IVUG_LISTPOPUP_STATE_DISMISSED;
	evas_object_smart_callback_call(pListPopup->obj, "popup,dismissed", NULL);
}


static void _on_btn_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	ListPopup *pListPopup = data;

	if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_POPUP)
	{

		MSG_HIGH("List Popup Dismissed. pListPopup=0x%08x", pListPopup);
		pListPopup->state = IVUG_LISTPOPUP_STATE_DISMISSED;
		evas_object_smart_callback_call(pListPopup->obj, "popup,dismissed", NULL);

// Should be removec explictly by User!
//		evas_object_del(pListPopup->obj);
	}
	else if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_CTX_POPUP)
	{
		MSG_HIGH("Ctx Popup Dismissed. pListPopup=0x%08x", pListPopup);

		elm_ctxpopup_dismiss(pListPopup->popup);
	}
}


Evas_Object *ivug_listpopup_add(Evas_Object *parent)
{
	Evas_Object *obj = NULL;

// Creating dummy object because we cannot get determine popup or ctxpopup here.
	obj = elm_box_add(parent);

	ListPopup *pListPopup = (ListPopup *)calloc(1, sizeof(ListPopup));

	if (pListPopup != NULL) {
		pListPopup->parent = parent;
		pListPopup->list = NULL;
		pListPopup->obj = obj;
		pListPopup->style = IVUG_POPUP_STYLE_LIST;
		pListPopup->eStatus = IVUG_LISTPOPUP_TYPE_HIDDEN;
		pListPopup->show_count = 0;
		evas_object_data_set(obj, "LISTPOPUP", (void *)pListPopup);
	}

	MSG_HIGH("List popup is added. obj=0x%08x", obj);

	return obj;
}

void ivug_listpopup_lang_set(Evas_Object *obj, language_handle_t hLang)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		pListPopup->hLang = hLang;
	}
}

void ivug_listpopup_set_style(Evas_Object *obj, ivug_popup_style style)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		pListPopup->style = style;
	}
}

Ivug_ListPopup_Item *ivug_listpopup_item_append(Evas_Object *obj, const char *iconpath, const char *caption_id, void *data)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return NULL;
	}

	Ivug_ListPopup_Item *pItem = calloc(1,sizeof(Ivug_ListPopup_Item));

	if (pItem != NULL) {
		pItem->obj = obj;
	}

	if (iconpath && pItem != NULL)
	{
		pItem->iconpath = eina_stringshare_add(iconpath);
		pListPopup->bIconUsed = true;
	}

	if (caption_id && pItem != NULL)
	{
		pItem->caption_id = eina_stringshare_add(caption_id);
	}

	if (pItem != NULL) {
		pItem->bDisabled = false;
		pItem->data = data;
		pItem->pListPopup = pListPopup;
	}

	MSG_MED("Item append: %s", caption_id);

	pListPopup->list = eina_list_append(pListPopup->list, pItem);

	int newIndex = eina_list_count(pListPopup->list);

// If already showed. need to update dynamically
	if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_POPUP && pItem != NULL)
	{
		static Elm_Genlist_Item_Class itc = {0,};

		itc.version = ELM_GENLIST_ITEM_CLASS_VERSION;

		if (pListPopup->style == IVUG_POPUP_STYLE_RADIO)
		{
			itc.item_style = "1text.2icon.6/popup";
			itc.func.content_get = _on_content_set;
		}
		else
		{
			itc.item_style = "1text/popup";
			itc.func.content_get = NULL;
		}

		itc.func.text_get = _on_label_set;
		itc.func.state_get = NULL;
		itc.func.del = _on_item_del;

		Elm_Object_Item *gItem;

		MSG_MED("Add popup item. %s", pItem->caption_id);
		gItem = elm_genlist_item_append(pListPopup->genlist, &itc, pItem, NULL /* parent */, ELM_GENLIST_ITEM_NONE, _on_genlist_selected, pItem);
		pItem->item = gItem;
		pItem->index = newIndex;

		elm_object_item_disabled_set(gItem, pItem->bDisabled);

// When item count is increase, popup height should be increased..
		unsigned int idx;

		idx = pListPopup->show_count;

		if (pListPopup->show_count == 0)
		{
			// Default policy.
			idx = newIndex;

	//		if (count < 2) idx = 2;

	// Check landscape mode also.
			if (newIndex > 4) idx = 4;
		}
		else
		{
			idx = pListPopup->show_count;
		}

		evas_object_size_hint_min_set(pListPopup->box, GET_POPUP_WIDTH(idx) * elm_config_scale_get(), GET_POPUP_HEIGHT(idx) * elm_config_scale_get());

	}

	return pItem;
}


Ivug_ListPopup_Item *ivug_listpopup_item_prepend(Evas_Object *obj, const char *iconpath, const char *caption_id, void *data)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	Ivug_ListPopup_Item *item = (Ivug_ListPopup_Item *)calloc(1, sizeof(Ivug_ListPopup_Item));

	if (item != NULL) {
		item->obj = obj;
	}

	if (iconpath && pListPopup != NULL && item != NULL)
	{
		item->iconpath = eina_stringshare_add(iconpath);
		pListPopup->bIconUsed = true;
	}

	if (caption_id && item != NULL)
	{
		item->caption_id = eina_stringshare_add(caption_id);
	}

	if (item != NULL) {
		item->bDisabled = false;
		item->data = data;
		item->pListPopup = pListPopup;
	}

	MSG_MED("Item prepend: %s", caption_id);

	if (pListPopup != NULL && item != NULL) {
		pListPopup->list = eina_list_prepend(pListPopup->list, item);
	}

// TODO If already showed. need to update dynamically

	return item;
}

void ivug_listpopup_item_clear(Evas_Object *obj)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_HIDDEN) {
			Ivug_ListPopup_Item *item;
			EINA_LIST_FREE(pListPopup->list, item)
			{
				_on_item_del(item, NULL);
			}
		} else if (pListPopup->eStatus == IVUG_LISTPOPUP_TYPE_POPUP) {
			elm_genlist_clear(pListPopup->genlist);
		}

		eina_list_free(pListPopup->list);
		pListPopup->list = NULL;
	}
}



Eina_List *ivug_listpopup_items_get(Evas_Object *obj)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		return pListPopup->list;
	} else {
		return NULL;
	}
}


void ivug_listpopup_item_disabled_set(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	item->bDisabled = true;
}

void ivug_listpopup_item_enabled_set(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	item->bDisabled = false;
}

void *ivug_listpopup_item_get_data(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	return item->data;
}

const char *ivug_listpopup_item_get_text(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	return item->caption_id;
}


void ivug_listpopup_item_selected_set(Ivug_ListPopup_Item *item)
{
// Only one item should be checked.
	IV_ASSERT(item != NULL);

	ListPopup *pListPopup = item->pListPopup;

	pListPopup->item_selected = item;
}


Ivug_ListPopup_Item *ivug_listpopup_item_find_by_data(Evas_Object *obj, void *data)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	Ivug_ListPopup_Item *pItem = NULL;
	Eina_List *tmp;

	if (pListPopup != NULL) {
		EINA_LIST_FOREACH(pListPopup->list, tmp, pItem)
		{
			if (pItem->data == data)
			{
				return pItem;
			}
		}
	}

	return NULL;

}


Elm_Object_Item *ivug_listpopup_item_get_item(Ivug_ListPopup_Item *item)
{
	IV_ASSERT(item != NULL);

	return item->item;
}

void ivug_listpopup_title_set(Evas_Object *obj, const char* title_id)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	MSG_ASSERT(title_id != NULL);

	if (pListPopup != NULL) {
		pListPopup->title = eina_stringshare_add(title_id);
	}

	MSG_MED("Popup title set : %s", title_id);
}

void ivug_listpopup_max_showitem_count(Evas_Object *obj, unsigned int cnt)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	MSG_ASSERT(cnt != 0);

	if (pListPopup != NULL) {
		pListPopup->show_count = cnt;
	}

	MSG_MED("Popup set show count : %d", cnt);
}


void ivug_listpopup_button_set(Evas_Object *obj, const char* caption_id1, ...)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	int i = 0;

	if (caption_id1 && pListPopup != NULL)
	{
		pListPopup->btn_caption[i++] = eina_stringshare_add(caption_id1);
		MSG_HIGH("Popup caption[%d] set : %s", i, caption_id1);
	}

	va_list ap;

	va_start(ap, caption_id1);

	const char *caption = va_arg(ap, const char *);

	while (caption != NULL) {
		if (caption && pListPopup != NULL) {
			pListPopup->btn_caption[i++] = eina_stringshare_add(caption);
			MSG_HIGH("Popup caption[%d] set : %s", i, caption);
		}

		caption = va_arg(ap, const char *);
	}

	va_end(ap);


}

bool ivug_listpopup_popup_show(Evas_Object *obj)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return false;
	}

	if (pListPopup->popup) {
		MSG_WARN("Previous popup is remained");
	}

	Evas_Object *popup;
// create popup
	popup = elm_popup_add(pListPopup->parent);

	if (!popup)
	{
		MSG_ERROR("Error : popup create failed.");
		return false;
	}

	pListPopup->popup = popup;

	elm_object_style_set(popup, "content_no_vhpad");

	if (pListPopup->title)
	   	ivug_elm_object_part_text_set(pListPopup->hLang, popup, "title,text", pListPopup->title);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "min_menustyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Evas_Object *btn_close = NULL;

	if (pListPopup->btn_caption[0])
	{
		btn_close = elm_button_add(popup);
		elm_object_style_set(btn_close, "popup_button/default");
		ivug_elm_object_text_set(pListPopup->hLang, btn_close, pListPopup->btn_caption[0]);
		elm_object_part_content_set(popup, "button1", btn_close);
		evas_object_data_set(btn_close, "_index", (void *)0);

		evas_object_smart_callback_add(btn_close, "clicked", _on_popup_btn_click, pListPopup);
	}

	if (pListPopup->btn_caption[1])
	{
		btn_close = elm_button_add(popup);
		elm_object_style_set(btn_close, "popup_button/default");
		ivug_elm_object_text_set(pListPopup->hLang, btn_close, pListPopup->btn_caption[1]);
		elm_object_part_content_set(popup, "button2", btn_close);

		evas_object_data_set(btn_close, "_index", (void *)1);
		evas_object_smart_callback_add(btn_close, "clicked", _on_popup_btn_click, pListPopup);
	}

	if (pListPopup->btn_caption[2])
	{
		btn_close = elm_button_add(popup);
		elm_object_style_set(btn_close, "popup_button/default");
		ivug_elm_object_text_set(pListPopup->hLang, btn_close, pListPopup->btn_caption[2]);
		elm_object_part_content_set(popup, "button3", btn_close);

		evas_object_data_set(btn_close, "_index", (void *)2);
		evas_object_smart_callback_add(btn_close, "clicked", _on_popup_btn_click, pListPopup);
	}


// create genlist
	Evas_Object *genlist;
	static Elm_Genlist_Item_Class itc = {0,};

	genlist = elm_genlist_add(layout);

	pListPopup->genlist = genlist;

	itc.version = ELM_GENLIST_ITEM_CLASS_VERSION;

	if (pListPopup->style == IVUG_POPUP_STYLE_RADIO)
	{
		itc.item_style = "1text.2icon.6/popup";
		itc.func.content_get = _on_content_set;

		pListPopup->rgroup = elm_radio_add(genlist);
		elm_radio_state_value_set(pListPopup->rgroup, 0);
		elm_radio_value_set(pListPopup->rgroup, 0);
	}
	else
	{
		itc.item_style = "1text/popup";
	   	itc.func.content_get = NULL;
	}

	itc.func.text_get = _on_label_set;
   	itc.func.state_get = NULL;
   	itc.func.del = _on_item_del;

	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	int count = eina_list_count(pListPopup->list);

	Ivug_ListPopup_Item *pItem = NULL;
	Eina_List *tmp;
	Elm_Object_Item *gItem;

	int index = 1;

	EINA_LIST_FOREACH(pListPopup->list, tmp, pItem)
	{
		MSG_MED("Add popup item. %s", pItem->caption_id);
		gItem = elm_genlist_item_append(genlist, &itc, pItem, NULL /* parent */, ELM_GENLIST_ITEM_NONE, _on_genlist_selected, pItem);
		pItem->item = gItem;
		pItem->index = index++;

		elm_object_item_disabled_set(gItem, pItem->bDisabled);
	}


// Put together
	unsigned int idx;

	idx = pListPopup->show_count;

	if (pListPopup->show_count == 0)
	{
		// Default policy.
		idx = count;

//		if (count < 2) idx = 2;

// Check landscape mode also.
		if (count > 4) idx = 4;
	}
	else
	{
		idx = pListPopup->show_count;
	}

	Evas_Object *box;
	box = elm_box_add(layout);
	evas_object_size_hint_min_set(box, GET_POPUP_WIDTH(idx) * elm_config_scale_get(), GET_POPUP_HEIGHT(idx) * elm_config_scale_get());

//	MSG_WARN("Set box size WH(%d,%d)", GET_POPUP_WIDTH(idx) * elm_config_scale_get(), GET_POPUP_HEIGHT(idx) * elm_config_scale_get());

	elm_box_pack_end(box, genlist);

	elm_object_part_content_set(layout, "elm.swallow.content" , box);

	elm_object_content_set(popup, box);

	evas_object_show(genlist);

	pListPopup->box = box;

	evas_object_event_callback_add(pListPopup->obj, EVAS_CALLBACK_DEL, _on_listpopup_deleted, pListPopup);

	MSG_HIGH("Show Genlist Popup");
	pListPopup->eStatus = IVUG_LISTPOPUP_TYPE_POPUP;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, pListPopup);

	evas_object_show(popup);

	return true;
}


bool ivug_listpopup_context_show(Evas_Object *obj, Evas_Object *hover, int x, int y)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return false;
	}

	if (pListPopup->popup)
	{
		MSG_WARN("Previous popup is remained");
	}

	Evas_Object* ctxpopup = NULL;
	ctxpopup = elm_ctxpopup_add(hover);

	if (!ctxpopup)
	{
		MSG_ERROR("Error : popup create failed.");
		return false;
	}

	pListPopup->popup = ctxpopup;

	elm_object_style_set(ctxpopup, "more/default");	// for more

	elm_ctxpopup_hover_parent_set(ctxpopup, hover);

	{
		Evas_Object *obj = hover;
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		MSG_SEC("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, pass, repeat, visible, propagate);
	}

	elm_ctxpopup_direction_priority_set(ctxpopup, ELM_CTXPOPUP_DIRECTION_DOWN,
												ELM_CTXPOPUP_DIRECTION_UP,
												ELM_CTXPOPUP_DIRECTION_RIGHT,
												ELM_CTXPOPUP_DIRECTION_LEFT);


	Ivug_ListPopup_Item *pItem = NULL;
	Eina_List *tmp;
	Elm_Object_Item *gItem;

	Evas_Object *icon;

	EINA_LIST_FOREACH(pListPopup->list, tmp, pItem)
	{
		MSG_MED("Add popup item. %s", pItem->caption_id);

		if (pItem->iconpath)
		{
			icon = elm_icon_add(ctxpopup);
			elm_image_file_set(icon, PREFIX"/res/edje/"PACKAGE"/ivug-icons.edj", pItem->iconpath);
		}
		else if (pListPopup->bIconUsed == true)		// For icon align
		{
			icon = elm_icon_add(ctxpopup);
			elm_image_file_set(icon, PREFIX"/res/edje/"PACKAGE"/ivug-icons.edj", "icon.mainmenu.transparent");
		}
		else
		{

			icon = NULL;
		}

		gItem = elm_ctxpopup_item_append(ctxpopup, GET_STR(pItem->caption_id), icon, _on_ctxpopup_selected, pItem);
		pItem->pListPopup = pListPopup;
		pItem->item = gItem;

		elm_object_item_disabled_set(gItem, pItem->bDisabled);
	}

	evas_object_event_callback_add(pListPopup->obj, EVAS_CALLBACK_DEL, _on_ctxpopup_deleted, pListPopup);

	evas_object_smart_callback_add(ctxpopup, "dismissed", _on_dismissed_cb, pListPopup);

	if (pListPopup->bRotationEnable)
		elm_ctxpopup_auto_hide_disabled_set(ctxpopup, EINA_TRUE);

	evas_object_move(ctxpopup, x, y);

	MSG_SEC("Show Context Popup(%d,%d)", x, y);
	pListPopup->eStatus = IVUG_LISTPOPUP_TYPE_CTX_POPUP;

	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, pListPopup);
	eext_object_event_callback_add(ctxpopup, EEXT_CALLBACK_MORE, _on_btn_back_clicked, pListPopup);		// When pressed backbutton 2 time, closed ctx popup

	evas_object_show(ctxpopup);

	return true;
}



void ivug_listpopup_context_get_geometry(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup == NULL) {
		return;
	}

	if (pListPopup->popup == NULL)
	{
		if (x) *x = 0;
		if (y) *y = 0;
		if (w) *w = 0;
		if (h) *h = 0;

		return;
	}

	if (strcmp (evas_object_type_get(pListPopup->popup), "elm_ctxpopup") != 0)
	{
		if (x) *x = 0;
		if (y) *y = 0;
		if (w) *w = 0;
		if (h) *h = 0;

		return;
	}

	evas_object_geometry_get(pListPopup->popup, x, y, w, h);

}

bool ivug_listpopup_context_set_rotate_enable(Evas_Object *obj, bool enable)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		pListPopup->bRotationEnable = enable;
	}

	return true;
}

bool ivug_listpopup_context_get_rotate_enable(Evas_Object *obj)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);
	
	if (pListPopup != NULL) {
		return pListPopup->bRotationEnable;
	} else {
		return false;
	}
}

bool ivug_listpopup_context_move(Evas_Object *obj, int x, int y)
{
	ListPopup *pListPopup = IV_LISTPOPUP(obj);

	if (pListPopup != NULL) {
		evas_object_move(pListPopup->popup, x, y);
	}

	return true;
}

