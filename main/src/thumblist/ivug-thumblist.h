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

#ifndef __IVUG_THUMBLIST_H__
#define __IVUG_THUMBLIST_H__

typedef void Image_Object;

typedef struct {
	int count;
	void *item_data;
} Thumblist_Block_Index;


typedef enum {
	THUMBLIST_NONE,
	THUMBLIST_BESTPIC,
	THUMBLIST_SOUNDPIC,
	THUMBLIST_PANORAMA,
	THUMBLIST_BURSTSHOT,
} Thumblist_Metaphore;

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *
ivug_thumblist_add(Evas_Object* parent);

/*
	If bestpic mode is enabled, you can change bestpic status by long press.

	"set,bestpic"
	"unset,bestpic"

	Default is FALSE;
*/
void
ivug_thumblist_set_bestpic_mode(Evas_Object *obj, Eina_Bool bEditMode);

void
ivug_thumblist_set_edit_mode(Evas_Object *obj, Eina_Bool bEditMode);

Eina_Bool ivug_thumblist_get_edit_mode(Evas_Object *obj);

/*
	Signals.

	"item,selected"
		changed center image.
		pointer event_info indicates Image_Object *

	"need,next"
	"need,prev"

	"unloaded"

	"set,bestpic"
	"unset,bestpic"

	"changed,mode,edit"			// Thumblist mode changes to Edit
*/

// Item list
Image_Object *
ivug_thumblist_append_item(Evas_Object *obj, const char *thumbnail_path, bool bVideo, void *item_data);

Image_Object *
ivug_thumblist_prepend_item(Evas_Object *obj, const char *thumbnail_path, bool bVideo, void *item_data);

void
ivug_thumblist_delete_item(Evas_Object* obj, Image_Object *img);

Eina_Bool
ivug_thumblist_update_item(Evas_Object* obj, Image_Object *img, const char *thumbnail_path, bool bVideo, void *item_data);

void
ivug_thumblist_clear_items(Evas_Object* obj);

void *
ivug_thumblist_get_item_data(Evas_Object* obj, Image_Object *img);

Image_Object *
ivug_thumblist_find_item_by_data(Evas_Object* obj, void *item_data);

unsigned int ivug_thumblist_items_count(Evas_Object *obj);

// Center
void
ivug_thumblist_select_item(Evas_Object* obj, Image_Object *img);


Image_Object *
ivug_thumblist_get_selected_item(Evas_Object* obj);

Eina_List/* Image_Object */ *
ivug_thumblist_get_items_bestpic(Evas_Object* obj);

Eina_List/* Image_Object */ *
ivug_thumblist_get_items_checked(Evas_Object* obj);

Eina_Bool
ivug_thumblist_checked_item_is(Evas_Object* obj);

Thumblist_Metaphore
ivug_thumblist_get_item_mode(Evas_Object* obj, Image_Object *img);

void
ivug_thumblist_set_item_mode(Evas_Object* obj, Image_Object *img, Thumblist_Metaphore mode);



#ifdef __cplusplus
}
#endif



#endif		// __IVUG_THUMBLIST_H__

