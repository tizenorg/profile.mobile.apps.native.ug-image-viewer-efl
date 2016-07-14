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

#ifndef __IVUG_EXT_UG_H__
#define __IVUG_EXT_UG_H__

#include <stdbool.h>
#include <notification.h>
#include "ivug-define.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPERATION_NAME_CALL "http://tizen.org/appcontrol/operation/call"

/*
	Launch extern module.
*/
bool ivug_ext_launch_videoplayer_simple(const char *filepath);
bool ivug_ext_launch_videoplayer(const char *uri, bool isLockScreen);

#ifdef USE_EXT_SLIDESHOW
ui_gadget_h  ivug_ext_launch_select_music(ug_result_cb result_func, ug_destroy_cb destroy_func, void *data);
#endif

bool ivug_ext_launch_videoeditor(const char *uri, app_control_reply_cb callback, void *data);

bool ivug_ext_launch_browser(const char *uri);

bool ivug_ext_launch_facebook(const char *uri);

bool ivug_ext_launch_print(const char *uri);

#ifdef IV_EXTENDED_FEATURES
app_control_h ivug_ext_launch_image_editor(const char *uri, app_control_reply_cb callback, void *data);

bool ivug_ext_launch_default(const char *uri, const char *operation, const char *pkg, const char *mime, void *data);
#endif


/*
	Actually, each sns pkg needs different parameter. so hard-coding cannot be removed.
*/
#ifdef IV_EXTENDED_FEATURES
bool ivug_ext_launch_sns(const char *pkgname, const char *uri);
#endif

#ifdef IV_EXTENDED_FEATURES
bool ivug_ext_launch_email(const char *uri);
#endif

bool ivug_ext_launch_nfc(const char *uri, void *data);


/*
	Below functions are not used at Now, but should be needed soon.
*/
bool ivug_ext_launch_picasa(const char *uri);

bool ivug_ext_launch_twitter(const char *uri);

/*
	Start mira cast ug.
*/
#ifdef IV_EXTENDED_FEATURES
bool ivug_ext_launch_allshare_cast(const char *szMacAddr, void *pUserData);


bool ivug_ext_launch_map(double latitude, double longitude, void *pUserData);

bool ivug_ext_launch_map_direction(double d_latitude, double d_longitude, void *pUserData);

bool ivug_ext_launch_share_text(const char *text);
#endif
bool ivug_ext_launch_slideshow(const char *path, int index);

#ifdef __cplusplus
}
#endif

#endif //__IVUG_EXT_UG_H__

