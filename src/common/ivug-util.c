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


#include "ivug-define.h"
#include "ivug-uuid.h"

#include "ivug-util.h"
#include "ivug-file-util.h"
#include "ivug-debug.h"
#include "ivug-file-info.h"
#include "ivug-db.h"

#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <glib.h>
#include<limits.h>

#include <system_settings.h>

#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <unicode/uloc.h>
#include <unicode/ucal.h>
#include <unicode/udatpg.h>
#include <unicode/utmscale.h>

/*
	If filepath is web url, return TRUE.

*/
bool ivug_is_web_uri(const char* uri)
{
//check file url type. local , http, ftp.
	IV_ASSERT(uri != NULL);

	static const char* web_protocal_name[] =
	{
		"http://",
		"https://",
		"ftp://",
		NULL,
	};

	int i = 0;
	while (web_protocal_name[i] != NULL) {
		if (strlen(uri) > strlen(web_protocal_name[i])) {
			if (strncmp(uri, web_protocal_name[i], strlen(web_protocal_name[i])) == 0) {
				return true;
			}
		}

		i++;
	}

	MSG_UTIL_MED("Not web uri. %s", uri);

	return false;
}

bool ivug_is_tel_uri(const char* uri)
{
//check file url type. local , http, ftp.
	IV_ASSERT(uri != NULL);

	static const char* tel_protocal_name[] =
	{
		"tel:",
		NULL,
	};

	int i = 0;
	while (tel_protocal_name[i] != NULL) {
		if (strlen(uri) > strlen(tel_protocal_name[i])) {
			if (strncmp(uri, tel_protocal_name[i], strlen(tel_protocal_name[i])) == 0) {
				return true;
			}
		}

		i++;
	}

	MSG_UTIL_MED("Not tel uri. %s", uri);

	return false;
}

unsigned int get_distance(int prevX, int prevY, int X, int Y)
{
	int dx = prevX - X;
	int dy = prevY - Y;

	return sqrt(dx*dx + dy*dy);
}

#define USE_ECORE_FILE

#include <Ecore_File.h>

/*
	Remove fname file.
	Returns true fname is not exist or removed sucessfully
*/
bool ivug_remove_file(const char *filepath)
{
	char error_msg[256];
	if (ivug_file_exists(filepath) == EINA_FALSE) {
		MSG_UTIL_ERROR("Already removed.%s", filepath);
		return true;
	}

#ifdef USE_ECORE_FILE
	if (ivug_file_unlink(filepath) == EINA_FALSE) {
		MSG_UTIL_ERROR("Cannot remove file : %s %s", filepath, strerror_r(errno, error_msg, sizeof(error_msg)));
		return false;
	}

	return true;
#else
	if (unlink(filepath) != 0) {
		MSG_UTIL_ERROR("Cannot remove file : %s %s", filepath, strerror_r(errno, error_msg, sizeof(error_msg)));
		return false;
	}

	return true;
#endif
}

bool ivug_rename_file(const char *src, const char *dst)
{
	if (!src) {
		MSG_UTIL_ERROR("source path is NULL");
		return false;
	}

	if (!dst) {
		MSG_UTIL_ERROR("Destination path is NULL");
		return false;
	}

	if (ivug_file_exists(src) == EINA_FALSE) {
		MSG_UTIL_ERROR("Source file is not exist : %s", src);
		return false;
	}

	if (ivug_file_exists(dst) == EINA_TRUE) {
		MSG_UTIL_ERROR("Destination file is exist : %s", src);
		return false;
	}

	char error_msg[256] = {0,};

	if (rename(src, dst) < 0) {
		MSG_UTIL_ERROR("Cannot rename from %s to %s : %s", src, dst, strerror_r(errno, error_msg, sizeof(error_msg)));
		return false;
	}

	return true;
}

bool ivug_copy_file(const char *filename, const char *dest)
{
#define DIR_MASK_DEFAULT 0775
	if (filename == NULL) {
		MSG_MAIN_ERROR("File does not existfilepath=%s", filename);
		return false;
	}

	if (ivug_file_cp(filename, dest) == EINA_FALSE) {
		MSG_MAIN_ERROR("ivug_file_cp failed. From %s To %s", filename, dest);
		return false;
	}

	return true;
}

char *
ivug_mktemp(char* filepath, char*ext)
{
	ivug_retv_if(!filepath || !ext, NULL);

	MSG_IVUG_HIGH("filepath %s, ext %s", filepath, ext);

	char tempname[IVUG_MAX_FILE_PATH_LEN+1] = {0};
	snprintf(tempname, sizeof(tempname), "%s_0.%s",filepath, ext);
	int i = 1;

// TODO : Will implement with another algorithm
	while (ivug_file_exists(tempname) == EINA_TRUE) {
		snprintf(tempname, sizeof(tempname),"%s_%d.%s", filepath, i, ext);
		i++;
	}

	MSG_IVUG_HIGH(" tempname %s, i %d", tempname, i);

	return strdup(tempname);
}

/*
 	Check whether given filepath file exists

 	CAUTION : filepath cannot be NULL.
*/
bool ivug_is_file_exist(const char* filepath)
{
	IV_ASSERT(filepath != NULL);

	if (ivug_file_exists(filepath) == EINA_TRUE) {
		return true;
	}

	return false;
}


/*
	USer should freed returned value!
*/
char *ivug_get_directory(const char *filepath)
{
	if (filepath == NULL) {
		MSG_UTIL_WARN("File path is NULL");
		return "NULL";
	}

#ifdef USE_ECORE_FILE
	return ivug_dir_get(filepath);
#else
	#error "Not implemented yet."
#endif
}

/*
	Returns start pointer of filename within filepath.
	No memory allocated in this function. so user do not free returned pointer.

	CAUTION : filepath cannot be NULL.
*/
const char * ivug_get_filename(const char *filepath)
{
	if (filepath == NULL) {
		MSG_UTIL_WARN("File path is NULL");
		return "NULL";
	}

#ifdef USE_ECORE_FILE
	return ivug_file_get(filepath);
#else

#define DIRECORY_SPLITTER '/'
	const char *pFileName = NULL;

	pFileName  =  strrchr(filepath, DIRECORY_SPLITTER);

	if (pFileName == NULL) {		// If not found
		return filepath;
	}

	return (pFileName+1);
#endif

}

bool ivug_prohibit_lcd_off(void)
{
	MSG_UTIL_MED("START : Sleep disabled");
//	return (power_lock_state(POWER_STATE_NORMAL, 0)==0 ? true : false);
	return false;
}

bool ivug_allow_lcd_off(void)
{
	MSG_UTIL_MED("END : Sleep disabled");
//	return (power_unlock_state(POWER_STATE_NORMAL)==0 ? true : false);
	return false;
}

long int ivug_atox(const char *number)
{
	char *endptr = NULL;
	long long int val = 0;

	errno = 0;

	val = strtoll(number, &endptr, 16);

	if ((errno == ERANGE && (val == LLONG_MAX || val == LLONG_MIN)) || (errno != 0 && val == 0)) {
		MSG_UTIL_ERROR("ERANGE = %d, LONG_MAX = %d, LONG_MIN = %d", ERANGE, LLONG_MAX, LLONG_MIN);
		MSG_UTIL_ERROR("strtol, val = %d, 0x%x, errno = %d, ", val, val, errno);
		return -1L;
	}

	if (endptr == number) {
		MSG_UTIL_ERROR("No digits were found, number = %s", number);
		return -1L;
	}

	return (long int)val;
}

int ivug_atoi(const char *number)
{
	char *endptr = NULL;
	long val = 0;

	errno = 0;

	val = strtol(number, &endptr, 10);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
		MSG_UTIL_ERROR("strtol, val = %d", val);
		return -1;
	}

	if (endptr == number) {
		MSG_UTIL_ERROR("No digits were found, number = %s", number);
		return -1;
	}

	return (int)val;
}

double ivug_atod(const char *number)
{
	char *endptr = NULL;
	double val = 0;

	errno = 0;

	val = strtod(number, &endptr);

	if ((errno == ERANGE && (val == -HUGE_VAL || val == HUGE_VAL)) || (errno != 0 && val == 0)) {
		MSG_UTIL_ERROR("strtod, val = %d", val);
		return -1;
	}

	if (endptr == number) {
		MSG_UTIL_ERROR("No digits were found, number = %s", number);
		return -1;
	}

	return val;
}

char * ivug_generate_file_name(const char *filepath, const char *extension, const char *dest_dir, bool hide)
{
	IV_ASSERT(filepath != NULL);

	MSG_SETAS_HIGH("filepath %s", filepath);

	char tempname[IVUG_MAX_FILE_PATH_LEN+1] = {0,};
	char *ext = NULL;
	char *filename = ivug_strip_ext(ivug_file_get(filepath));
	char *dir = NULL;

	if (extension) {
		ext = strdup(extension);
	} else {
		ext = ivug_fileinfo_get_file_extension(filepath);
	}

	if (dest_dir) {
		dir = strdup(dest_dir);
	} else {
		dir = ivug_get_directory(filepath);
	}

	int i = 0;

	for (i = 1; i < IVUG_MAX_FILE_PATH_LEN; i++) {
		if (hide == false) {
			snprintf(tempname, sizeof(tempname), "%s/%s_%d.%s", dir, filename, i, ext);
		} else {
			snprintf(tempname, sizeof(tempname), "%s/.%s_%d.%s", dir, filename, i, ext);
		}
		if (ivug_file_exists(tempname) == EINA_FALSE)
			break;
	}

	if (filename) {
		free(filename);
	}
	if (dir) {
		free(dir);
	}
	if (ext) {
		free(ext);
	}
	if (i == IVUG_MAX_FILE_PATH_LEN) {
		MSG_UTIL_ERROR("Cannot make file");
		return NULL;
	}

	MSG_UTIL_MED("tempname: %s", tempname);

	return strdup(tempname);
}

char * ivug_strip_string(char* name)
{
	MSG_UTIL_HIGH("Input name: %s", name);
	/* Check name length */
	if (strlen(name) == 0) {
		MSG_UTIL_WARN("Inserted text is empty!");
		return NULL;
	}
	MSG_UTIL_MED("Inserted name: %s, length: %d", name, strlen(name));

	/* Removes leading and trailing whitespace */
	g_strstrip(name);
	if (strlen(name) == 0) {
		MSG_UTIL_WARN("name includes only space!");
		return NULL;
	}
	return name;
}

bool ivug_is_facebook_image(const char *file)
{
#define FACEBOOL_ABLUM_PATH "/opt/usr/media/.com.samsung.facebook-service/data/album_sync/"
	ivug_retv_if(file == NULL, false);

	char *dir = ivug_dir_get(file);
	if (dir != NULL && strncmp(dir, FACEBOOL_ABLUM_PATH, strlen(FACEBOOL_ABLUM_PATH)) == 0) {
		free(dir);
		return true;
	}
	free(dir);
	return false;
}

bool ivug_validate_file_name(const char *fname)
{
	if (!fname)
		return false;

	const char badchar[] = "\\/;:*?<>|\"";
	const char *tmp = fname;

	/* check bad character */
	while (*tmp != '\0') {
		if (strchr(badchar, *tmp) != NULL) {
			MSG_UTIL_ERROR("Invalid file name=%s char=%c", fname, *tmp);
			return false;
		}
		tmp++;
	}
	return true;
}

char* ivug_get_duration_string(int millisecond)
{
	int second = (millisecond/1000) % 60;
	int minute = (millisecond/(1000*60)) % 60;
	int hour = (millisecond/(1000*60*60)) % 24;

	char buf[MAX_BYTE_LEN] = {0,};

	if (hour) {
		snprintf(buf, MAX_BYTE_LEN, "%d:%02d:%02d", hour, minute, second);
	} else {
		snprintf(buf, MAX_BYTE_LEN, "%02d:%02d", minute, second);
	}

	return strdup(buf);
}

bool ivug_is_supported_file_type(const char *fname)
{
	bool ret = false;
	char *mime_type = NULL;
	mime_type = ivug_fileinfo_get_mime_type(fname);
	if (mime_type == NULL) {
		MSG_SDATA_WARN("file path is not vaild = %s", fname);
		return false;
	}

	//image
	if (strncmp(mime_type, "image/jpeg", strlen("image/jpeg")) == 0
		|| strncmp(mime_type, "image/bmp", strlen("image/bmp")) == 0
		|| strncmp(mime_type, "image/png", strlen("image/png")) == 0
		|| strncmp(mime_type, "image/gif", strlen("image/gif")) == 0
		|| strncmp(mime_type, "image/vnd.wap.wbmp", strlen("image/vnd.wap.wbmp")) == 0)
	{
		ret = true;
	} else if (strncmp(mime_type, "video/", strlen("video/")) == 0) {
		ret = true;
	} else {
		MSG_SDATA_WARN("not supported file type = %s", fname);
		ret = false;
	}

	free(mime_type);
	return ret;
}
