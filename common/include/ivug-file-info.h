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

#ifndef __IVUG_FILE_INFO_H__
#define __IVUG_FILE_INFO_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	IVUG_FILE_INFO_ERR_NONE,
	IVUG_FILE_INFO_ERR_PATH_IS_NULL = -1,
	IVUG_FILE_INFO_ERR_INTERNAL = -2,
	IVUG_FILE_INFO_ERR_MAX = 4
}file_info_err_e;

/*
	ivug_fileinfo_*() returns false when error. otherwise return true and fill proper value in [out] param
*/
bool ivug_fileinfo_get_image_gps_info(const char* filepath, double * /* OUT */ latitude, double * /* OUT */ longitude);
bool ivug_fileinfo_get_video_gps_info(const char *filepath, double * /* OUT */ latitude, double * /* OUT */ longitude);

bool ivug_fileinfo_get_video_resolution(const char *filepath, int * /* OUT */ pWidth, int * /* OUT */pHeight);
bool ivug_fileinfo_get_image_resolution(const char *filepath, int * /* OUT */ pWidth, int * /* OUT */pHeight);


/*
	return file extension string.

	CAUTION : user should free returned string.
*/
char *ivug_fileinfo_get_file_extension(const char *filepath);


/*
	return mime type.

	CAUTION : user should free returned string.
*/
char *ivug_fileinfo_get_mime_type(const char *filepath);
char *ivug_fileinfo_get_focal_length(const char *path);
char *ivug_fileinfo_get_model_name(const char *path);
char *ivug_fileinfo_get_maker_name(const char *path);
char *ivug_fileinfo_get_aperture(const char *path);
char *ivug_fileinfo_get_exposure_time(const char *path);
char *ivug_fileinfo_get_iso(const char *path);

int ivug_fileinfo_get_flash_status(const char *path);

/*
   1 : top left
   2 : top right
   3 : bottom right
   4 : bottom left
   5 : left top
   6 : right top
   7 : right bottom
   8 : left bottom
*/
int ivug_fileinfo_get_orientation(const char *path);


int ivug_fileinfo_get_white_balance(const char *path);

#ifdef __cplusplus
}
#endif


#endif 		// __IVUG_FILE_INFO_H__

