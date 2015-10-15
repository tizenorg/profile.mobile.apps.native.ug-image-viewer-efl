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

#ifndef __IVUG_VIDEO_H__
#define __IVUG_VIDEO_H__

#include <Evas.h>
#include "ivug-define.h"

#ifdef __cplusplus
extern "C" {
#endif


/*
	"started" when video stream start to play
*/
Evas_Object *ivug_video_create(Evas_Object *parent);

Evas_Load_Error ivug_video_file_set(Evas_Object *obj, const char *file);
Evas_Load_Error ivug_video_file_unset(Evas_Object *obj);

Eina_Bool ivug_video_play(Evas_Object *obj);
Eina_Bool ivug_video_stop(Evas_Object *obj);
Eina_Bool ivug_video_pause(Evas_Object *obj);
Eina_Bool ivug_video_resume(Evas_Object *obj);

void ivug_video_region_get(const Evas_Object *obj, int *x, int *y, int *w, int *h);

void ivug_video_hold_set(const Evas_Object *obj, Eina_Bool hold);	// If set HOLD, all events including mouse is ignored.

void ivug_video_zoom_reset(Evas_Object *obj, Evas_Point *pCenter);

Eina_Bool ivug_video_is_playing(Evas_Object *obj);


#ifdef __cplusplus
}
#endif




#endif		// __IVUG_VIDEO_H__

