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

//#include <libexif/exif-data.h>	//for exif
#include <metadata_extractor.h>
#include <string.h>
#include <Evas.h>
#include <Ecore_Evas.h>
#include <Ecore_File.h>		// file get
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
/*sri
static bool _get_exif_string(ExifData *ed, ExifTag tag, const char *buf, int buf_len)
{
	ExifEntry *entry = NULL;
	//get exifentry
	entry = exif_data_get_entry(ed, tag);
	if (!entry)
	{
		return false;
	}

	// get value of the entry
	if (exif_entry_get_value(entry, (char *)buf, buf_len) == NULL)
	{
		return false;
	}
	return true;
}

static bool _get_exif_short(ExifData *ed, ExifTag tag, int *value)
{
	ExifEntry *entry = NULL;
	// get exifentry/
	entry = exif_data_get_entry(ed, tag);
	if (!entry)
	{
		return false;
	}

	// get value of the entry
	*value = exif_get_short(entry->data, exif_data_get_byte_order(entry->parent->parent));

	return true;
}
*/

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

/* LATITUDE : -90 ~ +90, LONGITUDE : -180 ~ +180) *//*
bool _get_gps_info_from_exifdata(ExifData *ed, double *latitude, double *longitude)
{
	IV_ASSERT(ed != NULL);
	IV_ASSERT(latitude != NULL);
	IV_ASSERT(longitude != NULL);

	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	double multiplier = 1.0;

	ifd = EXIF_IFD_GPS;
	tag = EXIF_TAG_GPS_LATITUDE_REF;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}

	if (buf[0] == 'S')	// SOUTH 
	{
		multiplier = -1.0;
	}

	tag = EXIF_TAG_GPS_LATITUDE;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}

	{
		buf[strlen(buf)] = '\0';
		double tmp_arr[3] = { 0.0, 0.0, 0.0 };
		int count = 0;
		char* p = strtok(buf, ", ");
		// split the buf by ,
		while (p != NULL)
		{
			tmp_arr[count] = ivug_atod(p);
			count++;
			p=strtok(NULL, ", ");
		}

		if (count != 3)
		{
			MSG_UTIL_ERROR("Cannot get latitude info : %s", p);
			return false;
		}
		*latitude = multiplier*(tmp_arr[0] + tmp_arr[1]/60 + tmp_arr[2]/3600);
	}

	multiplier = 1.0;
	tag = EXIF_TAG_GPS_LONGITUDE_REF;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}

	if (buf[0] == 'W')	// WEST 
	{
		multiplier = -1.0;
	}

	tag = EXIF_TAG_GPS_LONGITUDE;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}

	{
		buf[strlen(buf)] = '\0';
		double tmp_arr[3] = { 0.0, 0.0, 0.0 };
		int count = 0;
		char* p = strtok(buf, ", ");
		// split the buf by , 
		while (p != NULL)
		{
			tmp_arr[count] = ivug_atod(p);
			count++;
			p=strtok(NULL, ", ");
		}

		if (count != 3)
		{
			MSG_UTIL_ERROR("Cannot get Longitude info : %s", p);
			return false;
		}

		*longitude = multiplier*(tmp_arr[0] + tmp_arr[1]/60 + tmp_arr[2]/3600);
	}

	return true;
}

bool _get_image_gps_info(const char* filepath, double *latitude, double *longitude)
{
	IV_ASSERT(filepath != NULL);
	IV_ASSERT(latitude != NULL);
	IV_ASSERT(longitude != NULL);

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(filepath);
	if (!ed)
	{
		return false;
	}

	if (_get_gps_info_from_exifdata(ed, latitude, longitude) == false)
	{
		exif_data_unref(ed);
		return false;
	}

	exif_data_unref(ed);

	return true;
}

bool _get_orientation_from_exifdata(ExifData *ed, int *orient)
{
	MSG_DETAIL_HIGH("_get_orientation_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_ORIENTATION;

	if (_get_exif_short(ed, tag, orient) == false)
	{
		return false;
	}

	MSG_DETAIL_HIGH("orientation = %d", *orient);
	// 1 : top left
	//   2 : top right
	//   3 : bottom right
	//   4 : bottom left
	//   5 : left top
	//   6 : right top
	 //  7 : right bottom
	//   8 : left bottom 
	return true;
}

// out value must be freed 
static bool _get_maker_from_exifdata(ExifData *ed, char **maker)
{
	MSG_DETAIL_HIGH("_get_maker_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_MAKE;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("Maker = %s", buf);

	*maker = strdup(buf);
	return true;
}

//out value must be freed 
static bool _get_model_from_exifdata(ExifData *ed, char **model)
{
	MSG_DETAIL_HIGH("_get_model_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_MODEL;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("Model = %s", buf);

	*model = strdup(buf);
	return true;
}

bool _get_flash_from_exifdata(ExifData *ed, int *status)
{
	MSG_DETAIL_HIGH("_get_flash_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_FLASH;

	if (_get_exif_short(ed, tag, status) == false)
	{
		return false;
	}

	MSG_DETAIL_HIGH("Flash status = %d", *status);
	// LSB
	//   0b : Flash did not fire
	//   1b : Flash fired 
	return true;
}

//out value must be freed 
static bool _get_focal_length_from_exifdata(ExifData *ed, char **length)
{
	MSG_DETAIL_HIGH("_get_focal_length_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_FOCAL_LENGTH;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("Focal length = %s", buf);

	*length = strdup(buf);
	return true;
}

bool _get_white_balance_from_exifdata(ExifData *ed, int *white_balance)
{
	MSG_DETAIL_HIGH("_get_white_balance_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_WHITE_BALANCE;

	if (_get_exif_short(ed, tag, white_balance) == false)
	{
		return false;
	}

	MSG_DETAIL_HIGH("White balance = %d", *white_balance);
	// 0 : auto white balance
	 //  1 : menual white balance 
	return true;
}

bool _get_aperture_from_exifdata(ExifData *ed, char **aperture)
{
	MSG_DETAIL_HIGH("_get_aperture_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_APERTURE_VALUE;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("Aperture = %s", buf);

	*aperture = strdup(buf);
	return true;
}

bool _get_exposure_time_from_exifdata(ExifData *ed, char **exposure)
{
	MSG_DETAIL_HIGH("_get_exposure_time_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	char buf[BUF_LEN+1] = {'\0',};

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_EXPOSURE_TIME;

	if (_get_exif_string(ed, tag, buf, BUF_LEN) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("Exposure time = %s", buf);

	*exposure = strdup(buf);
	return true;
}

bool _get_iso_from_exifdata(ExifData *ed, int *iso)
{
	MSG_DETAIL_HIGH("_get_iso_from_exifdata");
	ExifIfd ifd;
	ExifTag tag;

	ifd = EXIF_IFD_EXIF;
	tag = EXIF_TAG_ISO_SPEED_RATINGS;

	if (_get_exif_short(ed, tag, iso) == false)
	{
		return false;
	}
	MSG_DETAIL_HIGH("ISO = %d", *iso);

	return true;
}
*/
/*
static bool _get_image_resolution(const char *path, int * pWidth, int *pHeight)
{
	IV_ASSERT(path != NULL);

	int width = 0;
	int height = 0;

	Evas *canvas;
	Ecore_Evas *ee;

	ee = ecore_evas_buffer_new(1, 1);
	if (!ee)
	{
		MSG_DETAIL_ERROR("Cannot get EVAS");
		return false;
	}

	canvas = ecore_evas_get(ee);

	evas_image_cache_set(canvas, 0);

	Evas_Object *img = evas_object_image_add(canvas);

	evas_object_image_file_set(img, NULL, NULL);
	//evas_object_image_load_orientation_set(img, EINA_TRUE);
	evas_object_image_load_scale_down_set(img, 0);

	evas_object_image_file_set(img, path, NULL);
	Evas_Load_Error error = evas_object_image_load_error_get(img);
	if (error != EVAS_LOAD_ERROR_NONE)
	{
		MSG_DETAIL_ERROR("Decoding Error(%d) : %s", error, path);
		evas_object_del(img);
		ecore_evas_free(ee);
		return false;
	}

	evas_object_image_size_get(img, &width, &height);

	evas_object_image_file_set(img, NULL, NULL);
	evas_object_del(img);

	ecore_evas_free(ee);

	*pWidth = width;
	*pHeight = height;

	MSG_DETAIL_HIGH("widht & height is [%d, %d]",  width, height);

	return true;
}
*/

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
/*sri
char *ivug_fileinfo_get_focal_length(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	char *length = NULL;
	if (_get_focal_length_from_exifdata(ed, &length) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	return length;
}

char *ivug_fileinfo_get_model_name(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	char *model = NULL;
	if (_get_model_from_exifdata(ed, &model) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	return model;
}

char *ivug_fileinfo_get_maker_name(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	char *maker = NULL;
	if (_get_maker_from_exifdata(ed, &maker) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	return maker;
}

int ivug_fileinfo_get_flash_status(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return -1;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return -1;
	}

	int status = 0;
	if (_get_flash_from_exifdata(ed, &status) == false)
	{
		exif_data_unref(ed);
		return -1;
	}

	exif_data_unref(ed);

	return status;
}

int ivug_fileinfo_get_orientation(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return IVUG_FILE_INFO_ERR_PATH_IS_NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return IVUG_FILE_INFO_ERR_INTERNAL;
	}

	int orient = 0;
	if (_get_orientation_from_exifdata(ed, &orient) == false)
	{
		exif_data_unref(ed);
		return IVUG_FILE_INFO_ERR_INTERNAL;
	}

	exif_data_unref(ed);

	return orient;
}

int ivug_fileinfo_get_white_balance(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return -1;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return -1;
	}

	int status = 0;
	if (_get_white_balance_from_exifdata(ed, &status) == false)
	{
		exif_data_unref(ed);
		return -1;
	}

	exif_data_unref(ed);

	return status;
}

char *ivug_fileinfo_get_aperture(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	char *aperture = NULL;
	if (_get_aperture_from_exifdata(ed, &aperture) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	return aperture;
}

char *ivug_fileinfo_get_exposure_time(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	char *exposure = NULL;
	if (_get_exposure_time_from_exifdata(ed, &exposure) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	return exposure;
}

char *ivug_fileinfo_get_iso(const char *path)
{
	if (path == NULL)
	{
		MSG_UTIL_ERROR("Cannot get mine type. path is NULL");
		return NULL;
	}

	char buf[BUF_LEN] = {0,};
	ExifData *ed = NULL;

	// get exifdata
	ed = exif_data_new_from_file(path);
	if (!ed)
	{
		return NULL;
	}

	int iso = -1;
	if (_get_iso_from_exifdata(ed, &iso) == false)
	{
		exif_data_unref(ed);
		return NULL;
	}

	exif_data_unref(ed);

	snprintf(buf, sizeof(buf), "%d", iso);

	return strdup(buf);
}*/

