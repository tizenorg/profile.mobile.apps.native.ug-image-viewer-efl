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

#ifndef __MESSAGE_H__
#define __ MESSAGE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void * MessageHandle;
typedef void (*FnMessage)(	int param1, int param2, int param3, void *param4, void *client_data);


bool remove_message_handle(MessageHandle handle);

MessageHandle create_message_handle();

bool send_message(MessageHandle handle, const char *command, int param1, int param2, int param3, void *param4);

#ifdef __cplusplus
}
#endif

#endif // __FMRADIO_MESSAGE_H__

