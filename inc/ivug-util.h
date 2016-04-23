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

#ifndef __IVUG_UTIL_H__
#define __IVUG_UTIL_H__

#include <stdbool.h>

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/*

*/
bool ivug_is_web_uri(const char* uri);

bool ivug_is_tel_uri(const char* uri);

bool ivug_remove_file(const char *filepath);

bool ivug_is_file_exist(const char* filepath);

bool ivug_rename_file(const char *src, const char *dst);

bool ivug_copy_file(const char *filename, const char *dest);

const char *ivug_get_filename(const char *filepath);

char *ivug_get_directory(const char *filepath);

unsigned int get_distance(int prevX, int prevY, int X, int Y);

/*
	Generate temporary file name with given path and extension.
	returned value should free() by user.
*/
char *ivug_mktemp(char* filepath, char*ext);


/*
	LCD sleep control.
*/
bool ivug_prohibit_lcd_off(void);
bool ivug_allow_lcd_off(void);


/*
	Get mime type from file path.
	should free returned after use.
*/

bool ivug_is_editable_video_file(char *filepath);


/*
	returned values should be freed by user
*/
int ivug_atoi(const char *number);

double ivug_atod(const char *number);

long int ivug_atox(const char *number);

char * ivug_generate_file_name(const char *filepath, const char *extension, const char *dest_dir, bool hide);

/*
	Removes leading and trailing whitespace from string
	This function doesn't allocate or reallocate any memory
*/
char * ivug_strip_string(char* name);


bool ivug_is_facebook_image(const char *file);

/*
	returned values should be freed by user
*/
#if 0
char * ivug_get_icu_date(time_t mtime);
#endif
char * ivug_get_duration_string(int millisecond);


bool ivug_validate_file_name(const char *fname);

bool ivug_is_supported_file_type(const char *fname);

#ifdef __cplusplus
}
#endif



#endif //__IVUG_UTIL_H__

