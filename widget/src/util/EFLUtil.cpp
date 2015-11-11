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

#include <Ecore.h>

#include "EFLUtil.h"
#include "ivug-debug.h"

#include "ivug-language-mgr.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH
#undef LOG_CAT
#define LOG_CAT "IV-UTIL"

namespace EFL
{

extern "C" const char *
elm_widget_type_get(const Evas_Object *obj);

extern "C" EAPI Eina_Bool
elm_widget_type_check(const Evas_Object *obj,
                      const char *type,
                      const char *func);



void get_evas_geometry(Evas_Object *obj, const char *prefix)
{
	int x, y, w, h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_HIGH("%s XYWH(%d,%d,%d,%d)", prefix);
}

Evas_Object *create_gesture(Evas_Object *parent)
{
	Evas_Object *gesture;

	gesture = elm_gesture_layer_add(parent);

	return gesture;
}

Evas_Object *create_rect(Evas_Object *parent, int r, int g, int b, int a)
{
	Evas_Object *obj;

	obj = evas_object_rectangle_add(evas_object_evas_get(parent));

	evas_object_size_hint_expand_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_color_set(obj, r, g, b, a);

	return obj;
}

Evas_Object *create_clipper(Evas_Object *parent)
{
	Evas_Object *obj;

	obj = evas_object_rectangle_add(evas_object_evas_get(parent));

	evas_object_size_hint_expand_set(obj, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(obj, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_color_set(obj, 255, 255, 255, 255);

	return obj;
}


Evas_Object *create_layout(Evas_Object *parent, const char *edj, const char *group)
{
	MSG_ASSERT(parent != NULL);

	Evas_Object *layout;
	layout = elm_layout_add(parent);

	if (layout == NULL) {
		MSG_ERROR("Cannot create layout");
		return NULL;
	}

	if (elm_layout_file_set(layout, edj, group) == EINA_FALSE) {
		MSG_ERROR("Layout file set failed! %s in %s", group, edj);
		evas_object_del(layout);
		return NULL;
	}

	evas_object_size_hint_expand_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

//	evas_object_show(layout);
	return layout;
}

Evas_Object *create_button(Evas_Object *parent, const char *style, const char *icon, const char *caption_id)
{
	Evas_Object *btn = NULL;

	btn = elm_button_add(parent);
	elm_object_style_set(btn, style);

	if (icon) {
		Evas_Object *ic = NULL;
		ic = elm_icon_add(btn);
		elm_image_file_set(ic, icon, NULL);
		evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
		elm_object_part_content_set(btn, "icon", ic);
	}

	if (caption_id) {
		elm_object_text_set(btn, caption_id);
	}

	return btn;
}


Evas_Object *create_icon(Evas_Object *parent, const char *edjname, const char *groupname)
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


__attribute__((used)) void dump_win(Evas_Object *obj)
{

	if (elm_widget_type_check(obj, "elm_win", __func__) == false) {
		MSG_ERROR("Obj(0x%08x) is not elm_win Object", obj);
		return;
	}

	void *pData = evas_object_smart_data_get(obj);

// A : ((Elm_Win_Smart_Data *)((Evas_Object_Smart *)(((Evas_Object *)ug_get_window())->object_data))->data)
// B : &(((Elm_Win_Smart_Data *)((Evas_Object_Smart *)(((Evas_Object *)ug_get_window())->object_data))->data)->resize_objs)

	volatile int Diff;

	Diff = 0xC4;

	Eina_List **ppList = (Eina_List **)((char *)pData + ((Diff)) /* B - A */);
	Eina_List *subobjs = *ppList;

// (gdb) set EFL::dump_win((Evas_Object *)ug_get_window())
	MSG_HIGH("pData=0x%08x SubObj=0x%08x pData+C4=0x%08x SubObjCnt=%d", pData, subobjs, reinterpret_cast<unsigned int>(pData) + (Diff), eina_list_count(subobjs));

	{
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		MSG_HIGH("Win=%s(%s,0x%08x) %s(%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), elm_widget_type_get(obj), obj, evas_object_type_get(obj), x, y, w, h, pass, repeat, visible, propagate);
	}

	const Eina_List *l;
	Evas_Object *child;

	void *MyData = NULL;

	EINA_LIST_FOREACH(subobjs, l, MyData) {
		child = (Evas_Object *)MyData;

		dump_obj(child, 0);
	}

}

__attribute__((used)) void dump_obj(Evas_Object *obj, int lvl)
{
	Eina_List *list = evas_object_smart_members_get(obj);

	if (lvl == 0) {
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(obj, &mW, &mH);
		evas_object_size_hint_max_get(obj, &MW, &MH);

		MSG_HIGH("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);
		lvl++;
	}

	Evas_Object *data;
	Eina_List *l;

	for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l)) {
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(data, &mW, &mH);
		evas_object_size_hint_max_get(data, &MW, &MH);

		char *space = new char[lvl * 2 + 1];

		for (int i = 0; i < lvl * 2; i++) {
			space[i] = ' ';
		}

		space[lvl * 2] = '\0';

		MSG_HIGH("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", space, evas_object_name_get(data), evas_object_type_get(data), data, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);

		delete[] space;

		dump_obj(data, lvl + 1);

	}
}



void dump_widget(Evas_Object *obj, int lvl)
{
	Eina_List *list = evas_object_smart_members_get(obj);

	if (lvl == 0) {
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		MSG_SEC("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), elm_widget_type_get(obj), obj, x, y, w, h, pass, repeat, visible, propagate);
		lvl++;
	}

	Evas_Object *data;
	Eina_List *l;

	for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l)) {
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		if (elm_widget_type_get(data) != NULL || evas_object_name_get(data) != NULL) {
			char *space = new char[lvl * 2 + 1];

			for (int i = 0; i < lvl * 2; i++) {
				space[i] = ' ';
			}

			space[lvl * 2] = '\0';

			MSG_SEC("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", space, evas_object_name_get(data), elm_widget_type_get(data), data, x, y, w, h, pass, repeat, visible, propagate);

			delete[] space;
		}

		dump_widget(data, lvl + 1);

	}
}


void dump_clipper(Evas_Object *clipper)
{

	const Eina_List *clippees;
	const Eina_List *l;

	Evas_Object *data;

	data = clipper;
	int x, y, w, h;

	evas_object_geometry_get(data, &x, &y, &w, &h);
	Eina_Bool repeat = evas_object_repeat_events_get(data);
	Eina_Bool pass = evas_object_pass_events_get(data);
	Eina_Bool visible = evas_object_visible_get(data);
	Eina_Bool propagate = evas_object_propagate_events_get(data);

	MSG_SEC("Clipper Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(data), evas_object_type_get(data), data,  x, y, w, h, pass, repeat, visible, propagate);

	clippees = evas_object_clipees_get(clipper);
	void *tmp;
	EINA_LIST_FOREACH(clippees, l, tmp) {
		data = (Evas_Object *)tmp;
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		MSG_SEC("      Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(data), evas_object_type_get(data), data,  x, y, w, h, pass, repeat, visible, propagate);
	}
}

char * current_render_method(Evas *e)
{
	char *szRet = NULL;

	//get render list that usable
	Eina_List *engine_list = evas_render_method_list();

	//id of rendering method
	int current_id = evas_output_method_get(e);

	//check rendering method same
	Eina_List *l;

	const char *engine_name;

	for (l = engine_list, engine_name = (const char *)eina_list_data_get(l); l; l = eina_list_next(l), engine_name = (const char *)eina_list_data_get(l)) {
		//id value of specific rendering method
		int engine_id = evas_render_method_lookup(engine_name);

		//"software_x11", "gl_x11"
		if (engine_id == current_id) {
			MSG_HIGH("current render method is %s!", engine_name);
			szRet = strdup(engine_name);
			break;
		}
	}

	evas_render_method_list_free(engine_list);

	return szRet;
}

Evas_Object *create_window(const char *title, int w, int h)
{
	Evas_Object *win;

	win = elm_win_add(NULL, "main", ELM_WIN_BASIC);
	elm_win_title_set(win, title);

	evas_object_resize(win, w, h);
	evas_object_show(win);

	return win;
}



};


namespace util
{

Image_Codec_Type
get_codec_type_with_size(const unsigned char *buffer, unsigned int size)
{
	if (size < 8) {
		MSG_ERROR("To small buffer size : %d", size);
		return eImageCodec_UNKNOWN;
	}

	if (buffer[0] == 0xff && buffer[1] == 0xd8) {	// JPEG
		return eImageCodec_JPEG;
	} else if (buffer[0] == 0x89 && buffer[1] == 0x50 && buffer[2] == 0x4e && buffer[3] == 0x47 &&
	           buffer[4] == 0x0d && buffer[5] == 0x0a && buffer[6] == 0x1a && buffer[7] == 0x0a) {
		return eImageCodec_PNG;
	} else {
		// ("GIF87a" or "GIF89a")

		if (memcmp(buffer, "GIF8", 4) == 0) {
			if (buffer[4] == '9' || buffer[4] == '7') {
				if (buffer[5] == 'a') {
					return eImageCodec_GIF;
				}
			}
		}
	}

	return eImageCodec_IMAGE;
}

Image_Codec_Type
get_codec_type(const char *filename)
{
	FILE *fp = NULL;

	fp = fopen(filename, "rb");

	if (fp == NULL) {
		char error_msg[256];
		MSG_SEC("Cannot find %s", filename);
		MSG_ERROR("Cannot open file : %s", strerror_r(errno, error_msg, sizeof(error_msg)));

		return eImageCodec_UNKNOWN;
	}

	unsigned char buffer[8];
	int readcnt = 0;

	readcnt = fread(&buffer, 1, sizeof(buffer), fp);

	if (readcnt != sizeof(buffer)) {
		if (feof(fp)) {
			MSG_ERROR("End Of File");
		}
		if (ferror(fp)) {
			perror("fread ERROR");
		}
		MSG_SEC("Read Error. file = %s, readcnt = %d", filename, readcnt);

		fclose(fp);
		return eImageCodec_UNKNOWN;
	}

	fclose(fp);

	MSG_LOW("ReadCount=%d", readcnt);
	return get_codec_type_with_size(buffer, readcnt);

}


bool is_openGL_enabled(Evas *e)
{
	MSG_ASSERT(e != NULL);

	Eina_List *engines, *l;
	int cur_id;
	int id;
	void *data;

	engines = evas_render_method_list();
	if (!engines) {
		MSG_ERROR("No engine is specified");
		return false;
	}

	cur_id = evas_output_method_get(e);

	EINA_LIST_FOREACH(engines, l, data) {
		const char *name = (char *)data;

		id = evas_render_method_lookup(name);
		if (name && id == cur_id) {
			if (!strcmp(name, "gl_x11")) {
				return true;
			}
			break;
		}
	}

	return false;

}


};


