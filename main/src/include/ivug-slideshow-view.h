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

#ifndef __IVUG_SLIDESHOW_VIEWER_H__
#define __IVUG_SLIDESHOW_VIEWER_H__

#include "ivug-parameter.h"
#include "ivug-medialist.h"

#include "ivug-slideshow.h"


typedef struct {
	Evas_Object *parent;

	Evas_Object *layout;		// Not visibile layout.

	ivug_view_by view_by;

	bool bStandAlone;

// List
	Media_List *mList;

	Ecore_Event_Handler *keydown_handler;

// Slide show;
	Media_Item *ss_curItem;
	SlideShow *ssHandle;

	char *album_name;
} Ivug_SlideShowView;



#ifdef __cplusplus
extern "C" {
#endif

void
ivug_slideshow_view_destroy(Ivug_SlideShowView *pSSView);

void
_ivug_slideshow_view_on_mmc_state_changed(void *data);

#ifdef __cplusplus
}
#endif


#endif	//__IVUG_SLIDESHOW_VIEWER_H__

