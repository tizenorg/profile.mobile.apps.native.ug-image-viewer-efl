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

#ifndef __IVUG_ANIM_H__
#define __IVUG_ANIM_H__

#include <stdbool.h>

typedef enum {
	ANIM_TYPE_LINEAR,
	ANIM_TYPE_QUARTIC_INOUT,
	ANIM_TYPE_QUINTIC_INOUT,
	ANIM_TYPE_SINE,
} anim_type_t;

/*
	t: current time b: start time c: change in value d: duration
*/
typedef double (*anim_transit_func_t) (double t, double b, double c, double d);

typedef struct {
/* private */
	double duration;
	double t_start;
	double t_varing;

	anim_transit_func_t transit_func;

	void *user_data;
} anim_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

anim_handle_t *create_animation();

void delete_animation(anim_handle_t *anim);

bool update_animation(anim_handle_t *anim, double current, double *value);

bool set_animation_type(anim_handle_t *anim, anim_type_t type);

bool set_animation(anim_handle_t *anim, int start, int end, double duration);

#ifdef __cplusplus
}
#endif


#endif
