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

#pragma once

// Slide Show

/*
	Start slide show.

	if bSlideFirst == EINA_TRUE, slide and then wait interval time
	if bSlideFirst == EINA_FALSE, wait interval time and then slide
*/
#include <Elementary.h>
#include "ivug-define.h"
#include "ivug-medialist.h"
#include "ivug-config.h"

typedef enum {
	SLIDE_SHOW_STOPPED = 0x00,
	SLIDE_SHOW_INTERRUPTED,
	SLIDE_SHOW_RUNNING,
	SLIDE_SHOW_PAUSE,
} slideshow_state_t;

/*
Signals
	"slideshow,finished" with slideshow_state_t
*/

typedef struct _SlideShow SlideShow;

#ifdef __cplusplus
extern "C" {
#endif

SlideShow *
ivug_ss_create(Evas_Object *parent);

bool
ivug_ss_start(SlideShow *pSlideShow , Media_Item *current, Media_List *list, Eina_Bool bSlideFirst);

bool
ivug_ss_resume(SlideShow *pSlideShow);

Evas_Object *
ivug_list_popoup_show(const char *title, void *data);

bool
ivug_ss_pause(SlideShow *pSlideShow);

bool
ivug_ss_stop(SlideShow *pSlideShow);

void
ivug_ss_delete(SlideShow *pSlideShow);

void
ivug_ss_resize(SlideShow *pSlideShow);

Media_Item *
ivug_ss_item_get(SlideShow *pSlideShow);

Evas_Object *
ivug_ss_object_get(SlideShow *pSlideShow);

void
ivug_ss_set_stop(SlideShow *pSlideShow);		// Remove this!!!

int _ivug_ss_get_sort(int *val);
#ifdef __cplusplus
}
#endif

