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

#ifndef __IVUG_DATA_LIST_H__
#define __IVUG_DATA_LIST_H__

#include "ivug-define.h"
#include "ivug-medialist.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
	stp/endp is botn -1 when want to retrieve all items.
*/

Eina_List *ivug_list_load_DB_items(const Filter_struct *filter, int stp, int endp);

Eina_List *ivug_list_load_DB_items_list(const Filter_struct *filter, Eina_List *list);

int ivug_list_get_item_cnt(const Filter_struct *filter);

int ivug_list_get_burst_item_cnt(const Filter_struct *filter, const char * burst_id);

void ivug_list_delete_items(Eina_List *items);

int ivug_list_get_dir_cnt(const char *basedir);

Eina_List *ivug_list_load_dir_items(const char *basedir);

Eina_List *ivug_list_append_item(Eina_List *list, const char *filepath);

Eina_List *ivug_list_prepend_item(Eina_List *list, const char *filepath);

Eina_List *
ivug_list_load_burst_items(const Filter_struct *filter, const char *burst_id, int stp, int endp);

Eina_List *
ivug_list_load_file_list(const Filter_struct *filter, Eina_List *list);

#ifdef __cplusplus
}
#endif

#endif


