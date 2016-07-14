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
#include "ivug-file-info.h"
#include "ivug-debug.h"
#include "ivug-util.h"
#include "ivug-file-util.h"

#include <metadata_extractor.h>
#include <string.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>
#include <Evas.h>

#include <mime_type.h>

#define BUF_LEN (255)

static const char *_conver_error(int err)
{
	switch (err)
	{
		case METADATA_EXTRACTOR_ERROR_NONE:
			return "Successful";
		case METADATA_EXTRACTOR_ERROR_INVALID_PARAMETER:
			return "Invalid parameter";
		case METADATA_EXTRACTOR_ERROR_OUT_OF_MEMORY:
			return "Out of memory";
		case METADATA_EXTRACTOR_ERROR_FILE_EXISTS:
			return "File already exists";
		case METADATA_EXTRACTOR_ERROR_OPERATION_FAILED:
			return "Operation failed";
		default:
		{
			static char error[128];
			snprintf(error, 128, "Unknow Error : %d(0x%08x)", err, err);
			return error;
		}
	}
	return NULL;
}

bool _get_video_gps_info(const char *filepath, double *latitude, double *longitude)
{
	IV_ASSERT(filepath != NULL);
	IV_ASSERT(latitude != NULL);
	IV_ASSERT(longitude != NULL);

	int ret = METADATA_EXTRACTOR_ERROR_NONE;
	metadata_extractor_h metadata;

	char *latitude_str = NULL;
	char *longitude_str = NULL;

	*latitude = 0.0;
	*longitude = 0.0;

	ret = metadata_extractor_create(&metadata);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_UTIL_ERROR("Fail metadata_extractor_create [%s]", _conver_error(ret));
		return false;
	}

	ret = metadata_extractor_set_path(metadata, filepath);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_UTIL_ERROR("Fail metadata_extractor_set_path [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		return false;
	}

	ret = metadata_extractor_get_metadata(metadata, METADATA_LATITUDE, &latitude_str);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_UTIL_ERROR("Fail metadata_extractor_get_metadata [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		if (latitude_str != NULL) {
			free(latitude_str);
			latitude_str = NULL;
		}
		return false;
	}

	ret = metadata_extractor_get_metadata(metadata, METADATA_LONGITUDE, &longitude_str);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_UTIL_ERROR("Fail metadata_extractor_get_metadata [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		if (longitude_str != NULL) {
			free(longitude_str);
			longitude_str = NULL;
		}
		if (latitude_str != NULL) {
			free(latitude_str);
			latitude_str = NULL;
		}
		return false;
	}

	if (latitude_str && longitude_str) {
		MSG_UTIL_LOW("lat = %s, longi = %s", latitude_str, longitude_str);
		*latitude = ivug_atod(latitude_str);
		*longitude = ivug_atod(longitude_str);
	}

	ret = metadata_extractor_destroy(metadata);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE) {
		MSG_UTIL_ERROR("Fail metadata_extractor_destroy [%s]", _conver_error(ret));
	}
	if (longitude_str != NULL) {
		free(longitude_str);
		longitude_str = NULL;
	}
	if (latitude_str != NULL) {
		free(latitude_str);
		latitude_str = NULL;
	}
	return true;
}

static bool
_get_video_resolution(const char *path, int * /* OUT */ pWidth, int * /* OUT */pHeight)
{
	IV_ASSERT(path != NULL);

	int ret = METADATA_EXTRACTOR_ERROR_NONE;
	metadata_extractor_h metadata;

	char *width_str;
	char *height_str;

	ret = metadata_extractor_create(&metadata);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE)
	{
		MSG_UTIL_ERROR("Fail metadata_extractor_create [%s]", _conver_error(ret));
		return false;
	}

	ret = metadata_extractor_set_path(metadata, path);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE)
	{
		MSG_UTIL_ERROR("Fail metadata_extractor_set_path [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		return false;
	}

	ret = metadata_extractor_get_metadata(metadata, METADATA_VIDEO_WIDTH, &width_str);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE)
	{
		MSG_UTIL_ERROR("Fail metadata_extractor_get_metadata [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		if (width_str != NULL) {
			free(width_str);
			width_str = NULL;
		}
		return false;
	}

	ret = metadata_extractor_get_metadata(metadata, METADATA_VIDEO_HEIGHT, &height_str);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE)
	{
		MSG_UTIL_ERROR("Fail metadata_extractor_get_metadata [%s]", _conver_error(ret));
		ret = metadata_extractor_destroy(metadata);
		MSG_UTIL_HIGH("metadata_extractor_destroy [%s]", _conver_error(ret));
		if (height_str != NULL) {
			free(height_str);
			height_str = NULL;
		}
		if (width_str != NULL) {
			free(width_str);
			width_str = NULL;
		}
		return false;
	}

	if (width_str && height_str) {
		MSG_UTIL_LOW("w = %s, h = %s", width_str, height_str);
		*pWidth = ivug_atoi(width_str);
		*pHeight = ivug_atoi(height_str);
	}
	ret = metadata_extractor_destroy(metadata);
	if (ret != METADATA_EXTRACTOR_ERROR_NONE)
	{
		MSG_UTIL_ERROR("Fail metadata_extractor_destroy [%s]", _conver_error(ret));
	}
	if (height_str != NULL) {
		free(height_str);
		height_str = NULL;
	}
	if (width_str != NULL) {
		free(width_str);
		width_str = NULL;
	}
	return true;
}

bool ivug_fileinfo_get_image_resolution(const char *path, int * /* OUT */ pWidth, int * /* OUT */pHeight)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get image resolution. path is NULL");
		return false;
	}

	if (ivug_is_file_exist(path) == false)
	{
		MSG_UTIL_ERROR("%s : %s is not exist", __func__, path);
		return false;
	}

	return false;
}


bool ivug_fileinfo_get_video_resolution(const char *path, int * /* OUT */ pWidth, int * /* OUT */pHeight)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get video resolution. path is NULL");
		return false;
	}

	if (ivug_is_file_exist(path) == false)
	{
		MSG_UTIL_ERROR("%s : %s is not exist", __func__, path);
		return false;
	}

	return _get_video_resolution(path, pWidth, pHeight);
}

bool ivug_fileinfo_get_video_gps_info(const char *path, double *latitude, double *longitude)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get video gps location. path is NULL");
		return false;
	}

	if (ivug_is_file_exist(path) == false)
	{
		MSG_UTIL_ERROR("%s : %s is not exist", __func__, path);
		return false;
	}

	return _get_video_gps_info(path, latitude, longitude);
}


bool ivug_fileinfo_get_image_gps_info(const char* path, double *latitude, double *longitude)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get image gps location. path is NULL");
		return false;
	}

	if (ivug_is_file_exist(path) == false)
	{
		MSG_UTIL_ERROR("%s : %s is not exist", __func__, path);
		return false;
	}

	return false;
}


char *ivug_fileinfo_get_file_extension(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get file extension. path is NULL");
		return NULL;
	}

	char *ext = NULL;

	ext = strrchr(ivug_file_get(path), '.');

	if ((ext != NULL) && ((ext+1) != NULL))
	{
		return strdup(ext + 1);
	}

	return NULL;

}

char *ivug_fileinfo_get_mime_type(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}
	//check mime type.
	char *mime_type = NULL;
	char *type = NULL;
	int retcode = -1;
	char *ext = NULL;

	ext = strrchr(path, '.');
	if (ext != NULL) {
		ext++;
		retcode = mime_type_get_mime_type(ext, &type);
	} else {
		retcode = mime_type_get_mime_type(path, &type);
	}

	if ((type == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
		MSG_UTIL_ERROR("Fail to get mime type with return value [%d]", retcode);
		return NULL;
	}

	MSG_UTIL_MED("mime type = %s", type);
	if (type != NULL) {
		mime_type = strdup(type);
	}
	free(type);
	type = NULL;
	return mime_type;
}
