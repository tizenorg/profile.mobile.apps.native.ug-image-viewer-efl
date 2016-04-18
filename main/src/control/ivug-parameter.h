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

#ifndef __IVUG_PARAMETER_H__
#define __IVUG_PARAMETER_H__

#include <stdbool.h>
#include <app.h>
#include <Elementary.h>

#include "ivug-define.h"
#include "ivug-datatypes.h"
#include "ivug-filter.h"

typedef enum {
	IVUG_MODE_INVAILD = 0x00,
	IVUG_MODE_NORMAL, 				// include web album
	IVUG_MODE_SINGLE,				// from My Files
	IVUG_MODE_DISPLAY,				// just show one image with no menu
	IVUG_MODE_SAVE,					// from "take a photo" in other app
	IVUG_MODE_FILE,					// file viewer(can flick)
	IVUG_MODE_CAMERA,				// camera(can flick)
	IVUG_MODE_SETAS,				// SetAs in setting menu.
	IVUG_MODE_SLIDESHOW,			// Slideshow
	IVUG_MODE_HELP,
	IVUG_MODE_EMAIL,
	IVUG_MODE_SELECT,
	IVUG_MODE_HIDDEN
} ivug_mode;

//set as ug
typedef enum {
	IVUG_SET_AS_UG_TYPE_INVALID = 0x00,
	IVUG_SET_AS_UG_TYPE_WALLPAPER,
	IVUG_SET_AS_UG_TYPE_LOCKSCREEN,
	IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN,
	IVUG_SET_AS_UG_TYPE_CALLER_ID,
	IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID,
	IVUG_SET_AS_UG_TYPE_CROP,			// For OSP
	IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP,
	IVUG_SET_AS_UG_TYPE_MAX,
} ivug_setas_type;

typedef struct {
	ivug_mode mode;
	ivug_setas_type setas_type;		// Only needed when mode is IVUG_MODE_SETAS

	bool bStandalone;	// launched by appsvc

	ivug_view_by view_by;

	char* filepath;
	UUID album_id;	//cluster id of media service

	ivug_media_type media_type;
	ivug_sort_type sort_type;		// TODO : extract from here. sort type can be used in case of default

	/* for places view */
	bool footsteps;
	double max_longitude;
	double min_longitude;
	double max_latitude;
	double min_latitude;

	/* for time line view */
	long timeline_start;
	long timeline_end;

	/* for set as view crop box*/
	unsigned int width;
	unsigned int height;
	bool bRatioFix;

	bool bTestMode;			// Indicate test mode. when test mode, application is terminate when back-key is clicked
	int tag_id;

	int start_index;

	Eina_List *selected_list;
	Eina_List * /* filepath */ multiple_list;
	int total_selected;
	int select_view_max_count;
	long int select_view_limit_size;
	long int select_view_selected_size;
} ivug_parameter;


#ifdef __cplusplus
extern "C" {
#endif


ivug_parameter*
ivug_param_create_from_bundle(app_control_h service);

void
ivug_param_delete(ivug_parameter* data);


/*
	Covert ivug_paramter to db filter or allshare filter
*/
Filter_struct *
ivug_param_create_filter(const ivug_parameter *param);

#ifdef __cplusplus
}
#endif



#endif // __IVUG_PARAMETER_H__


