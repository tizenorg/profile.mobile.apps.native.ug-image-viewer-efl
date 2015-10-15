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

#ifndef __IVUG_VIBRATION_H__
#define __IVUG_VIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define INVALID_HAPTIC_HANDLE (NULL)
#define VIBRATION_DURATION (500) 		// 500 ms

typedef void * vibration_h;

/*
	If success, returns handle. otherwise NULL.
*/
vibration_h ivug_vibration_create(void);

/*
	duration in ms
*/
bool ivug_vibration_play(vibration_h handle, int duration);

bool ivug_vibration_stop(vibration_h handle);

bool ivug_vibration_delete(vibration_h handle);


#ifdef __cplusplus
}
#endif

#endif // __IVUG_VIBRATION_H__

