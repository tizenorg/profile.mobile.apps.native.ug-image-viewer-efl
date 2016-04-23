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

#include "ivug-debug.h"
#include "ivug-callback.h"

#include <stdlib.h>
//callback_func_t g_callback;

callback_handle_t * ivug_callback_register()
{
	callback_handle_t *callback_handle = calloc(1, sizeof(callback_handle_t));
	return callback_handle;
}

void ivug_callback_set_callback(callback_handle_t *handle, callback_func_t callback, void *data)
{
	IV_ASSERT(handle != NULL);
	handle->CBFunc = callback;
	handle->data = data;
}

void ivug_callback_call(callback_handle_t *handle, void *data1, void *data2, void *data3)
{
	IV_ASSERT(handle != NULL);
	if (handle->CBFunc)
	{
		(handle->CBFunc)(data1, data2, data3, handle->data);
	}
}

void ivug_callback_unregister(callback_handle_t *handle)
{
	IV_ASSERT(handle != NULL);
	free(handle);
}

