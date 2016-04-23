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

#ifndef __IVUG_POPUP_H__
#define __IVUG_POPUP_H__

#include "ivug-common.h"
//#include "ivug-widgets.h"

typedef enum _Popup_Response
{
	POPUP_RESPONSE_NONE = -1,
	POPUP_RESPONSE_TIMEOUT = -2,
	POPUP_RESPONSE_OK = -3,
	POPUP_RESPONSE_CANCEL = -4,
}Popup_Response;

typedef enum _Longpress_Popup_Response
{
	LPPOPUP_RESPONSE_NONE = 0,
	LPPOPUP_RESPONSE_COPY,
	LPPOPUP_RESPONSE_MANUALLY_DETECT,
}Longpress_Popup_Response;

// Selected data

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object*
ivug_popup_add(Evas_Object* parent, const char* title_id);

/*
	Create & show popup with 2 button (Yes/No)
*/
Evas_Object* ivug_2btn_popup_show(Evas_Object* parent, const char* title,
	const char* contents, Evas_Smart_Cb response, void* user_data);

/*
	Create & show popup with no button.
	TODO : Rename function
*/
Evas_Object *ivug_timeout_popup_show(Evas_Object *parent,
	Evas_Smart_Cb response, void *data, const char *sztitle, const char *content);


Evas_Object*
ivug_deletepopup_show(Evas_Object* parent, const char* title,
					const char* contents, Evas_Smart_Cb response, void* user_data);


/*
	Create show copy popup in coordinate (x,y)
*/
Evas_Object* ivug_longpress_popup_show(Evas_Object *parent, int x, int y, bool bUseExtMenu, Evas_Smart_Cb response, void *data);

Evas_Object* ivug_deletepopup_show(Evas_Object* parent, const char* title,
		const char* contents, Evas_Smart_Cb response, void* user_data);

Evas_Object *ivug_rename_popup_show(Evas_Object *parent, const char *filename, Evas_Smart_Cb response, void *data);

Evas_Object *ivug_radio_popoup_show(Evas_Object *parent, const char *title,
				int selected_index, Eina_List *name_list, Evas_Smart_Cb response, void *data);

Evas_Object *ivug_progress_popup_show(Evas_Object *parent, char *title_id,
				       Evas_Smart_Cb response, void *data);

Evas_Object *ivug_tag_buddy_access_popup_show(Evas_Object *parent, Evas_Smart_Cb check_response,
					Evas_Smart_Cb response, void *data);

Evas_Object *ivug_nearby_popup_show(Evas_Object *parent, Evas_Smart_Cb check_response, bool state,
					Evas_Smart_Cb response, void *data);

Evas_Object *ivug_processing_popup_show(Evas_Object *parent);

#ifdef __cplusplus
}
#endif


#endif //__IVUG_POPUP_H__
