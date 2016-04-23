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


#ifndef __IVUG_CALLBACK_H__
#define __IVUG_CALLBACK_H__

typedef void (*callback_func_t) (void *data1, void *data2, void *data3, void *user_data);

typedef struct {
	callback_func_t CBFunc;
	void *data;
}callback_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

callback_handle_t * ivug_callback_register(void);

void ivug_callback_set_callback(callback_handle_t *handle, callback_func_t callback, void *data);

void ivug_callback_call(callback_handle_t *handle, void *data1, void *data2, void *data3);

void ivug_callback_unregister(callback_handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif				// __IVUG_CALLBACK_H__
//! End of a file
