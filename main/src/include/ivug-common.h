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

#ifndef __IVUG_COMMON_H__
#define __IVUG_COMMON_H__

#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-uuid.h"

#include <Elementary.h>
#include <assert.h>

#include "statistics.h"
#include "ivug-debug.h"
#include "ivug-string.h"
#include "ivug-util.h"
#include "ivug-file-util.h"
#include "ivug-define.h"
#include "ivug-config.h"
#include "ivug-widget.h"
#include "ivug-context.h"

/*
Definition "PREFIX" is declared in CMakelist.txt
	PREFIX is "/usr/ug"
	PACKAGE is "ug-image-viewer-efl"

	Path name does not include trailing /.

	DATA_PATH is /opt/usr/ug/data
*/

#define LOCALE_PATH 					PREFIX"/res/locale"
#define IMAGE_PATH						PREFIX"/res/images/"PACKAGE
#define EDJ_PATH 						PREFIX"/res/edje/"PACKAGE
#define DATA_PATH 						DATADIR"/"PACKAGE

/*
	Home screen and Lock screen image should be put in DATA_PATH.
*/


#define NAVI_OPTION_BTN_STYLE	"naviframe/title/default"//"multiline"

#define WHITE_THEME

#ifdef WHITE_THEME
#define IVUG_DEFAULT_BG_COLOR		249
#else
#define IVUG_DEFAULT_BG_COLOR		48
#endif

#define USE_DEFAULT_DOWNLOADS_FOLDER

#ifdef USE_DEFAULT_DOWNLOADS_FOLDER
#define DEFAULT_DOWNLOADS_FOLDER "/opt/usr/media/Downloads"
#endif

#define DEFAULT_IMAGE_FOLDER "/opt/usr/media/Images"
#define DEFAULT_THUMBNAIL		"/opt/usr/share/media/.thumb/thumb_default.png"

#define PATH_SDCARD		"/opt/storage/sdcard/"

#define _EDJ(o)			elm_layout_edje_get(o)

#define IVUG_WEB_DOWNLOAD_TEMP_DIR DATA_PATH

#define MENUBAR_TIMEOUT_SEC (2.0f)

/*
	Final image path
*/
#define IVUG_HOME_SCREEN_PATH		DATA_PATH"/.homescreen.jpg";
#define IVUG_LOCK_SCREEN_PATH		DATA_PATH"/.lockscreen.jpg";

/*
	Screen path for APPSVC
*/
#define IVUG_APPSVC_HOME_SCREEN_PATH	DATA_PATH"/.iv_homescreen.jpg";

#endif /* __IVUG_COMMON_H__ */

