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

#ifndef __IVUG_COMMENT_H__
#define __IVUG_COMMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void * Handle ;

Handle ivug_comment_loadfile(const char *fname);

void ivug_comment_set(Handle handle, const char *comment);

const char *ivug_comment_get(Handle handle);

void ivug_comment_savefile(Handle handle, const char *fname);

void ivug_comment_closefile(Handle handle);

#ifdef __cplusplus
}
#endif

#endif		// __IVUG_COMMENT_H__

