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
#include <notification.h>

#include "ivug-common.h"
#include "ivug-name-view.h"
#include "ivug-popup.h"
#include "ivug-media.h"
#include "ivug-vibration.h"
#include "ivug-util.h"
#include "ivug-language-mgr.h"
#include <notification.h>

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH

#undef LOG_CAT
#define LOG_CAT "IV-NAME-VIEW"

#define IVUG_POPUP_EDJ_NAME full_path(EDJ_PATH, "/ivug-popup.edj")

struct _Ivug_NameView
{
	Evas_Object *layout;
	Evas_Object *scroller;

	Evas_Object *navibar;
	Elm_Object_Item *navi_it;

	//use popup
	Evas_Object *popup;

	Evas_Object *entry;
	Evas_Object *btn_done;

	char *guide_txt;
	char *init_txt;

#ifdef USE_MAXLENGTH_VIBE
	vibration_h haptic_handle;
#endif

	FNResponse fnresponse;
	void *clientdata;

	ivug_name_mode mode;

	char *filter_txt;
	char *ivTemp_entry;

	Eina_Bool bAllowNull;

};

static void
_on_timeout_response_reset(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

	Ivug_NameView *pNameView = (Ivug_NameView *)data;

	ecore_imf_context_input_panel_show(elm_entry_imf_context_get(pNameView->entry));

	elm_object_focus_set(pNameView->entry, EINA_TRUE);
}

static void
_on_keypad_show(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

#ifdef HIDE_TITLE_AT_LANDSCAPE
	Ivug_NameView *pNameView = (Ivug_NameView *)data;

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(pNameView->navibar);

	int rot = gGetRotationDegree();
	if (rot == 90 || rot == 270) {
		elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
	}
#endif
}

static void
_on_keypad_hide(void *data, Evas_Object *obj, void *event_info)
{
	MSG_ASSERT(data != NULL);

#ifdef HIDE_TITLE_AT_LANDSCAPE
	Ivug_NameView *pNameView = (Ivug_NameView *)data;

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(pNameView->navibar);

	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_FALSE);
#endif

}

void _on_btn_more_clicked(void *data, Evas_Object *obj, void *event_info)
{
}

static void
_ivug_rename_view_on_entry_actiavated(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("Entry Activated callback pNameView(0x%08x)", pNameView);

	if (pNameView->mode == NAME_VIEW_MODE_MULTI_LINE) {
		MSG_HIGH("DEBUG ME. When is called?");
		return;
	}

	Evas_Object *entry = pNameView->entry;

	char *name = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	MSG_HIGH("name = %s", name);

	if (name) {
		char *new_name = NULL;
		new_name = ivug_strip_string(name);
		if (new_name == NULL) {
			MSG_ERROR("ivug_strip_string failed");
			ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(entry));
			ivug_timeout_popup_show(pNameView->layout, _on_timeout_response_reset, pNameView, IDS_ERROR, IDS_ENTRY_IS_EMPTY);
			free (name);
			return;
		}

		if (pNameView->fnresponse) {
			(pNameView->fnresponse)(pNameView, NAME_VIEW_RESPONSE_OK, new_name, pNameView->clientdata);
		}

		free(name);

		/* destroy after pop and transit ended */
		/* ivug_name_view_destroy(pNameView); */
	} else {
		ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(entry));
	}
}

static void
_ivug_rename_view_on_entry_clicked(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	elm_entry_select_none(pNameView->entry);

	MSG_HIGH("Entry Clicked callback pNameView(0x%08x)", pNameView);

	return;
}

static void
_ivug_name_view_maxlength_reached(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

#ifdef USE_MAXLENGTH_VIBE
	if (pNameView->haptic_handle == INVALID_HAPTIC_HANDLE) {
		pNameView->haptic_handle = ivug_vibration_create();
		if (pNameView->haptic_handle == INVALID_HAPTIC_HANDLE) {
			MSG_ERROR("ivug_vibration_create failed");
			return;
		}
	} else {
		ivug_vibration_stop(pNameView->haptic_handle);
	}

	ivug_vibration_play(pNameView->haptic_handle ,VIBRATION_DURATION);
#else
	char message[50] = {0,};
	int ret = -1;
	snprintf(message, 50, GET_STR(IDS_MAX_CHAR_LENGTH_REACHED), MAX_CHAR_LEN);

	ret = notification_status_message_post(message);
	if (ret != NOTIFICATION_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("notification_status_message_post() ERROR [0x%x]", ret);
	}
#endif
}

static void
_ivug_name_view_callback_long_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	evas_object_smart_callback_del(pNameView->entry, "clicked", (Evas_Smart_Cb)
								   _ivug_rename_view_on_entry_clicked);

	return;
}

static void
_ivug_name_view_save_cb(void *data, Evas_Object* obj, void* event_info)
{
	MSG_HIGH("pNameView Saving ");
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	Evas_Object *entry = pNameView->entry;

	char *name = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	MSG_HIGH("name = %s", name);

	if (name) {
		char *new_name = NULL;
		new_name = ivug_strip_string(name);
		if (new_name == NULL) {
			MSG_ERROR("ivug_strip_string failed");
			elm_object_focus_set(entry, EINA_FALSE);
			ivug_timeout_popup_show(pNameView->layout, _on_timeout_response_reset, pNameView, IDS_ERROR, IDS_ENTRY_IS_EMPTY);
			free (name);
			return;
		}

		if (pNameView->fnresponse) {
			(pNameView->fnresponse)(pNameView, NAME_VIEW_RESPONSE_OK, new_name, pNameView->clientdata);
		}

		free(name);
	}

	/* destroy after pop and transit ended */
	/* ivug_name_view_destroy(pNameView); */
}

static void
_ivug_name_view_cancel_cb(void *data, Evas_Object* obj, void* event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("Name view(0x%08x) Cancel callback!", pNameView);

	ecore_imf_context_input_panel_hide(elm_entry_imf_context_get(pNameView->entry));

	if (pNameView->fnresponse) {
		(pNameView->fnresponse)(pNameView, NAME_VIEW_RESPONSE_CANCEL, NULL, pNameView->clientdata);
	}

	if (pNameView->popup) {
		evas_object_del(pNameView->popup);
		pNameView->popup = NULL;
	}
	if (pNameView->ivTemp_entry) {
		free(pNameView->ivTemp_entry);
		pNameView->ivTemp_entry = NULL;
	}
}

static bool _ivug_validate_text(const char *text, const char *filter)
{
	if (!text)
		return false;

	if (filter == NULL)
		return true;

	const char *tmp = text;

	/* hidden property check */
	if (strncmp(tmp, ".", strlen(".")) == 0) {
		return false;
	}

	/* check bad character */
	while (*tmp != '\0') {
		if (strchr(filter, *tmp) != NULL) {
			MSG_ERROR("Invalid text=%s char=%c", text, *tmp);
			return false;
		}
		tmp++;
	}
	return true;
}

static void _ivug_name_view_on_entry_changed(void *data, Evas_Object *obj, void *event_info)
{
	Ivug_NameView *pNameView = (Ivug_NameView *)data;
	MSG_ASSERT(pNameView != NULL);

	Evas_Object *entry = pNameView->entry;

	char *content = elm_entry_markup_to_utf8(elm_entry_entry_get(entry));

	if (!content) {
		return;
	}

	if (strlen(content) == 0) {
		MSG_HIGH("Input string is NULL");
		ivug_elm_object_part_text_set(gGetLanguageHandle(), pNameView->entry, "elm.guide", pNameView->guide_txt);

		if (pNameView->bAllowNull == EINA_FALSE) {
			elm_object_disabled_set(pNameView->btn_done, EINA_TRUE);
		}
	} else if (_ivug_validate_text(content, pNameView->filter_txt) == false) {
		MSG_HIGH("invalid char ISF : %s", content);
		int position = elm_entry_cursor_pos_get(pNameView->entry);
		elm_entry_entry_set(pNameView->entry, elm_entry_utf8_to_markup(pNameView->ivTemp_entry));
		elm_entry_cursor_begin_set(pNameView->entry);
		elm_entry_cursor_pos_set(pNameView->entry, position - 1);
		if (content) {
			free(content);
			content = NULL;
		}
		content = strdup(pNameView->ivTemp_entry);
		int ret = notification_status_message_post(GET_STR(IDS_INVALID_NAME));
		if (ret != NOTIFICATION_ERROR_NONE) {
			MSG_IMAGEVIEW_ERROR("notification_status_message_post() ERROR [0x%x]", ret);
		}

		ivug_name_view_show_imf(pNameView);
	} else {
		MSG_HIGH("ISF: %s", content);
		ivug_elm_object_part_text_set(gGetLanguageHandle(), pNameView->entry, "elm.guide", IDS_NULL);
		char *new_name = ivug_strip_string(content);

		if ((pNameView->init_txt && strcmp(pNameView->init_txt, content) == 0) || new_name == NULL) {
			elm_object_disabled_set(pNameView->btn_done, EINA_TRUE);
		} else {
			elm_object_disabled_set(pNameView->btn_done, EINA_FALSE);
		}
	}
	if (pNameView->ivTemp_entry) {
		free(pNameView->ivTemp_entry);
		pNameView->ivTemp_entry = NULL;
	}
	pNameView->ivTemp_entry = content;
}

static void _ivug_name_view_on_entry_changed_user(void *data, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("Entry changed by user");
}

static void _ivug_name_view_on_entry_preedit_changed(void *data, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("_ivug_name_view_on_entry_preedit_changed");

	_ivug_name_view_on_entry_changed(data, obj, event_info);
}


static Evas_Object *_ivug_name_view_editfield_create(Ivug_NameView *pNameView)
{
	Evas_Object *editfield = NULL;

	if (pNameView->mode == NAME_VIEW_MODE_SINGLE_LINE) {
		editfield = elm_entry_add(pNameView->layout);
		elm_entry_single_line_set(editfield, EINA_TRUE);
		elm_entry_scrollable_set(editfield, EINA_TRUE);
	} else {
		editfield = elm_entry_add(pNameView->layout);
	}

	elm_entry_select_none(pNameView->entry);

	evas_object_smart_callback_add(editfield, "activated", _ivug_rename_view_on_entry_actiavated, pNameView);
	evas_object_smart_callback_add(editfield, "clicked", _ivug_rename_view_on_entry_clicked, pNameView);
	evas_object_smart_callback_add(editfield, "changed", _ivug_name_view_on_entry_changed, pNameView);
	evas_object_smart_callback_add(editfield, "preedit,changed", _ivug_name_view_on_entry_preedit_changed, pNameView);
	evas_object_smart_callback_add(editfield, "changed,user", _ivug_name_view_on_entry_changed_user, pNameView);
	evas_object_smart_callback_add(editfield, "maxlength,reached", (Evas_Smart_Cb)_ivug_name_view_maxlength_reached, pNameView);
	evas_object_smart_callback_add(editfield, "longpressed", (Evas_Smart_Cb)_ivug_name_view_callback_long_clicked_cb, pNameView);

	evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_entry_cnp_mode_set(editfield, ELM_CNP_MODE_PLAINTEXT);

	return editfield;

}

Evas_Object *
ivug_layout_add2(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);

	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_ERROR("edj loading fail, filepath = %s Group = %s", edj, group);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	return layout;
}

static
Evas_Object *ivug_button_add(Evas_Object *parent, const char *style, language_handle_t hLang, const char *caption_id, Evas_Object *icon, Evas_Smart_Cb pFunc, const void *  data)
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

Ivug_NameView *
ivug_name_view_create(Evas_Object *parent, ivug_name_mode mode)
{
	MSG_HIGH("NameView Create");

	MSG_ASSERT(parent != NULL);

	Ivug_NameView *pNameView = calloc(1, sizeof(Ivug_NameView));
	if (pNameView == NULL) {
		MSG_ERROR("Cannot allocated memory");
		return NULL;
	}

#ifdef USE_MAXLENGTH_VIBE
	pNameView->haptic_handle = INVALID_HAPTIC_HANDLE;
#endif

	pNameView->popup = elm_popup_add(parent);
	elm_popup_align_set(pNameView->popup, 0.5, 0.5);

	elm_object_domain_translatable_part_text_set(pNameView->popup, "title,text", IVUG_TEXT_DOMAIN, IDS_RENAME);
	/* Create view base layout */
	pNameView->layout = ivug_layout_add2(pNameView->popup, IVUG_POPUP_EDJ_NAME, "popup_input_text");
	evas_object_name_set(pNameView->layout, "NameBase");
	if (pNameView->layout == NULL) {
		MSG_ERROR("Cannot create layout");
		free(pNameView);
		return NULL;
	}

	/* Create layout & edit field */
	pNameView->entry = _ivug_name_view_editfield_create(pNameView);
	evas_object_name_set(pNameView->entry, "Entry");
	evas_object_show(pNameView->entry);

	evas_object_data_set(pNameView->entry, "entry data", pNameView);

	if (mode == NAME_VIEW_MODE_SINGLE_LINE) {
		elm_object_part_content_set(pNameView->layout, "elm.swallow.content", pNameView->entry);

		elm_object_content_set(pNameView->popup, pNameView->layout);
	}

	Evas_Object *btn_cancel = ivug_button_add(pNameView->popup, "popup_button/default",
											  gGetLanguageHandle(), IDS_CANCEL, NULL, _ivug_name_view_cancel_cb, pNameView);
	elm_object_part_content_set(pNameView->popup, "button1", btn_cancel);

	Evas_Object *btn_done = ivug_button_add(pNameView->popup, "popup_button/default",
											gGetLanguageHandle(), IDS_RENAME_BUTTON, NULL, _ivug_name_view_save_cb, pNameView);
	elm_object_part_content_set(pNameView->popup, "button2", btn_done);

	elm_object_disabled_set(btn_done, EINA_TRUE);

	pNameView->btn_done = btn_done;

	eext_object_event_callback_add(pNameView->popup, EEXT_CALLBACK_BACK, _ivug_name_view_cancel_cb, pNameView);

	evas_object_show(pNameView->popup);

	MSG_HIGH("Created NameView. Obj=0x%08x", pNameView->layout);
	return pNameView;
}


void
ivug_name_view_destroy(Ivug_NameView *pNameView)
{
	ivug_ret_if(pNameView == NULL);

	MSG_HIGH("Destroy Name View");

#ifdef USE_MAXLENGTH_VIBE
	if (pNameView->haptic_handle != INVALID_HAPTIC_HANDLE) {
		ivug_vibration_stop(pNameView->haptic_handle);
		ivug_vibration_delete(pNameView->haptic_handle);

		pNameView->haptic_handle = INVALID_HAPTIC_HANDLE;
	}
#endif

	if (pNameView->navibar) {
		evas_object_del(pNameView->navibar);
		pNameView->navibar = NULL;
	}

	if (pNameView->guide_txt) {
		free(pNameView->guide_txt);
	}

	if (pNameView->init_txt) {
		free(pNameView->init_txt);
		pNameView->init_txt = NULL;
	}

	if (pNameView->popup) {
		evas_object_del(pNameView->popup);
		pNameView->popup = NULL;
	}

	if (pNameView->filter_txt) {
		free(pNameView->filter_txt);
	}

	Evas_Object *conformant = gGetCurrentConformant();//ug_get_conformant();

	evas_object_smart_callback_del(conformant, "virtualkeypad,state,on", _on_keypad_show);
	evas_object_smart_callback_del(conformant, "virtualkeypad,state,off", _on_keypad_hide);
	evas_object_smart_callback_del(conformant, "clipboard,state,on", _on_keypad_show);
	evas_object_smart_callback_del(conformant, "clipboard,state,off", _on_keypad_hide);

	free(pNameView);
	pNameView = NULL;

	MSG_HIGH("Name view removed");
}


Evas_Object *
ivug_name_view_object_get(Ivug_NameView *pNameView)
{
	MSG_ASSERT(pNameView != NULL);

	return pNameView->layout;
}

void
ivug_name_view_set_response_callback(Ivug_NameView *pNameView, FNResponse resp, void *data)
{
	MSG_ASSERT(pNameView != NULL);

	if (pNameView->fnresponse != NULL) {
		MSG_WARN("Old response callback is overwritten");
	}

	pNameView->fnresponse = resp;
	pNameView->clientdata = data;
}


void
ivug_name_view_set_title(Ivug_NameView *pNameView, const char *title)
{
	MSG_ASSERT(pNameView != NULL);

	elm_object_item_part_text_set(pNameView->navi_it, "elm.text.title", title);

	char *domain = ivug_language_mgr_get_text_domain(title);
	elm_object_item_domain_text_translatable_set(pNameView->navi_it, domain, EINA_TRUE);
}

void
ivug_name_view_set_text(Ivug_NameView *pNameView, const char *str)
{
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("Set Text : %s", str);

	char *markup = elm_entry_utf8_to_markup(str);

	if (markup != NULL) {
		pNameView->init_txt = strdup(markup);
	}

	elm_entry_entry_set(pNameView->entry, markup);
	elm_entry_select_all(pNameView->entry);
	elm_object_focus_set(pNameView->entry, EINA_TRUE);

	elm_entry_cursor_end_set(pNameView->entry);

	if (markup) {
		free(markup);
		markup = NULL;
	}
}

void
ivug_name_view_set_focus(Ivug_NameView *pNameView)
{
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("Set Focus on entry. pNameView(0x%08x)", pNameView);

	elm_entry_cursor_end_set(pNameView->entry);
	evas_object_show(pNameView->entry);

	elm_object_focus_set(pNameView->entry, EINA_TRUE);	// show keypad
}

void
ivug_name_view_set_max_length(Ivug_NameView *pNameView, int max_len)
{
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("ivug_name_view_set_max_length to %d", max_len);

	Evas_Object *entry = pNameView->entry;

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = MAX_CHAR_LEN;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);

	evas_object_smart_callback_add(entry, "maxlength,reached", _ivug_name_view_maxlength_reached, (void *) pNameView);
}

void
ivug_name_view_set_guide_text(Ivug_NameView *pNameView, const char *text_id)
{
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("ivug_name_view_set_guide_text %s", text_id);

	Evas_Object *entry = pNameView->entry;

	if (pNameView->guide_txt) {
		free(pNameView->guide_txt);
	}

	pNameView->guide_txt = strdup(text_id);

	ivug_elm_object_part_text_set(gGetLanguageHandle(), entry, "elm.guide", pNameView->guide_txt);
}

void
ivug_name_view_set_filter_text(Ivug_NameView *pNameView, const char *filter_text)
{
	MSG_ASSERT(pNameView != NULL);

	MSG_HIGH("ivug_name_view_set_guide_text %s", filter_text);

	if (pNameView->filter_txt)
		free(pNameView->filter_txt);

	pNameView->filter_txt = strdup(filter_text);
}


void ivug_name_view_allow_null_set(Ivug_NameView *pNameView, Eina_Bool bNullAllowed)
{
	MSG_ASSERT(pNameView != NULL);

	pNameView->bAllowNull = bNullAllowed;

	if (pNameView->bAllowNull == EINA_TRUE) {
		elm_object_disabled_set(pNameView->btn_done, EINA_FALSE);
	}
}

void ivug_name_view_show_imf(Ivug_NameView *pNameView)
{
	MSG_ASSERT(pNameView != NULL);

	if (pNameView->entry) {
		evas_object_show(pNameView->entry);
		elm_object_focus_set(pNameView->entry, EINA_TRUE);
	}
}

