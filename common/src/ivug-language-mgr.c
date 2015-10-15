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

#include "ivug-define.h"
#include "ivug-debug.h"
#include "ivug-util.h"
#include "ivug-db.h"

#include <libintl.h>

#include "ivug-language-mgr.h"
#include <glib.h>

#include <Elementary.h>

#define LOG_LVL DBG_MSG_LVL_HIGH
#define LOG_CAT "IV-LANGUAGE"

#define IVUG_TEXT_DOMAIN PACKAGE

typedef struct
{
	Evas_Object *obj;
	obj_type type;
	const char *text_id;
	const char *part;
	void *data;
}obj_data;

typedef struct
{
	Elm_Object_Item *obj_item;
	const char *text_id;
}obj_item_data;

typedef struct _lang_mgr *lang_mgr;
struct  _lang_mgr{
	GList *objs;
	GList *obj_items;
	GList *glist_items;
	GList *grid_items;
}_lang_mgr;

#define G_LIST_FOREACH(list, l, data) \
  for (l = list,                         \
       data = g_list_nth_data(l, 0);     \
       l;                                \
       l = g_list_next(l),            \
       data = g_list_nth_data(l, 0))


static void __obj_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	obj_data *item = data;
	lang_mgr mgr = (lang_mgr)item->data;
	ivug_ret_if(mgr == NULL);
	ivug_ret_if(item == NULL);

	MSG_MED("unregister obj : 0x%08x %s", item->obj, item->text_id);

	mgr->objs =
		g_list_delete_link(mgr->objs, g_list_find(mgr->objs, item));

	free(item);
}

static void __glist_free(void *data)
{
	obj_data *item = data;
	ivug_ret_if(item == NULL);
	if (item->obj)
	{
		evas_object_event_callback_del(item->obj, EVAS_CALLBACK_DEL, __obj_del_cb);
	}
	free(item);
}

static char *__get_text(const char *ID)
{
	ivug_retv_if(ID == NULL, NULL);
	char *str;

	if (strstr(ID, "IDS_COM"))
		str = dgettext("sys_string", ID);
	else
		str = dgettext(IVUG_TEXT_DOMAIN, ID);//gettext(ID);

	return str;
}

static void __update_obj(void *data, void *userdata)
{
	char *text;
	obj_data *item = data;
	ivug_ret_if(item == NULL);
	MSG_MED("handle: 0x%x, ID:%s",item->obj, item->text_id);

	text = __get_text(item->text_id);

	if (item->type == OBJ_TYPE_ELM_OBJECT)
	{
		if (item->part == NULL)
		{
			elm_object_text_set(item->obj, text);
		}
		else
		{
			elm_object_part_text_set(item->obj, item->part, text);
		}
	}
	else if (item->type == OBJ_TYPE_EDJE_OBJECT)
		edje_object_part_text_set(elm_layout_edje_get(item->obj), item->part, text);
	else
		MSG_WARN("Unhandled case");
}

static void __update_obj_item(void *data, void *userdata)
{
	char *text;
	obj_item_data *item_data = data;
	ivug_ret_if(item_data == NULL);

	MSG_MED("handle: 0x%x, ID:%s",item_data->obj_item, item_data->text_id);
	text = __get_text(item_data->text_id);
	elm_object_item_text_set(item_data->obj_item, text);
}

static void __update_list(void *data, void *userdata)
{
	Elm_Object_Item *item = data;
	ivug_ret_if(item == NULL);
	MSG_MED("handle: 0x%x",item);
	elm_genlist_item_update(item);
}

static void __update_grid(void *data, void *userdata)
{
	Elm_Object_Item *item = data;
	ivug_ret_if(item == NULL);
	MSG_MED("handle: 0x%x",item);
	elm_gengrid_item_update(item);
}

int ivug_language_mgr_create(language_handle_t *handle)
{
	MSG_HIGH("ivug_language_mgr_create 0x%x", *handle);

	lang_mgr mgr = calloc(1, sizeof(_lang_mgr));
	if (!mgr)
	{
		MSG_WARN("Error: calloc");
		return -1;
	}
	*handle = (language_handle_t)mgr;
	return 0;
}

int ivug_language_mgr_destroy(language_handle_t handle)
{
	lang_mgr mgr = (lang_mgr)handle;

	ivug_retv_if(mgr == NULL, -1);
	g_list_free_full(mgr->objs, __glist_free);
	mgr->objs = NULL;

	g_list_free(mgr->glist_items);
	mgr->glist_items = NULL;

	g_list_free(mgr->grid_items);
	mgr->grid_items = NULL;

	free(mgr);
	mgr = NULL;

	return 0;
}

void ivug_language_mgr_register_object(language_handle_t handle, Evas_Object *obj, obj_type type, const char *part, const char *text_id)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	obj_data *item = calloc(1, sizeof(obj_data));
	ivug_ret_if(item == NULL);

	MSG_MED("register obj : 0x%08x %s", obj, text_id);

	item->type = type;
	item->part = part;
	item->text_id = text_id;
	item->obj = obj;
	item->data = handle;

	evas_object_event_callback_add(obj, EVAS_CALLBACK_DEL, __obj_del_cb, item);

	mgr->objs = g_list_append(mgr->objs, item);
}

void ivug_language_mgr_register_object_item(language_handle_t handle, Elm_Object_Item *object_item, const char *text_ID)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	obj_item_data *data = calloc(1, sizeof(obj_item_data));
	ivug_ret_if(data == NULL);

	MSG_MED("register obj item : 0x%08x %s", object_item, text_ID);

	data->obj_item = object_item;
	data->text_id = text_ID;

	mgr->obj_items= g_list_append(mgr->obj_items, data);
}

void ivug_language_mgr_unregister_object_item(language_handle_t handle, Elm_Object_Item *object_item)
{
	lang_mgr mgr = (lang_mgr)handle;
	GList *l;
	obj_item_data *data;

	G_LIST_FOREACH(mgr->obj_items, l, data)
	{
		if (data && data->obj_item == object_item)
		{
			MSG_MED("register obj item : 0x%08x %s", data->obj_item, data->text_id);

			mgr->obj_items = g_list_delete_link(mgr->obj_items, l);
			if (data)
				free(data);
			break;
		}
	}
}

void ivug_language_mgr_object_item_text_ID_set(language_handle_t handle, Elm_Object_Item *object_item, const char *text_ID)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	GList *l;
	obj_item_data *data;

	G_LIST_FOREACH(mgr->obj_items, l, data)
	{
		if (data->obj_item == object_item)
		{
			MSG_MED("set ID: %s", text_ID);
			data->text_id = text_ID;
			break;
		}
	}
}

void ivug_language_mgr_register_genlist_item(language_handle_t handle, Elm_Object_Item *item)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	mgr->glist_items =
		g_list_append(mgr->glist_items, item);
}

void ivug_language_mgr_unregister_genlist_item(language_handle_t handle, Elm_Object_Item *item)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	mgr->glist_items =
		g_list_delete_link(mgr->glist_items, g_list_find(mgr->glist_items, item));
}

void ivug_language_mgr_register_gengrid_item(language_handle_t handle, Elm_Object_Item *item)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	mgr->grid_items =
		g_list_append(mgr->grid_items, item);
}

void ivug_language_mgr_unregister_gengrid_item(language_handle_t handle, Elm_Object_Item *item)
{
	lang_mgr mgr = (lang_mgr)handle;
	ivug_ret_if(mgr == NULL);
	mgr->grid_items =
		g_list_delete_link(mgr->grid_items, g_list_find(mgr->grid_items, item));
}

void ivug_language_mgr_update(language_handle_t handle)
{
	lang_mgr mgr = (lang_mgr)handle;
	MSG_HIGH("language changed. update text");
	ivug_ret_if(mgr == NULL);
	g_list_foreach(mgr->objs, __update_obj, NULL);
	g_list_foreach(mgr->obj_items, __update_obj_item, NULL);
	g_list_foreach(mgr->glist_items, __update_list, NULL);
	g_list_foreach(mgr->grid_items, __update_grid, NULL);
}

void ivug_elm_object_text_set(language_handle_t handle, Evas_Object *obj, const char *text_id)
{
	char *text = __get_text(text_id);
	elm_object_text_set(obj, text);

	ivug_language_mgr_register_object(handle, obj, OBJ_TYPE_ELM_OBJECT, NULL, text_id);
}

void ivug_elm_object_item_text_set(language_handle_t handle, Elm_Object_Item *item, const char *text_id)
{
	char *text = __get_text(text_id);
	elm_object_item_text_set(item, text);

	ivug_language_mgr_register_object_item(handle, item, text_id);
}

void ivug_elm_object_part_text_set(language_handle_t handle, Evas_Object *obj, const char *part, const char *text_id)
{
	char *text = __get_text(text_id);
	elm_object_part_text_set(obj, part, text);

	ivug_language_mgr_register_object(handle, obj, OBJ_TYPE_ELM_OBJECT, part, text_id);
}

void ivug_edje_object_part_text_set(language_handle_t handle, Evas_Object *obj, const char *part, const char *text_id)
{
	char *text = __get_text(text_id);
	edje_object_part_text_set(elm_layout_edje_get(obj), part, text);

	ivug_language_mgr_register_object(handle, obj, OBJ_TYPE_EDJE_OBJECT, part, text_id);
}

char * GET_STR(const char *ID)
{
	return __get_text(ID);
}

char * ivug_language_mgr_get_text_domain(const char *ID)
{
	ivug_retv_if(ID == NULL, NULL);

	if (strstr(ID, "IDS_COM"))
		return "sys_string";
	else
		return IVUG_TEXT_DOMAIN;
}

