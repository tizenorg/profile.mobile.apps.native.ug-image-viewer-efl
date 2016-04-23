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

#include <ui-gadget-module.h>
#include <Elementary.h>

#include "ivug-debug.h"
#include "ivug-widget.h"

#include "ivug-language-mgr.h"

#define IVUG_TEXT_DOMAIN PACKAGE

static void _on_obj_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	char *szMsg = (char *)data;
	IV_ASSERT(szMsg != NULL);

	MSG_IVUG_HIGH("On Object deleted. %s, 0x%08x", szMsg, obj);

	free(szMsg);
}
#if 0 //Chandan
void ivug_layout_attach_to_window(Evas_Object *parent, Evas_Object* layout)
{
	elm_win_resize_object_add(ug_get_window(), layout);
}
#endif
void ivug_on_obj_deleted(Evas_Object* obj, const char *msg, const char *func, int line)
{
	char buf[128];

	snprintf(buf, sizeof(buf), "%s(L%d):%s 0x%08x", func, line, msg, (unsigned int)obj);

	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, _on_obj_deleted, strdup(buf));
}

void ivug_on_obj_deleted_cancel(Evas_Object* obj)
{
	evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, _on_obj_deleted);
}

Evas_Object* ivug_bg_add(Evas_Object* parent, int r, int g, int b)
{
	IV_ASSERT(parent != NULL);

	Evas_Object *bg = elm_bg_add(parent);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//elm_win_resize_object_add(parent, bg);

	elm_bg_color_set(bg, r,  g, b);

	evas_object_show(bg);

	return bg;
}
#if 0 //Chandan
Evas_Object *
ivug_layout_add(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);

	Evas_Object *layout;

	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_IVUG_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_IVUG_ERROR("edj loading fail, filepath=%s Group=%s", edj, group);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	ivug_layout_attach_to_window(parent, layout);

	return layout;
}
#endif
Evas_Object*
ivug_default_layout_add(Evas_Object *win)
{
/*
	Default layout.

		"elm.swallow.bg"
		"elm.swallow.content"
		"elm.swallow.controlbar"
*/
	IV_ASSERT(win != NULL);

	Evas_Object *layout;
	layout = elm_layout_add(win);

	if (layout == NULL) {
		MSG_SETAS_ERROR("Cannot create layout");
		return NULL;
	}

	Eina_Bool ret = EINA_FALSE;

	const char *profile = elm_config_profile_get();
	MSG_IVUG_HIGH("profile = %s", profile);
	if (!strcmp(profile, "mobile")) {
		ret = elm_layout_theme_set(layout, "layout", "application", "noindicator");
		MSG_IVUG_HIGH("layout/application/noindicator");
	} else if (!strcmp(profile, "desktop")) {
		ret = elm_layout_theme_set(layout, "layout", "application", "noindicator");
		MSG_IVUG_HIGH("layout/application/noindicator");
	} else {
		ret = elm_layout_theme_set(layout, "layout", "application", "noindicator");
		MSG_IVUG_HIGH("layout/application/noindicator");
	}

	if (ret == EINA_FALSE) {
		MSG_IVUG_ERROR("theme set fail");
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	return layout;
}

Evas_Object *
ivug_layout_add2(Evas_Object *parent, const char *edj, const char *group)
{
	IV_ASSERT(parent != NULL);

	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_IVUG_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_IVUG_ERROR("edj loading fail, filepath = %s Group = %s", edj, group);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(layout);
	return layout;
}

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

Evas_Object *ivug_icon_add(Evas_Object *parent, const char *edjname, const char *groupname)
{
	Evas_Object *icon;

	icon = elm_icon_add(parent);

	if (elm_image_file_set(icon, edjname, groupname) == EINA_FALSE) {
		MSG_IVUG_ERROR("Cannot file set. EDJ=%s Group=%s", edjname, groupname);
		evas_object_del(icon);
		return NULL;
	}

	evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(icon, 1, 1);
	evas_object_size_hint_expand_set(icon, 1, 1);

	return icon;
}

Evas_Object *ivug_toolbar_add(Evas_Object *parent, const char *style)
{
	Evas_Object *toolbar = elm_toolbar_add(parent);
	if (!toolbar) {
		MSG_IVUG_ERROR("tool bar create failed");
		return NULL;
	}
	if (style)
		elm_object_style_set(toolbar, style);
	elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
	elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
	elm_toolbar_select_mode_set(toolbar, ELM_OBJECT_SELECT_MODE_NONE);

	return toolbar;
}

Elm_Object_Item *ivug_toolbar_item_append(Evas_Object *toolbar, const char *text_id, Evas_Smart_Cb callback, void *data)
{
	Elm_Object_Item *item = NULL;

	char *domain = NULL;

	if (strstr(text_id, "IDS_COM"))
		domain = strdup("sys_string");
	else
		domain = strdup(IVUG_TEXT_DOMAIN);

	item = elm_toolbar_item_append(toolbar, NULL, text_id, callback, data);
	elm_object_item_domain_translatable_text_set(item, domain, text_id);

	free(domain);

	return item;
}

void _ivug_set_indicator_overlap_mode(const char *func, int line, bool bOverlap)
{
	Evas_Object *conform = (Evas_Object *)ug_get_conformant();
	IV_ASSERT(conform != NULL);

	if (bOverlap == true) {
		MSG_IVUG_HIGH("[%s:%4d] Set Overlap Mode! conform(0x%08x)", func, line, conform);
		elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_TRUE);
	} else {
		MSG_IVUG_HIGH("[%s:%4d] Set Non-Overlap Mode! conform(0x%08x)", func, line, conform);
		elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_FALSE);
	}
}


void _ivug_set_indicator_visibility(const char *func, int line,Evas_Object *win, bool bShow)
{
	IV_ASSERT(win != NULL);

	if (bShow == true) {
		MSG_IVUG_HIGH("[%s:%4d] Set Indicator Visible! win(0x%08x)", func, line, win);
		elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	} else {
		MSG_IVUG_HIGH("[%s:%4d] Set Indicator Hidden! win(0x%08x)", func, line, win);
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);
	}
}

