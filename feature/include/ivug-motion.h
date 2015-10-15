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

#ifndef __IVUG_MOTION_H__
#define __IVUG_MOTION_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	IV_MOTION_TILT,
	IV_MOTION_PANNING,
	IV_MOTION_MAX
}motion_type_e;

typedef void *motion_handle_t;

typedef void (*motion_callback_t) (motion_handle_t handle, int dx, int dy, void *data);

motion_handle_t ivug_motion_register_sensor(motion_type_e type, motion_callback_t cb_func, void *data);
void ivug_motion_unregister_sensor(motion_type_e type, motion_handle_t);

#ifdef __cplusplus
}
#endif

#endif				// __IVUG_MOTION_H__
//! End of a file
