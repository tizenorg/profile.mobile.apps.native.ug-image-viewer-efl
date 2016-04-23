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


#ifndef __IVUG_CONTEXT_H__
#define __IVUG_CONTEXT_H__

#include "ivug-define.h"

#include "ivug-callback.h"

#include "ivug-language-mgr.h"
#include "ivug-common.h"
#include "ivug-util.h"
#include "ivug-widget.h"

#include "ivug-db.h"

#include "ivug-language-mgr.h"

/*
	Managing context(instance)
*/
#ifdef __cplusplus
extern "C" {
#endif

bool
ivug_context_deinit();

bool
ivug_context_init(Evas_Object *win, Evas_Object *conform);

void
ivug_context_destroy_me(const char *file, int line);

#define DESTROY_ME() \
	do { \
		ivug_context_destroy_me(__FILE__, __LINE__); \
	} while(0)



void
gSetRotationDegree(int degree);

Elm_Theme*
gGetSystemTheme(void);

int
gGetRotationDegree(void);

#if 0//Chandan
ui_gadget_h
gGetUGHandle(void);
#endif
Evas_Object *
gGetCurrentWindow(void);

callback_handle_t *
gGetCallbackHandle(void);

language_handle_t
gGetLanguageHandle(void);

int
gGetScreenWidth();

int
gGetScreenHeight();

void gSetAlbumIndex(const char* val);
const char* gGetAlbumIndex();

app_control_h
gGetServiceHandle(void);

void
gSetServiceHandle(app_control_h service);

bool gGetDestroying();

void gSetDestroying(bool isDestroying);
#if 0//Chandan
Evas_Object *gGetParentLayout();
#endif
#ifdef __cplusplus
}
#endif

#endif //__IVUG_CONTEXT_H__
