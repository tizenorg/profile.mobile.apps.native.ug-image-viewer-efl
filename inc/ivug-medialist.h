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

#ifndef __IVUG_MEDIALIST_H__
#define __IVUG_MEDIALIST_H__

#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-media.h"

#include "ivug-filter.h"

//typedef void *Media_Item;
//typedef void Media_List;
typedef struct _Media_Data Media_Data;

typedef bool (*ivug_medialist_cb)(Media_List *mList, void *data);

#ifdef __cplusplus
extern "C" {
#endif

Media_List * ivug_medialist_create();

bool ivug_medialist_set_callback(Media_List *mList, ivug_medialist_cb callback, void *data);

Media_Item *ivug_medialist_load(Media_List *mList, Filter_struct *filter);
Media_Item *ivug_medialist_reload(Media_List *mList, Media_Item *current);

void ivug_medialist_del(Media_List *mList);

/*
	return total count
*/
int ivug_medialist_get_count(Media_List *mList);

/*
	Needed for displaying title. ex) Index/Total Count
*/
int ivug_medialist_get_index(Media_List *mList, Media_Item *item);


Media_Item *ivug_medialist_get_first(Media_List *mList);
Media_Item *ivug_medialist_get_last(Media_List *mList);

Media_Item *ivug_medialist_get_next(Media_List *mList, Media_Item *item);
Media_Item *ivug_medialist_get_prev(Media_List *mList, Media_Item *item);

void ivug_medialist_delete_item(Media_List *mList, Media_Item *item, bool deleteItem);

Media_Data *ivug_medialist_get_data(const Media_Item *item);

Media_Item *ivug_medialist_get_random_item(Media_List *mList);
Media_Item *ivug_medialist_get_shuffle_item(Media_List *mList, Media_Item *item);


Media_Item *ivug_medialist_find_item_by_index(Media_List *mList, int index);
Media_Item *ivug_medialist_find_item_by_filename(Media_List *mList, const char* filepath);
Media_Item *ivug_medialist_find_item_by_uuid(Media_List *mList, UUID uuid);

Media_Item *ivug_medialist_append_item(Media_List *mList, const char *filepath);
Media_Item *ivug_medialist_prepend_item(Media_List *mList, const char *filepath);

void ivug_medialist_set_update_callback(Media_List *mList);
void ivug_medialist_del_update_callback(Media_List *mList);

bool ivug_medialist_set_current_item(Media_List *mList, Media_Item *mitem);

Media_Item * ivug_medialist_get_current_item(Media_List *mList);

/*
	Returns previously selected item
*/
Media_Item * ivug_medialist_get_prev_item(Media_List *mList);

bool ivug_medialist_need_update(Media_List *mList);

void ivug_medialist_set_update_flag(Media_List *mList, bool flag);

void ivug_media_list_free(Media_List *mList);

Eina_List *ivug_medialist_get_burst_item_list(Media_List *mList, const char *burst_id);

void ivug_medialist_del_burst_item_list(Eina_List *list);

/* Return filter must be freed */
Filter_struct * ivug_medialist_get_filter(Media_List *mList);

#ifdef __cplusplus
}
#endif

#endif //__IVUG_MEDIALIST_H__

