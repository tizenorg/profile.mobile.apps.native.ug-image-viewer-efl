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

#ifndef __IVUG_LANGUAGE_MGR_H
#define __IVUG_LANGUAGE_MGR_H

#include <Elementary.h>

typedef enum
{
	OBJ_TYPE_ELM_OBJECT, 		//elm_object_text_set(obj, text)
	OBJ_TYPE_EDJE_OBJECT, 	//edje_object_part_text_set(obj, part, text)
	OBJ_TYPE_MAX,
}obj_type;

typedef void *language_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

int ivug_language_mgr_create(language_handle_t *handle);
int ivug_language_mgr_destroy(language_handle_t handle);

/*part and string_id must be static*/
void ivug_language_mgr_register_object(language_handle_t handle, Evas_Object *obj, obj_type type, const char *part, const char *string_id);
void ivug_language_mgr_register_object_item(language_handle_t handle, Elm_Object_Item *object_item, const char *text_ID);
void ivug_language_mgr_unregister_object_item(language_handle_t handle, Elm_Object_Item *object_item);
void ivug_language_mgr_object_item_text_ID_set(language_handle_t handle, Elm_Object_Item *object_item, const char *text_ID);


void ivug_language_mgr_register_genlist_item(language_handle_t handle, Elm_Object_Item *item);
void ivug_language_mgr_unregister_genlist_item(language_handle_t handle, Elm_Object_Item *item);

void ivug_language_mgr_register_gengrid_item(language_handle_t handle, Elm_Object_Item *item);
void ivug_language_mgr_unregister_gengrid_item(language_handle_t handle, Elm_Object_Item *item);

void ivug_language_mgr_update(language_handle_t handle);

void ivug_elm_object_text_set(language_handle_t handle, Evas_Object *obj, const char *text_id);
void ivug_elm_object_item_text_set(language_handle_t handle, Elm_Object_Item *item, const char *text_id);
void ivug_elm_object_part_text_set(language_handle_t handle, Evas_Object *obj, const char *part, const char *text_id);
void ivug_edje_object_part_text_set(language_handle_t handle, Evas_Object *obj, const char *part, const char *text_id);
char * ivug_language_mgr_get_text_domain(const char *ID);

char * GET_STR(const char *ID);

#ifdef __cplusplus
}
#endif

#endif	//__IVUG_LANGUAGE_MGR_H

