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

#ifndef __IVUG_NAME_VIEW_H__
#define __IVUG_NAME_VIEW_H__

#include "ivug-common.h"
#include "ivug-vibration.h"

typedef enum {
	NAME_VIEW_RESPONSE_OK,
	NAME_VIEW_RESPONSE_CANCEL,
} ivug_name_response;


typedef enum {
	NAME_VIEW_MODE_SINGLE_LINE,
	NAME_VIEW_MODE_MULTI_LINE,
} ivug_name_mode;

typedef struct _Ivug_NameView Ivug_NameView;


typedef void (*FNResponse)(Ivug_NameView *pView, ivug_name_response resp, const char *string, void *pClientData);

#ifdef __cplusplus
extern "C" {
#endif

Ivug_NameView *
ivug_name_view_create(Evas_Object *parent, ivug_name_mode mode);


void
ivug_name_view_set_title(Ivug_NameView *pNameView, const char *title);

/*
	Set initial string for editfield
*/
void
ivug_name_view_set_text(Ivug_NameView *pNameView, const char *str);

void
ivug_name_view_set_response_callback(Ivug_NameView *pNameView, FNResponse resp, void *data);

void
ivug_name_view_destroy(Ivug_NameView *pNameView);

Evas_Object *
ivug_name_view_object_get(Ivug_NameView *pNameView);

void
ivug_name_view_set_focus(Ivug_NameView *pNameView);

void
ivug_name_view_set_max_length(Ivug_NameView *pNameView, int max_len);

void
ivug_name_view_set_guide_text(Ivug_NameView *pNameView, const char *text_id);

void
ivug_name_view_set_filter_text(Ivug_NameView *pNameView, const char *filter_text);


/*
	Default is FALSE
*/
void
ivug_name_view_allow_null_set(Ivug_NameView *pNameView, Eina_Bool bNullAllowed);

void
ivug_name_view_show_imf(Ivug_NameView *pNameView);

#ifdef __cplusplus
}
#endif

#endif	// __IVUG_NAME_VIEW_H__


