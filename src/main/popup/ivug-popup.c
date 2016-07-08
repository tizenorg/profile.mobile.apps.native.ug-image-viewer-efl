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

#include <efl_extension.h>

#include "ivug-popup.h"
#include "ivug-vibration.h"
#include "ivug-name-view.h"

#include "ivug-language-mgr.h"
#include "ivug-context.h"

#define NOTIFY_TIMEOUT	3.0

#define IVUG_POPUP_EDJ_NAME full_path(EDJ_PATH, "/ivug-popup.edj")

#define TAG_BUDDY_ACCESS_FONT_SIZE 40
#define TAG_BUDDY_ACCESS_FONT_COLOR "#FFFFFFFF"
#define TAG_BUDDY_ACCESS_BUFFER 1024

typedef struct _Ivug_Popup
{
	Evas_Object *parent;
	Evas_Object *popup;
	Evas_Object *layout;

	Evas_Object *obj;

	int selected_index;

	Popup_Response response;

#ifdef USE_MAXLENGTH_VIBE
	vibration_h haptic_handle;
#endif

	Evas_Smart_Cb callback;
	void *data;

}Ivug_Popup;

typedef struct
{
	int index;
	char *name;
	char *icon_path;
	Evas_Object *radio_main;
	Ivug_Popup *p_iv_popup;
	Elm_Object_Item *item;
} ivug_radio_popup_item;

static Ivug_Popup * ivug_popup_create()
{
	Ivug_Popup *iv_popup = calloc(1, sizeof(Ivug_Popup));

#ifdef USE_MAXLENGTH_VIBE
	iv_popup->haptic_handle = INVALID_HAPTIC_HANDLE;
#endif

	return iv_popup;
}

static void ivug_popup_delete(Ivug_Popup *iv_popup)
{
#ifdef USE_MAXLENGTH_VIBE
	if (iv_popup->haptic_handle != INVALID_HAPTIC_HANDLE)
	{
		ivug_vibration_stop(iv_popup->haptic_handle);
		ivug_vibration_delete(iv_popup->haptic_handle);

		iv_popup->haptic_handle = INVALID_HAPTIC_HANDLE;
	}
#endif

	if (iv_popup->popup)
	{
		evas_object_del(iv_popup->popup);
		iv_popup->popup = NULL;
	}

	if (iv_popup->layout)
	{
		evas_object_del(iv_popup->layout);
		iv_popup->layout = NULL;
	}

	free(iv_popup);
}


static void _on_btn_back_clicked(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_Popup *iv_popup = (Ivug_Popup *)data;

	if (iv_popup->callback)
	{
		iv_popup->response = POPUP_RESPONSE_CANCEL;
		iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
	}

	ivug_popup_delete(iv_popup);
}


static void _on_btn_back_progressing_clicked(void *data, Evas_Object *obj, void *event_info)
{
	MSG_IMAGEVIEW_HIGH("Ignore back button");
}

static void _on_popup_response(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_Popup *iv_popup = (Ivug_Popup *)data;
	Popup_Response response = (Popup_Response)evas_object_data_get(obj, "response");
	MSG_IMAGEVIEW_HIGH("response callback=%d", response);

	if (iv_popup->callback)
	{
		iv_popup->response = response;
		iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
	}

	ivug_popup_delete(iv_popup);
}


static void _on_ctxpopup_dismissed(void *data, Evas_Object *obj, void *event_info)
{
// Triggered when clicked outside ctxpopup
	MSG_IMAGEVIEW_HIGH("Dismissed response");

	Ivug_Popup *iv_popup = (Ivug_Popup *)data;

	if (iv_popup->callback)
	{
		iv_popup->response = LPPOPUP_RESPONSE_NONE;
		iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
	}

	ivug_popup_delete(iv_popup);
}

Evas_Object*
ivug_popup_add(Evas_Object* parent, const char* title_id)
{
	Evas_Object *popup;

	popup = elm_popup_add(parent);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (title_id)
		ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);

	return popup;
}

static Evas_Object *ivug_button_add(Evas_Object *parent, const char *style, language_handle_t hLang, const char *caption_id, Evas_Object *icon, Evas_Smart_Cb pFunc, const void *  data)
{
	IV_ASSERT(parent != NULL);

	Evas_Object *btn;

	btn = elm_button_add(parent);
	if (btn == NULL) {
		return NULL;
	}

	if (style)
		elm_object_style_set(btn, style);

	if (caption_id)
		ivug_elm_object_text_set(hLang, btn, caption_id);

	if (icon)
		elm_object_part_content_set(btn, "icon", icon);

	evas_object_propagate_events_set(btn, EINA_FALSE);

	evas_object_smart_callback_add(btn, "clicked", pFunc, (void*)data);

	return btn;
}

Evas_Object*
ivug_deletepopup_show(Evas_Object* parent, const char* title_id,
					const char* contents_id, Evas_Smart_Cb response, void* user_data)
{
	Evas_Object *popup = NULL;
	Evas_Object *btn_delete = NULL;
	Evas_Object *btn_cancel = NULL;
	//create popup

	IV_ASSERT(response != NULL);

	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	popup = elm_popup_add(parent);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ivug_elm_object_text_set(gGetLanguageHandle(), popup, contents_id);

	if (title_id)
		ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);

	btn_cancel = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_CANCEL, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button1", btn_cancel);
	evas_object_data_set(btn_cancel, "response", (void *)POPUP_RESPONSE_CANCEL);

	btn_delete = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_DELETE, NULL, _on_popup_response, iv_popup);
	//elm_object_style_set(btn_delete, "style1/delete");
	elm_object_part_content_set(popup, "button2", btn_delete);
	evas_object_data_set(btn_delete, "response", (void *)POPUP_RESPONSE_OK);

	iv_popup->popup = popup;
	iv_popup->callback = response;
	iv_popup->data = user_data;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, iv_popup);

	evas_object_show(popup);

	return popup;
}


Evas_Object*
ivug_2btn_popup_show(Evas_Object* parent, const char* title_id,
					const char* contents_id, Evas_Smart_Cb response, void* user_data)
{
	IV_ASSERT(response != NULL);

	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	Evas_Object *popup;
	Evas_Object *btn_yes;
	Evas_Object *btn_no;
	//create popup

	popup = elm_popup_add(parent);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	ivug_elm_object_text_set(gGetLanguageHandle(), popup, contents_id);
	if (title_id)
	{
		ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);
	}

	btn_no = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_NO, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button1", btn_no);
	evas_object_data_set(btn_no, "response", (void *)POPUP_RESPONSE_CANCEL);

	btn_yes = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_YES, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button2", btn_yes);
	evas_object_data_set(btn_yes, "response", (void *)POPUP_RESPONSE_OK);

	iv_popup->popup = popup;
	iv_popup->callback = response;
	iv_popup->data = user_data;

	evas_object_show(popup);
	evas_object_layer_set(popup, EVAS_LAYER_MAX);

	return popup;
}

Evas_Object *ivug_timeout_popup_show(Evas_Object *parent,
	Evas_Smart_Cb response, void *data, const char *sztitle_id, const char *content_id)

{
	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	Evas_Object *popup;

	popup = elm_popup_add(parent);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(popup, GET_STR(content_id));

	if (response)
	{
		iv_popup->callback = response;
	}

	if (data)
	{
		iv_popup->data = data;
	}

	if (sztitle_id)
		elm_object_part_text_set(popup, "title,text", GET_STR(sztitle_id));

	elm_popup_timeout_set(popup, 3.0);
	evas_object_smart_callback_add(popup, "timeout", _on_popup_response, iv_popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, iv_popup);

	iv_popup->popup = popup;

	evas_object_show(popup);

	return popup;

}

Evas_Object*
ivug_longpress_popup_show(Evas_Object *parent, int x, int y, bool bUseExtMenu, Evas_Smart_Cb response, void *data)
{
	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	Evas_Object* ctx_popup = NULL;

	ctx_popup = elm_ctxpopup_add(parent);

	//elm_ctxpopup_item_append(ctx_popup, GET_STR(IDS_COPY_TO_CLIPBOARD), NULL, _on_longpress_popup_selected, iv_popup);

	evas_object_smart_callback_add(ctx_popup, "dismissed", _on_ctxpopup_dismissed, iv_popup);

	evas_object_move(ctx_popup, x, y);

	iv_popup->popup = ctx_popup;
	iv_popup->callback = response;
	iv_popup->data = data;

	evas_object_show(ctx_popup);

	return ctx_popup;
}

void
_ivug_rename_enter_click_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_Popup *iv_popup = (Ivug_Popup *)data;
	IV_ASSERT(iv_popup != NULL);

	Evas_Object *entry = iv_popup->obj;

	char *name = elm_entry_markup_to_utf8 (elm_entry_entry_get(entry));

	MSG_IMAGEVIEW_HIGH("name = %s", name);

	if (name)
	{
		char *new_name = NULL;
		new_name = ivug_strip_string(name);
		if (new_name == NULL)
		{
			MSG_IMAGEVIEW_ERROR("rename failed");
			free(name);
			ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(entry));
			return;
		}
		elm_object_text_set(entry, new_name);
		if (iv_popup->callback)
		{
			iv_popup->response = POPUP_RESPONSE_OK;
			iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
		}
		ivug_popup_delete(iv_popup);
	}
	else
	{
		//ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(entry));
	}
}

void
_ivug_rename_entry_changed(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_Popup *iv_popup = (Ivug_Popup *)data;
	IV_ASSERT(iv_popup != NULL);

	Evas_Object *entry = iv_popup->obj;
	Evas_Object *btn_ok = elm_object_part_content_get(iv_popup->popup, "button1");

	char *content = elm_entry_markup_to_utf8 (elm_entry_entry_get(entry));

	if (content != NULL && strlen(content) == 0)
	{
		MSG_IMAGEVIEW_HIGH("ISF1 : %s", content);
		elm_object_disabled_set(btn_ok, EINA_TRUE);
	}
	else
	{
		MSG_IMAGEVIEW_HIGH("ISF : %s", content);
		elm_object_disabled_set(btn_ok, EINA_FALSE);
	}
	free(content);
}

void
_ivug_rename_maxlength_reached(void *data, Evas_Object *obj, void *event_info)
{
#ifdef USE_MAXLENGTH_VIBE
	Ivug_Popup *iv_popup = (Ivug_Popup *)data;
	IV_ASSERT(iv_popup != NULL);

	if (iv_popup->haptic_handle == INVALID_HAPTIC_HANDLE)
	{
		iv_popup->haptic_handle = ivug_vibration_create();
		if (iv_popup->haptic_handle == INVALID_HAPTIC_HANDLE)
		{
			MSG_IMAGEVIEW_ERROR("ivug_vibration_create failed");
			return;
		}
	}
	else
	{
		ivug_vibration_stop(iv_popup->haptic_handle);
	}

	ivug_vibration_play(iv_popup->haptic_handle ,VIBRATION_DURATION);
#endif
}

Evas_Object *ivug_rename_popup_show(Evas_Object *parent, const char *filename, Evas_Smart_Cb response, void *data)
{
	Evas_Object *popup = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *layout = NULL;

	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	popup = elm_popup_add(parent);
	if (!popup)
	{
		MSG_IMAGEVIEW_ERROR("popup is NULL");
		goto ENTRY_POPUP_FREE;
	}
	//evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_object_style_set(popup, "menustyle");
	elm_object_part_text_set(popup, "title,text", "Rename");
	
	char *popup_edj = IVUG_POPUP_EDJ_NAME;

	layout = ivug_layout_add2(popup, popup_edj, "popup_entryview");
	free(popup_edj);
	if (!layout)
	{
		MSG_IMAGEVIEW_ERROR("layout is NULL");
		goto ENTRY_POPUP_FREE;
	}

	entry = elm_entry_add(popup);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_object_text_set(entry, filename);
	elm_entry_cursor_end_set(entry);
	elm_object_part_content_set(layout, "elm.swallow.content", entry);
	elm_object_content_set(popup, layout);

	evas_object_smart_callback_add(entry, "activated", _ivug_rename_enter_click_cb, iv_popup);
	evas_object_smart_callback_add(entry, "changed", _ivug_rename_entry_changed, iv_popup);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = 64;
	limit_filter_data.max_byte_count = 0;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	evas_object_smart_callback_add(entry, "maxlength,reached", _ivug_rename_maxlength_reached, (void *)iv_popup);

	evas_object_show(layout);

	Evas_Object *btn_cancel = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_CANCEL, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button1", btn_cancel);
	evas_object_data_set(btn_cancel, "response", (void *)POPUP_RESPONSE_CANCEL);

	Evas_Object *btn_ok = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_OK, NULL, _ivug_rename_enter_click_cb, iv_popup);
	elm_object_part_content_set(popup, "button2", btn_ok);
	evas_object_data_set(btn_ok, "response", (void *)POPUP_RESPONSE_OK);

	iv_popup->popup = popup;
	iv_popup->callback = response;
	iv_popup->data = data;
	iv_popup->obj = entry;

	evas_object_show(popup);

	return popup;

ENTRY_POPUP_FREE:
	if (popup)
	{
		evas_object_del(popup);
	}
	ivug_popup_delete(iv_popup);
	return NULL;
}

static void _block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_Popup *iv_popup = (Ivug_Popup *)data;
	Popup_Response response = (Popup_Response)evas_object_data_get(obj, "response");
	MSG_IMAGEVIEW_HIGH("response callback=%d", response);

	if (iv_popup->callback)
	{
		iv_popup->response = response;
		iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
	}

	ivug_popup_delete(iv_popup);
}

static char *
_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	ivug_radio_popup_item *popup_item = (ivug_radio_popup_item *)data;
	//int index = popup_item->index;

	if (strcmp(part, "elm.text") == 0)
	{
		return strdup(GET_STR(popup_item->name));
	}
	return NULL;
}

static Evas_Object *
_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	ivug_radio_popup_item *popup_item = (ivug_radio_popup_item *)data;

	Evas_Object *radio;
	Evas_Object *radio_main = popup_item->radio_main;
	int index = popup_item->index;
	int selected_index = popup_item->p_iv_popup->selected_index;

	if (strcmp(part, "elm.icon") == 0 || strcmp(part, "elm.swallow.icon") == 0)
	{
		radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, radio_main);
		if (index == selected_index)
			elm_radio_value_set(radio, selected_index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);

		return radio;
	}

	return NULL;
}

static void
_gl_del(void *data, Evas_Object *obj)
{
	ivug_radio_popup_item *popup_item = (ivug_radio_popup_item *)data;

	if (popup_item->name)
	{
		free(popup_item->name);
		popup_item->name = NULL;
	}

	free(popup_item);

	return;
}

static void
_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_Popup *iv_popup = (Ivug_Popup *)data;

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	ivug_radio_popup_item *popup_item = NULL;

	MSG_IMAGEVIEW_HIGH("_gl_sel");

	if (item)
	{
		popup_item = (ivug_radio_popup_item *)elm_object_item_data_get(item);
		iv_popup->selected_index = popup_item->index;
		elm_genlist_item_update(item);

		if (iv_popup->callback)
		{
			iv_popup->response = iv_popup->selected_index;
			iv_popup->callback(iv_popup->data, iv_popup->popup, &(iv_popup->response));
		}

		ivug_popup_delete(iv_popup);
	}
}

Evas_Object *ivug_radio_popoup_show(Evas_Object *parent, const char *title_id,
				int selected_index, Eina_List *name_list, Evas_Smart_Cb response, void *data)
{
#define ONE_ITEM_HEIGHT (115)
#define GET_POPUP_HEIGHT(icnt)	((((ONE_ITEM_HEIGHT*icnt)-1) > 408) ? (408) : ((ONE_ITEM_HEIGHT*icnt)-1))
#define GET_POPUP_WIDTH(icnt)	(614)

	Evas_Object *popup;
	int index = 0;
	Elm_Object_Item *item;
	Evas_Object *genlist;

	Evas_Object *radio_main;

	static Elm_Genlist_Item_Class itc = {0,};

	ivug_radio_popup_item *popup_item;

	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	popup = elm_popup_add(parent);
	elm_object_style_set(popup, "content_no_vhpad");
	ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, iv_popup);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#if REMOVE_1_BUTTON
	Evas_Object *btn_close;

	btn_close = ivug_button_add(popup, "popup_button/default",gGetLanguageHandle(),  IDS_CLOSE, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button1", btn_close);
	evas_object_data_set(btn_close, "response", (void *)POPUP_RESPONSE_OK);
#endif

	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "content", "min_menustyle");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	itc.item_style = "1text.1icon.2/popup";
	itc.func.text_get = _gl_text_get;
	itc.func.content_get = _gl_content_get;
	itc.func.state_get = NULL;
	itc.func.del = _gl_del;

	genlist = elm_genlist_add(layout);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_propagate_events_set(genlist, EINA_FALSE);

	radio_main = elm_radio_add(genlist);
	elm_radio_state_value_set(radio_main, -1);
	elm_radio_value_set(radio_main, -1);
	evas_object_propagate_events_set(radio_main, EINA_FALSE);

	Eina_List *l = NULL;
	char *name = NULL;

	EINA_LIST_FOREACH(name_list, l, name)
	{
		popup_item = calloc(1, sizeof(ivug_radio_popup_item));

		if (popup_item != NULL) {
			popup_item->index = index++;
			popup_item->name = strdup(name);
			popup_item->radio_main = radio_main;
			popup_item->p_iv_popup = iv_popup;

			item = elm_genlist_item_append(genlist, &itc, (void *)popup_item, NULL,
				ELM_GENLIST_ITEM_NONE, _gl_sel, iv_popup);

			popup_item->item = item;
		}
	}

	Evas_Object *box;
	box = elm_box_add(layout);
	evas_object_size_hint_min_set(box, GET_POPUP_WIDTH(index-1) * elm_config_scale_get(), GET_POPUP_HEIGHT(index-1) * elm_config_scale_get());
	evas_object_show(genlist);
	elm_box_pack_end(box, genlist);
	elm_object_part_content_set(layout, "elm.swallow.content" , box);
	elm_object_content_set(popup, box);
	//elm_object_content_set(popup, genlist);

	iv_popup->popup = popup;
	iv_popup->callback = response;
	iv_popup->data = data;
	iv_popup->obj = radio_main;
	iv_popup->selected_index = selected_index;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, iv_popup);

	evas_object_show(popup);

	return popup;
}

Evas_Object *ivug_progress_popup_show(Evas_Object *parent, char *title_id,
				       Evas_Smart_Cb response, void *data)
{
	Evas_Object *popup = NULL;

	Ivug_Popup *iv_popup = ivug_popup_create();
	if (iv_popup == NULL) {
		MSG_IMAGEVIEW_ERROR("ivug_popup_create ERROR");
		return NULL;
	}

	popup = elm_popup_add(parent);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(popup, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(popup, "block,clicked", _block_clicked_cb, iv_popup);
	ivug_elm_object_part_text_set(gGetLanguageHandle(), popup, "title,text", title_id);

	Evas_Object *progressbar = elm_progressbar_add(parent);
	elm_object_style_set(progressbar, "list_title_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_progressbar_unit_format_set(progressbar, NULL);
	evas_object_show(progressbar);

	elm_object_content_set(popup, progressbar);

	evas_object_data_set(popup, "pbar", (void *)progressbar);

	Evas_Object *btn_cancel = ivug_button_add(popup, "popup_button/default", gGetLanguageHandle(), IDS_CANCEL, NULL, _on_popup_response, iv_popup);
	elm_object_part_content_set(popup, "button1", btn_cancel);
	evas_object_data_set(btn_cancel, "response", (void *)POPUP_RESPONSE_OK);

	evas_object_show(popup);

	iv_popup->popup = popup;
	iv_popup->callback = response;
	iv_popup->data = data;
	iv_popup->obj = progressbar;

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_clicked, iv_popup);

	return popup;
}

Evas_Object *ivug_processing_popup_show(Evas_Object *parent)
{
	Evas_Object *popup;
	Evas_Object *progressbar;

	Evas_Object *layout;
	Evas_Object *label;

	popup = elm_popup_add(parent);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _on_btn_back_progressing_clicked, NULL);
	label = elm_label_add(popup);
	elm_object_style_set(label, "popup/default");
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	char buf[128];

	snprintf(buf, 128, "<align=center>%s<align>", IDS_PROCESSING);

	elm_object_text_set(label, buf);
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(label, 0.5, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(popup);
	char *popup_edj = IVUG_POPUP_EDJ_NAME;
	Eina_Bool ret = elm_layout_file_set(layout, popup_edj, "popup_processingview_1button");
	if (ret == EINA_FALSE) {
		MSG_IMAGEVIEW_ERROR("Layout file set failed!");
	}
	free(popup_edj);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	elm_progressbar_pulse(progressbar, EINA_TRUE);
	elm_object_style_set(progressbar, "list_process");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	elm_object_part_content_set(layout, "elm.swallow.text", label);

	elm_object_content_set(popup, layout);
	evas_object_show(popup);

	return popup;
}



