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

#ifndef __IVUG_CROP_VIEW_H__
#define __IVUG_CROP_VIEW_H__

#include "ivug-define.h"

typedef enum {
	IVUG_SETAS_NORMAL,	// launched from mainview
	IVUG_SETAS_UG,		// ug mode
	IVUG_SETAS_APPSVC,	// appsvc mode
} ivug_setas_mode;

typedef enum {
	IVUG_CTRLBAR_SET_SCREEN_HOME,
	IVUG_CTRLBAR_SET_SCREEN_LOCK,
	IVUG_CTRLBAR_SET_SCREEN_BOTH,
	IVUG_CTRLBAR_SET_SCREEN_UNDEFINED,		// Show select popup when click ok button.
	IVUG_CTRLBAR_SET_SCREEN_MAX
} ivug_set_screen_type;

typedef enum {
	CROP_ERROR_TYPE_NONE,
	CROP_ERROR_TYPE_UNKNOWN_FORMAT,
	CROP_ERROR_TYPE_PERMISSION_DENIED,
	CROP_ERROR_TYPE_INVALID_FILE,
	CROP_ERROR_TYPE_GENERAL,
	CROP_ERROR_TYPE_TOO_SMALL,
	CROP_ERROR_TYPE_MAX
} Crop_Error;

typedef struct {
	Evas_Object *layout;
	Evas_Object *notify;

	Evas_Object *toolbar;

	Evas_Object *photocam;
	Evas_Object *cropbox;

	Evas_Object *gesture;

	Evas_Object *btn_back;

	Evas_Object *contents_area;
	Evas_Object *notify_area;

	Elm_Object_Item *btn_ok;

	Evas_Object *popup;

	bool bUseRotate;
	char *file_path;
	char *result_path;

	char *dest_dir;
	char *dest_name;

	int init_x;
	int init_y;
	int init_w;
	int init_h;

	int w;
	int h;

	struct {		// Previous image rect position. used by lcd rotate
		int x;
		int y;
		int w;
		int h;
	} prev;

	int min_size;

// When bTransEnd & bLoaded is all true, then rectagle is displaying
	bool bTransEnd;		// Transition is ended
	bool bLoaded;		// Image is loaded.

	bool bFixedRatio;
	bool bUseRatio;
	double ratio;
} IvugCropView;

/*
	Used for cropping images. view does not find face info. this is different from setas view.

	signals
		"loaded" with error code
		"ok,clicked"		- Button "OK" clicked with filepath
		"cancel,clicked"	- Button "Cancel" clicked
		"destroyed"
*/

#endif 		// __IVUG_CROP_VIEW_H__

