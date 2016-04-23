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

#include <string.h>
#include <limits.h>

#include "ivug-common.h"
#include "ivug-debug.h"
#include "ivug-parameter.h"
#include "ivug-file-info.h"
#include "ivug-base.h"
#include "ivug-main-view.h"
#include "ivug-crop-ug.h"
#include "ivug-context.h"
#include <notification.h>

#include "ivug-db.h"
#include "ivug-util.h"

/*
static inline char * ivug_get_resource_path() {
	char * path = app_get_resource_path();
	return path;
}

static inline char* full_path(char *str1, char *str2) {
	char path[1024] = {};
	snprintf(path, 1024, "%s%s", str1, str2);
	char *full_path = strdup(path);
	return full_path;
}*/

/*

Phone -> Menu -> Call setting -> Videocall Image

W/IV-COMMON(1356): 0:00:00.012[F:ivug-parameter.c L:  416][HIGH] **********************************
W/IV-COMMON(1356): debug.c: __custom_sec_debug_msg(380) > [SECURE_LOG] 0:00:00.013[F:ivug-parameter.c L:  374][SECURE]   Path : /opt/usr/media/Images/image5.jpg
W/IV-COMMON(1356): debug.c: __custom_sec_debug_msg(380) > [SECURE_LOG] 0:00:00.013[F:ivug-parameter.c L:  374][SECURE]   View Mode : SETAS
W/IV-COMMON(1356): debug.c: __custom_sec_debug_msg(380) > [SECURE_LOG] 0:00:00.013[F:ivug-parameter.c L:  374][SECURE]   Setas type : VideoCallID
W/IV-COMMON(1356): debug.c: __custom_sec_debug_msg(380) > [SECURE_LOG] 0:00:00.013[F:ivug-parameter.c L:  374][SECURE]   Resolution : 176x144
W/IV-COMMON(1356): 0:00:00.013[F:ivug-parameter.c L:  418][HIGH] **********************************

*/


//bundle key
#define IVUG_BUNDLE_KEY_VIEW_MODE 		"View Mode"
#define IVUG_BUNDLE_KEY_PATH			"Path"
#define IVUG_BUNDLE_KEY_THUMBNAIL_PATH	"Thumb Path"
#define IVUG_BUNDLE_KEY_ALBUM_IDX 		"Album index"
#define IVUG_BUNDLE_KEY_VIEW_BY			"View By"
#define IVUG_BUNDLE_KEY_INDEX			"Index"
#define IVUG_BUNDLE_KEY_SORT_BY			"Sort By"
#define IVUG_BUNDLE_KEY_FOOTSTEPS		"Footsteps"

#define IVUG_BUNDLE_KEY_MAX_LONGITUDE	"LON_MAX"
#define IVUG_BUNDLE_KEY_MIN_LONGITUDE	"LON_MIN"
#define IVUG_BUNDLE_KEY_MAX_LATITUDE	"LAT_MAX"
#define IVUG_BUNDLE_KEY_MIN_LATITUDE	"LAT_MIN"

#define IVUG_BUNDLE_KEY_TIMELINE_START	"Timeline_Start"
#define IVUG_BUNDLE_KEY_TIMELINE_END	"Timeline_End"

#define IVUG_BUNDLE_KEY_SETAS_TYPE		"Setas type"
#define IVUG_BUNDLE_KEY_TAG_NAME		"Tag name"

#define IVUG_BUNDLE_KEY_RESOLUTION		"Resolution"
#define IVUG_BUNDLE_KEY_FIXED_RATIO		"Fixed ratio"

#define IVUG_BUNDLE_KEY_CROP_MODE		"http://tizen.org/appcontrol/data/image/crop_mode"
#define IVUG_BUNDLE_VALUE_CROP_MODE_AUTO		"auto"
#define IVUG_BUNDLE_VALUE_CROP_MODE_FIT_TO_SCREEN	"fit_to_screen"

#define IVUG_BUNDLE_KEY_MEDIA_TYPE		"Media type"

#define IVUG_BUNDLE_KEY_STANDALONE		"Standalone"

#define IVUG_BUNDLE_KEY_CONTACT_ID		"Contact id"
#define IVUG_BUNDLE_KEY_CLUSTER_ID		"Cluster id"
#define IVUG_BUNDLE_KEY_GROUP_ID		"Group id"

#define IVUG_BUNDLE_KEY_SELECTED_INDEX	"Selected index"
#define IVUG_BUNDLE_KEY_SELECT_SIZE		"Select Size"
#define IVUG_BUNDLE_KEY_SELECTED_INDEX_FAV	"Selected index fav"

#define IVUG_BUNDLE_SORT_DATE			"Date"
#define IVUG_BUNDLE_SORT_DATEDESC		"DateDesc"

//default values
#define IVUG_DEFAULT_MODE IVUG_MODE_SINGLE
#define IVUG_DEFAULT_INDEX (1)				// First item
#define IVUG_FILE_PREFIX		"file://"

#define UG_RES_PATH 		ivug_get_resource_path()
#define IMG_PATH full_path(UG_RES_PATH, "edje/images")


static inline
void _ivug_free(char **val)
{
	free(*val);
	*val = NULL;
}

static ivug_mode
_get_view_mode(ivug_parameter* data, const char* val)
{
	IV_ASSERT(val != NULL);

#define IVUG_BUNDLE_VALUE_VIEW_MODE_ALBUM 	"ALBUM"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_NORMAL 	"NORMAL"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_SINGLE 	"SINGLE"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_GALLERY 	"GALLERY"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_CAMERA	"CAMERA"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_DISPLAY	"DISPLAY"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_SAVE	"SAVE"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_SETAS		"SETAS"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_SLIDESHOW   "SLIDESHOW"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_EMAIL 	"EMAIL"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_SELECT		"SELECT"
#define IVUG_BUNDLE_VALUE_VIEW_MODE_HELP 	"HELP"

	int len = strlen(val);

	if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_NORMAL, len) == 0
			|| strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_GALLERY, len) == 0) {
		return IVUG_MODE_NORMAL;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_ALBUM, len) == 0) {
		// Probably, UnUsed.
		MSG_MAIN_HIGH("ALBUM is deprecated!!!! plz check");
		return IVUG_MODE_NORMAL;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_CAMERA, len) == 0) {
		data->start_index = 1;						//apply window loading
		data->view_by = IVUG_VIEW_BY_FOLDER;		// In camera case, All images should be shown in camera folder.
		return IVUG_MODE_CAMERA;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_SINGLE, len) == 0) {
		// All menu is available
		if (data->view_by != IVUG_VIEW_BY_FOLDER)
			data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_SINGLE;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_DISPLAY, len) == 0) {
		// No menu except SETAS menu
		data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_DISPLAY;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_SAVE, len) == 0) {
		data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_SAVE;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_SETAS, len) == 0) {
		data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_SETAS;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_SELECT, len) == 0) {
		data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_SELECT;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_SLIDESHOW, len) == 0) {
		if (data->view_by == IVUG_VIEW_BY_INVAILD) {
			data->view_by = IVUG_VIEW_BY_FOLDER;
		}
		return IVUG_MODE_SLIDESHOW;
	} else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_MODE_EMAIL, len) == 0) {
		data->view_by = IVUG_VIEW_BY_FILE;
		return IVUG_MODE_EMAIL;
	}
	MSG_MAIN_HIGH("Invalid mode : %s", val);
	return IVUG_MODE_INVAILD;
}


static ivug_view_by
_get_view_by(const char* val)
{
	//bundle value
#define IVUG_BUNDLE_VALUE_VIEW_BY_ALL			"All"
#define IVUG_BUNDLE_VALUE_VIEW_BY_HIDDEN_ALL	"Hidden_All"
#define IVUG_BUNDLE_VALUE_VIEW_BY_FAVORITES		"Favorites"
#define IVUG_BUNDLE_VALUE_VIEW_BY_TAGS			"Tags"
#define IVUG_BUNDLE_VALUE_VIEW_BY_FLODER		"By Folder"
#define IVUG_BUNDLE_VALUE_VIEW_BY_HIDDEN_FLODER	"Hidden_Folder"
#define IVUG_BUNDLE_VALUE_VIEW_BY_PLACES		"Places"
#define IVUG_BUNDLE_VALUE_VIEW_BY_TIMELINE		"Timeline"

	IV_ASSERT(val != NULL);

	int len = strlen(val);

	if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_FLODER, len) == 0)
	{
		return IVUG_VIEW_BY_FOLDER;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_HIDDEN_FLODER, len) == 0)
	{
		return IVUG_VIEW_BY_HIDDEN_FOLDER;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_ALL, len) == 0)
	{
		return IVUG_VIEW_BY_ALL;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_HIDDEN_ALL, len) == 0)
	{
		return IVUG_VIEW_BY_HIDDEN_ALL;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_FAVORITES, len) == 0)
	{
		return IVUG_VIEW_BY_FAVORITES;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_TAGS, len) == 0)
	{
		return IVUG_VIEW_BY_TAG;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_PLACES, len) == 0)
	{
		return IVUG_VIEW_BY_PLACES;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_VIEW_BY_TIMELINE, len) == 0)
	{
		return IVUG_VIEW_BY_TIMELINE;
	}

	MSG_MAIN_HIGH("Invalid view by : %s", val);

	return IVUG_VIEW_BY_FILE;

}


static ivug_setas_type
_get_setas_type(const char* val)
{
	IV_ASSERT(val != NULL);

#define IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER	"Wallpaper"
#define IVUG_BUNDLE_VALUE_SETAS_UG_LOCKSCREEN	"Lockscreen"
#define IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER_N_LOCKSCREEN	"Wallpaper & Lockscreen"
#define IVUG_BUNDLE_VALUE_SETAS_UG_CALLERID		"CallerID"
#define IVUG_BUNDLE_VALUE_SETAS_UG_VIDEO_CALLEID	"VideoCallID"
#define IVUG_BUNDLE_VALUE_SETAS_UG_CROP			"Crop"
#define IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER_CROP	"Wallpaper Crop"

	int len = strlen(val);

	if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_WALLPAPER;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER_CROP, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_LOCKSCREEN, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_LOCKSCREEN;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_WALLPAPER_N_LOCKSCREEN, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_CALLERID, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_CALLER_ID;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_VIDEO_CALLEID, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_SETAS_UG_CROP, len) == 0)
	{
		return IVUG_SET_AS_UG_TYPE_CROP;
	}

	MSG_MAIN_HIGH("Invalid setas ug type : %s", val);
	return IVUG_SET_AS_UG_TYPE_INVALID;
}

static ivug_media_type
_get_media_type(const char* val)
{
	IV_ASSERT(val != NULL);
#define IVUG_BUNDLE_VALUE_MEDIA_TYPE_ALL	"All"
#define IVUG_BUNDLE_VALUE_MEDIA_TYPE_IMAGE	"Image"
#define IVUG_BUNDLE_VALUE_MEDIA_TYPE_VIDEO	"Video"

	int len = strlen(val);

	if (strncmp(val, IVUG_BUNDLE_VALUE_MEDIA_TYPE_IMAGE, len) == 0)
	{
		return IVUG_MEDIA_TYPE_IMAGE;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_MEDIA_TYPE_VIDEO, len) == 0)
	{
		return IVUG_MEDIA_TYPE_VIDEO;
	}
	else if (strncmp(val, IVUG_BUNDLE_VALUE_MEDIA_TYPE_ALL, len) == 0)
	{
		return IVUG_MEDIA_TYPE_ALL;
	}

	MSG_MAIN_HIGH("Invalid media type : %s", val);
	return IVUG_MEDIA_TYPE_MAX;
}

static bool _data_print(app_control_h service, const char *key, void *user_data)
{
	char *value;

	char **value_array;
	int array_len = 1;
	int i;
	bool array = false;

	app_control_is_extra_data_array(service, key, &array);
	if (array == false)
	{
		app_control_get_extra_data(service, key, &value);
		MSG_HIGH("  %s : %s", key, value);
		free(value);
	}
	else
	{
		app_control_get_extra_data_array(service, key, &value_array, &array_len);
		MSG_HIGH("  %s :", key);
		for (i=0; i<array_len; i++)
		{
			MSG_HIGH(" %s", value_array[i]);
		}
		for (i=0; i<array_len; i++)
		{
			free(value_array[i]);
		}
		free(value_array);
	}

	return true;
}

static void _print_app_control_data(app_control_h service)
{
	int ret = app_control_foreach_extra_data(service, _data_print, NULL);

	if (APP_CONTROL_ERROR_NONE != ret)
	{
		MSG_HIGH("app_control_foreach_extra_data ERROR");
	}
}

static void _send_result_to_caller()
{
	int ret = 0;
	app_control_h service = NULL;
	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_HIGH("app_control_create failed");
		return;
	}

	ret = app_control_add_extra_data(service, "ivug.status", "started");
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_HIGH("app_control_add_extra_data failed");
	}
	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);

	MSG_IMAGEVIEW_HIGH("Send load started event to caller");
}

static void _on_base_deleted(void * data, Evas * e, Evas_Object * obj, void * event_info)
{
	MSG_IMAGEVIEW_ERROR("_on_base_deleted");
}

static void launch_image_viewer(app_control_h service, ug_data *ugd)
{
	MSG_IMAGEVIEW_HIGH("Image Viewer BEGIN %s, ", __func__);

	PERF_CHECK_END(LVL0, "On Create -> On Start");

	PERF_CHECK_BEGIN(LVL0, "On Start");

	if (!ugd) {
		MSG_IMAGEVIEW_ERROR("Invalid UG. Data=0x%08x", ugd);
		return ;
	}

	int ux, uy, uw, uh;

	evas_object_geometry_get(ugd->base, &ux, &uy, &uw, &uh);

	MSG_IMAGEVIEW_HIGH("Image Viewer : %s BaseGeometry(%d,%d,%d,%d)", __func__, ux, uy, uw, uh);

	if (ugd->bError == true) {
		PERF_CHECK_END(LVL0, "On Start");
		MSG_IMAGEVIEW_ERROR("UG create has ERROR");
		notification_status_message_post(GET_STR(IDS_UNABLE_TO_OPEN_FILE));
		//ug_destroy_me(gGetUGHandle());
		return;
	}

	if (ugd->main_view) {
		PERF_CHECK_BEGIN(LVL1, "main_view_start");
		ivug_main_view_start(ugd->main_view, service);
		PERF_CHECK_END(LVL1, "main_view_start");
	} else if (ugd->crop_ug) {
		ivug_crop_ug_start(ugd->crop_ug);
	}

	ivug_add_reg_idler(ugd->main_view);

	PERF_CHECK_END(LVL0, "On Start");

	if (ugd->ivug_param->bStandalone == true) {
		_send_result_to_caller();
	}
}

void ivug_set_indicator_visibility(Evas_Object *win, bool bShow)
{
	IV_ASSERT(win != NULL);

	if (bShow == true) {
		elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_OPAQUE);
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
	} else {
		elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_TRANSPARENT);
	}
}

static
void ivug_set_indicator_overlap_mode(bool bOverlap)
{
	Evas_Object *conform = gGetCurrentConformant();
	IV_ASSERT(conform != NULL);

	if (bOverlap == true) {
		elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_TRUE);
	} else {
		elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "");

		evas_object_data_set(conform, "overlap", (void *)EINA_FALSE);
	}
}

//parsing bundle
void
ivug_param_create_from_bundle(app_control_h service, void *ugdata)
{
	if (service == NULL) {
		MSG_MED("bundle value is NULL");
		return;
	}
	gSetServiceHandle(service);

	ug_data *ugd = (ug_data *)ugdata;
	ugd->ivug_param = NULL;

	//print key and value.
	MSG_HIGH("**********************************");
	_print_app_control_data(service);
	MSG_HIGH("**********************************");

	//parsing param
	ivug_parameter* data = (ivug_parameter*)calloc(1, sizeof(ivug_parameter));

	if (data == NULL) {
		MSG_HIGH("Cannot allocate memory");
		return ;
	}

// Is appsvc launch
	char* standalone = NULL;

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_STANDALONE, &standalone);
	if (standalone != NULL) {
		data->bStandalone = true;
		_ivug_free(&standalone);
	} else {
		data->bStandalone = false;
	}
// View By
	if (data->view_by == IVUG_VIEW_BY_INVAILD) {
		char* szViewBy = NULL;

		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_VIEW_BY, &szViewBy);
		if (szViewBy != NULL) {
			MSG_HIGH("View By = %s", szViewBy);

			data->view_by = _get_view_by(szViewBy);

			if (data->view_by == IVUG_VIEW_BY_TAG) {
				char* szTagName = NULL;

				app_control_get_extra_data (service, IVUG_BUNDLE_KEY_TAG_NAME, &szTagName);
				if (szTagName != NULL) {
					tag_handle t_handle = ivug_db_get_tag_handle(szTagName);
					if (t_handle == NULL) {
						MSG_HIGH("View by Tag. but tag handle is NULL");
						ivug_param_delete(data);
						return;
					}

					data->tag_id = ivug_db_get_tag_id(t_handle);

					ivug_db_destroy_tag(t_handle);

					MSG_HIGH("Tag name=%s", szTagName);
					MSG_HIGH("Tag id=%d", data->tag_id);
					_ivug_free(&szTagName);
				} else {
					MSG_HIGH("View by Tag. but Tagname is NULL");

					_ivug_free(&szViewBy);
					ivug_param_delete(data);
					return;
				}
			}
			_ivug_free(&szViewBy);
		} else {
			MSG_HIGH("View By is NULL");
		}
	}

//parse mode
	if (data->mode == IVUG_MODE_INVAILD) {
		char* szMode = NULL;

		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_VIEW_MODE, &szMode);
		if (szMode != NULL) {
			data->mode = _get_view_mode(data, szMode);
			MSG_HIGH("View mode is %s(%d)", szMode, data->mode);
			_ivug_free(&szMode);
		} else {
			data->mode = IVUG_DEFAULT_MODE;
			MSG_HIGH("View mode is NULL. Set Default(%d)", data->mode);
		}
	}
//parse path
	char* szFilePath = NULL;
	bool isArray = false;
	app_control_is_extra_data_array(service, "http://tizen.org/appcontrol/data/path", &isArray);
	if (isArray == true) {
		char **path_array = NULL;
		char *file_path = NULL;
		int array_length = 0;
		app_control_get_extra_data_array(service, "http://tizen.org/appcontrol/data/path", &path_array, &array_length);
		int i = 0;

		for (i=0; i<array_length; i++)
		{
			MSG_HIGH("[%d]File path is %s", i, path_array[i]);
			if (strncmp(IVUG_FILE_PREFIX, path_array[i], strlen(IVUG_FILE_PREFIX)) == 0) {
				file_path = strdup(path_array[i] + strlen(IVUG_FILE_PREFIX));
			} else {
				file_path = strdup(path_array[i]);
			}

			data->multiple_list = eina_list_append(data->multiple_list, (void *)file_path);
		}

		for (i=0; i<array_length; i++)
		{
			free(path_array[i]);
		}

		free(path_array);
		app_control_get_uri(service, &szFilePath);	// app gadget

		data->mode = IVUG_MODE_DISPLAY;
		data->view_by = IVUG_VIEW_BY_FILE;
	} else {
		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_PATH, &szFilePath);
		if (szFilePath == NULL) {
			MSG_MAIN_HIGH("File path is NULL");
			app_control_get_uri(service, &szFilePath);	// app gadget
			if (szFilePath) {
				MSG_MAIN_HIGH("App gadget launched %s", szFilePath);
				if (data->view_by != IVUG_VIEW_BY_FOLDER
					&& data->view_by != IVUG_VIEW_BY_HIDDEN_FOLDER
					&& data->view_by != IVUG_VIEW_BY_DIRECTORY) {
					data->view_by = IVUG_VIEW_BY_FILE;
				}
				if (data->mode == IVUG_MODE_INVAILD) {
					data->mode = IVUG_MODE_SINGLE;
				}
			}
		} else {
			if (strstr (szFilePath, "/.") != NULL) {
				data->mode = IVUG_MODE_HIDDEN;
			}
		}
	}

	if (szFilePath != NULL) {
		if (strncmp(IVUG_FILE_PREFIX, szFilePath, strlen(IVUG_FILE_PREFIX)) == 0) {
			data->filepath = strdup(szFilePath + strlen(IVUG_FILE_PREFIX));
		} else {
			data->filepath = strdup(szFilePath);
		}
		MSG_HIGH("Current File = %s", data->filepath);
		_ivug_free(&szFilePath);
	} else {
		ivug_param_delete(data);
		return;
	}

	if (data->view_by != IVUG_VIEW_BY_HIDDEN_ALL
		&& data->view_by != IVUG_VIEW_BY_HIDDEN_FOLDER
		&& data->view_by != IVUG_VIEW_BY_DIRECTORY
		&& ivug_is_web_uri(data->filepath) == false) {
		media_handle file_handle = ivug_db_get_file_handle(data->filepath);
		if (file_handle == NULL) {
			MSG_HIGH("Current File = %s is not in DB", data->filepath);
			data->view_by = IVUG_VIEW_BY_FILE;
		} else {
			MSG_HIGH("Current File = %s is in DB", data->filepath);
			ivug_db_destroy_file_handle(file_handle);
		}
	}

//parse image index at album
	char* val = NULL;

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_INDEX, &val);
	if (val) {
		data->start_index = ivug_atoi(val);
		MSG_HIGH("Slide Index = %d", data->start_index);
		_ivug_free(&val);
	} else {
		if (data->view_by == IVUG_VIEW_BY_FOLDER) {		// when by foloder mode, if index is not exist. we cannot determin what image is show. so close applicationb
			MSG_HIGH("IVUG_VIEW_BY_FOLDER but index was not set. Setas Invalid(%d)", IVUG_INVALID_INDEX);
			data->start_index = IVUG_INVALID_INDEX;
		} else {
			data->start_index = IVUG_DEFAULT_INDEX;
		}
		MSG_HIGH("Slide Index is not set. Set as default : %d", data->start_index);
	}

/* Adding sort by option */
	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_SORT_BY, &val);
	if (val) {
		if (!strcmp(val, IVUG_BUNDLE_SORT_DATE)) {
			data->sort_type = IVUG_MEDIA_ASC_BY_DATE;
		} else if (!strcmp(val, IVUG_BUNDLE_SORT_DATEDESC)) {
			data->sort_type = IVUG_MEDIA_DESC_BY_DATE;
		}
		_ivug_free(&val);
	}

	//parse album id -album id is cluster id of media service
	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_ALBUM_IDX, &val);
	if (val && data->view_by != IVUG_VIEW_BY_ALL) {
		gSetAlbumIndex(val);
		data->album_id = uuid_getuuid(val);
		MSG_HIGH("album_uuid is %s", uuid_getchar(data->album_id));
	} else {
		data->album_id = INVALID_UUID;
		MSG_HIGH("Album UUID is NULL");
	}
	if (val) {
		_ivug_free(&val);
	}

	app_control_get_extra_data (service, "TESTMODE", &val);
	if (val && strcmp(val, "TRUE") == 0) {
		data->bTestMode = true;
		_ivug_free(&val);
	}

	char* viewBy = NULL;

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_VIEW_BY, &viewBy);
	if (viewBy != NULL) {
		MSG_HIGH("View By is = %s", viewBy);
		data->view_by = _get_view_by(viewBy);
	}

	if (data->view_by == IVUG_VIEW_BY_FOLDER || data->view_by == IVUG_VIEW_BY_HIDDEN_FOLDER) {
/*
		example:
		**********************************
		  Bundle Count = 2
		   View Mode:CAMERA
		   Path:/opt/usr/media/Camera shots/IMAGE0021.jpg
		**********************************
*/
		// Get album id from file path.
		char *dir = ivug_get_directory(data->filepath);
		media_handle m_handle = ivug_db_get_folder_handle(dir);
		free(dir);

		if (data->album_id == INVALID_UUID) {
			if (m_handle == NULL) {
				MSG_HIGH("View by Folder. but media handle is NULL");
				//ivug_param_delete(data);
				//return NULL;
				data->view_by = IVUG_VIEW_BY_DIRECTORY;	// check manually
			} else {
				data->album_id = ivug_db_get_folder_id(m_handle);

				gSetAlbumIndex(uuid_getchar(data->album_id));

				MSG_HIGH("Get Album ID(%s) from file %s", uuid_getchar(data->album_id), data->filepath);
			}
		}
		if (m_handle) {
			ivug_db_destroy_folder_handle(m_handle);
		}
	}

// TODO : Check parameter integrity
	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_MEDIA_TYPE, &val);
	if (val != NULL) {
		data->media_type = _get_media_type(val);
		MSG_HIGH("Media Type=%s(%d)", val, data->media_type);
		_ivug_free(&val);
	} else {
		MSG_HIGH("Media type is not specified.");
	}

	long long int m = 0;
	long int e = 0;

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_MAX_LONGITUDE, &val);
	if (val != NULL) {
		eina_convert_atod(val, strlen(val), &m, &e);
		data->max_longitude = ldexp((double)m, e);

		MSG_HIGH("Max Longitude =%f", data->max_longitude);
		_ivug_free(&val);
	} else {
		MSG_HIGH("Max Longitude is not specified.");
	}

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_MIN_LONGITUDE, &val);
	if (val != NULL) {
		eina_convert_atod(val, strlen(val), &m, &e);
		data->min_longitude = ldexp((double)m, e);

		MSG_HIGH("Min Longitude =%f", data->min_longitude);
		_ivug_free(&val);
	} else {
		MSG_HIGH("Min Longitude is not specified.");
	}

/*
	Do not use strtod() instead of g_strtod().

	strtod() follows locale setting.

	for ex) French usr comma(,) as decimal point
*/
	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_MAX_LATITUDE, &val);
	if (val != NULL) {
		eina_convert_atod(val, strlen(val), &m, &e);
		data->max_latitude = ldexp((double)m, e);

		MSG_HIGH("Max Latitude =%f", data->max_latitude);
		_ivug_free(&val);
	} else {
		MSG_HIGH("Max Latitude is not specified.");
	}

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_MIN_LATITUDE, &val);
	if (val != NULL) {
		eina_convert_atod(val, strlen(val), &m, &e);
		data->min_latitude = ldexp((double)m, e);

		MSG_HIGH("Min Latitude =%f", data->min_latitude);
		_ivug_free(&val);
	} else {
		MSG_HIGH("Min Latitude is not specified.");
	}

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_TIMELINE_START, &val);
	if (val != NULL) {
		data->timeline_start = ivug_atoi(val);
		MSG_HIGH("time line start = %ld", data->timeline_start);
		_ivug_free(&val);
	}

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_TIMELINE_END, &val);
	if (val != NULL) {
		data->timeline_end = ivug_atoi(val);
		MSG_HIGH("time line end = %ld", data->timeline_end);
		_ivug_free(&val);
	}

	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_SETAS_TYPE, &val);
	if (val) {
		MSG_HIGH("SetAs UG Type=%s", val);
		data->setas_type = _get_setas_type(val);

		char* resolution = NULL;

		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_RESOLUTION, &resolution);
		if (resolution) {
			sscanf(resolution, "%5dx%5d", &(data->width), &(data->height));
			MSG_HIGH("Rectangle width = %d, height = %d", data->width, data->height);
			if (data->width == 0 || data->height == 0) {
				MSG_IVUG_ERROR("Resolution is invalid");
				_ivug_free(&resolution);
				_ivug_free(&val);
				ivug_param_delete(data);
				return;
			}
		} else {
			// WH(0,0) means initial scissorbox size is same as image size.
			data->width = 0;
			data->height = 0;
			MSG_HIGH("Rectangle ratio is not set. Set as default : %dx%d", data->width, data->height);
		}

		data->bRatioFix = false;
		char* bRatioFix = NULL;

		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_FIXED_RATIO, &bRatioFix);
		if (bRatioFix) {
			MSG_HIGH("Fixed ratio=%s", bRatioFix);
			if (strcmp(bRatioFix, "TRUE") == 0) {
				data->bRatioFix = true;
			}
			_ivug_free(&bRatioFix);
		}


// app-control guide.
		char* crop_mode = NULL;
		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_CROP_MODE, &crop_mode);
		if (crop_mode) {
			MSG_HIGH("Crop mode=%s", crop_mode);
			if (strcmp(crop_mode, IVUG_BUNDLE_VALUE_CROP_MODE_FIT_TO_SCREEN) == 0) {
				int lcd_x = 0;
				int lcd_y = 0;
				int lcd_w = 0;
				int lcd_h = 0;
				int rot = 0;
				evas_object_geometry_get(ugd->window, &lcd_x, &lcd_y, &lcd_w, &lcd_h);
				rot = gGetRotationDegree();
				if (rot == 90 || rot == 270) {
					int temp = lcd_w;
					lcd_w = lcd_h;
					lcd_h = temp;
				}
				data->width = lcd_w;
				data->height = lcd_h;
				data->bRatioFix = true;
			}
			_ivug_free(&crop_mode);
		}

		_ivug_free(&val);
	}

	data->footsteps = false;	/* default */
	app_control_get_extra_data (service, IVUG_BUNDLE_KEY_FOOTSTEPS, &val);
	if (val) {
		MSG_HIGH("Footsteps=%s", val);
		if (strcmp(val, "TRUE") == 0) {
			data->footsteps = true;
		}
		_ivug_free(&val);
	}

	if (data->mode == IVUG_MODE_SELECT) {
		char **index_list = NULL;
		int index_len = 0;
		char **index_list_fav = NULL;
		int index_len_fav = 0;
		char *val = NULL;
		char *limitsize = NULL;
		char *selsize = NULL;
		char *max_count = NULL;

		app_control_get_extra_data_array(service, IVUG_BUNDLE_KEY_SELECTED_INDEX, &index_list, &index_len);
		app_control_get_extra_data_array(service, IVUG_BUNDLE_KEY_SELECTED_INDEX_FAV, &index_list_fav, &index_len_fav);
		app_control_get_extra_data (service, IVUG_BUNDLE_KEY_ALBUM_IDX, &val);
		app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_SIZE, &limitsize);
		app_control_get_extra_data(service, IVUG_BUNDLE_KEY_SELECT_SIZE, &selsize);
		app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_COUNT, &max_count);

		data->select_view_limit_size = atoll(limitsize);
		data->select_view_selected_size = atoll(selsize);
		data->select_view_max_count = atoi(max_count);

		if(strcmp(val,  "GALLERY_ALBUM_FAVOURITE_ALBUMS_ID")==0){
			if (index_list_fav != NULL) {
				int i;
				for (i = 0; i < index_len_fav; i++) {
					data->selected_list = eina_list_append(data->selected_list,index_list_fav[i]);
				}

			}
		} else {
			if (index_list != NULL) {
				int i;
				for (i = 0; i < index_len; i++) {

					data->selected_list = eina_list_append(data->selected_list,index_list[i]);
				}

			}
		}

		data->total_selected = index_len + index_len_fav ;
		MSG_HIGH(" total count of images is %d",data->total_selected);
		free(index_list);

	}

	ugd->ivug_param = data;




	/*Added from on_create*/

	if (ugd->ivug_param->mode == IVUG_MODE_SETAS && ugd->ivug_param->setas_type != IVUG_SET_AS_UG_TYPE_CALLER_ID && ugd->ivug_param->setas_type != IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP) {
#ifdef TEST_FOR_CROP
		ugd->ivug_param->setas_type = IVUG_SET_AS_UG_TYPE_CROP;
		ugd->ivug_param->width = 450;
		ugd->ivug_param->height = 300;
		ugd->ivug_param->bRatioFix = true;
#endif
		MSG_HIGH("UG types=%d", ugd->ivug_param->setas_type);

		switch (ugd->ivug_param->setas_type) {
		case IVUG_SET_AS_UG_TYPE_CALLER_ID:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_CALLER_ID");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_VIDEO_CALL_ID");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_LOCKSCREEN:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_LOCKSCREEN");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER_N_LOCKSCREEN");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_WALLPAPER_CROP");
			return;
			break;

		case IVUG_SET_AS_UG_TYPE_CROP:
			MSG_HIGH("IVUG_SET_AS_UG_TYPE_CROP");
			return;
			break;
		default:
			MSG_HIGH("Invalid SetAS UG Type:%d", ugd->ivug_param->setas_type);
			return;

		}
	} else {
		PERF_CHECK_BEGIN(LVL1, "main_view_create");

		ivug_set_indicator_visibility(ugd->window, false);		// Set indicator visibility false.
		ivug_set_indicator_overlap_mode(true);				// Set comformant as no indicator mode

		ugd->main_view = ivug_main_view_create(ugd->base, ugd->ivug_param);

		PERF_CHECK_END(LVL1, "main_view_create");

		if (ugd->main_view == NULL) {	//set main view.
			MSG_ERROR("Main View Layout Loading Fail");
			ugd->bError = true;
			ugd->bErrMsg = strdup("Layout Loading Fail");
			goto ON_CREATE_ERROR;
		}
//		ugd->main_view->window = ugd->window;

		//elm_win_resize_object_add(ugd->window, ugd->main_view);//chandan

		if (ugd->ivug_param->bTestMode == true) {
			ivug_main_view_set_testmode(ugd->main_view, true);
		}

// Load list.
		PERF_CHECK_BEGIN(LVL1, "main_view_set_list");

		if (ivug_main_view_set_list(ugd->main_view, ugd->ivug_param) == false) {
			MSG_ERROR("Cannot load media list.");
			// Need popup?
			if (ugd->ivug_param->mode != IVUG_MODE_HIDDEN) {
				ugd->bError = true;
				ugd->bErrMsg = strdup(IDS_UNABLE_TO_OPEN_FILE);
				goto ON_CREATE_ERROR;
			}
		}

		Evas_Object *layout = ivug_main_view_object_get(ugd->main_view);
		elm_object_part_content_set(ugd->base, "elm.swallow.content", layout);	//swallow

		elm_win_resize_object_add(ugd->window, ugd->base);//chandan
		PERF_CHECK_END(LVL1, "main_view_set_list");
	}

	PERF_CHECK_END(LVL0, "On Create");


	PERF_CHECK_BEGIN(LVL0, "On Create -> On Start");

	evas_object_size_hint_weight_set(ugd->base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	int ux, uy, uw, uh;

	evas_object_geometry_get(ugd->base, &ux, &uy, &uw, &uh);

	MSG_HIGH("Image Viewer : %s Base(0x%08x) Geometry(%d,%d,%d,%d)", __func__, ugd->base, ux, uy, uw, uh);

	evas_object_event_callback_add(ugd->base, EVAS_CALLBACK_DEL, _on_base_deleted, ugd);

	/*Added from on_start*/
	launch_image_viewer(service, ugd);
	return;

ON_CREATE_ERROR:
	if (ugd->main_view) {
		ivug_main_view_destroy(ugd->main_view);
		ugd->main_view = NULL;
	}

	if (ugd->base == NULL) {
		ugd->base = elm_layout_add(ugd->window);
		elm_layout_theme_set(ugd->base, "layout", "application", "default");
	}

	return;
}

void
ivug_param_delete(ivug_parameter* data)
{
	IV_ASSERT(data != NULL);
	if (data->filepath)
	{
		free(data->filepath);
		data->filepath = NULL;
	}
	//eina_list_free(data->selected_list); // it freed at filter delete

	char *array = NULL;
	EINA_LIST_FREE(data->multiple_list, array)
	{
		free(array);
	}

	uuid_free(data->album_id);

	free(data);

	MSG_MAIN_HIGH("Parameter is freed.");

}


Filter_struct *ivug_param_create_filter(const ivug_parameter *param)
{
	IV_ASSERT(param != NULL);

	Filter_struct *filter_str = (Filter_struct *)calloc(1, sizeof(Filter_struct));
	IV_ASSERT(filter_str != NULL);

	media_handle handle = NULL;
	char *dir = NULL;

	if (param->multiple_list)
	{

		filter_str->type = FILTER_DIRECTORY;

		filter_str->filepath = strdup(param->filepath);
		filter_str->file_list = param->multiple_list;

		return filter_str;
	}

	if (param->view_by == IVUG_VIEW_BY_DIRECTORY)
	{
		filter_str->type = FILTER_DIRECTORY;
		filter_str->view_by = param->view_by;

		Direcotry_Filter *filter = (Direcotry_Filter *)calloc(1, sizeof(Direcotry_Filter));

		IV_ASSERT(filter != NULL);

		filter_str->dir_filter = filter;
		filter_str->dir_filter->basedir = ivug_get_directory(param->filepath);
		filter_str->dir_filter->current = strdup(param->filepath);

		return filter_str;
	}
	else
	{
		filter_str->type = FILTER_DB;
	}

	filter_str->view_by = param->view_by;
	filter_str->media_type = param->media_type;
	filter_str->sort_type = param->sort_type;
	filter_str->index = param->start_index;
	filter_str->selected_list = param->selected_list;
	if (param->filepath)
	{
		filter_str->filepath = strdup(param->filepath);
	}

	if (filter_str->type == FILTER_DB)
	{
		DB_Filter *filter = calloc(1, sizeof(DB_Filter));

		IV_ASSERT(filter != NULL);

		switch (filter_str->view_by)
		{
		case IVUG_VIEW_BY_PLACES:
			filter->place.max_longitude = param->max_longitude;
			filter->place.min_longitude = param->min_longitude;
			filter->place.max_latitude = param->max_latitude;
			filter->place.min_latitude = param->min_latitude;

			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_PLACES");
			break;

		case IVUG_VIEW_BY_TIMELINE:
			filter->time.start = param->timeline_start;
			filter->time.end = param->timeline_end;

			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_TIMELINE");
			break;

		case IVUG_VIEW_BY_TAG:
			filter->tag_id = param->tag_id;
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_TAG");
			break;
		case IVUG_VIEW_BY_FAVORITES:
			filter->album_id = INVALID_UUID;
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_FAVORITES");
			break;
		case IVUG_VIEW_BY_FILE:
			if (param->filepath != NULL) {
				filter->file_path = strdup(param->filepath);
				MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_FILE");
			}
			break;
		case IVUG_VIEW_BY_ALL:
			filter->album_id = uuid_assign(param->album_id);
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_ALL");
			break;
		case IVUG_VIEW_BY_HIDDEN_ALL:
			filter->album_id = uuid_assign(param->album_id);
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_HIDDEN_ALL");
			break;
		case IVUG_VIEW_BY_FOLDER:
			if (param->album_id == NULL)
			{
				dir = ivug_get_directory(param->filepath);
				handle = ivug_db_get_folder_handle(dir);
				filter->album_id = ivug_db_get_folder_id(handle);
				free(dir);
			}
			else
			{
				filter->album_id = uuid_assign(param->album_id);
			}
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_FOLDER");
			break;
		case IVUG_VIEW_BY_HIDDEN_FOLDER:
			if (param->album_id == NULL)
			{
				dir = ivug_get_directory(param->filepath);
				handle = ivug_db_get_folder_handle(dir);
				filter->album_id = ivug_db_get_folder_id(handle);
				free(dir);
			}
			else
			{
				filter->album_id = uuid_assign(param->album_id);
			}
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_HIDDEN_FOLDER");
			break;
		case IVUG_VIEW_BY_DIRECTORY:
			// TODO : Need more things?
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_DIRECTORY");
			break;
		case IVUG_VIEW_BY_INVAILD:
			MSG_MAIN_HIGH("param->view_by is IVUG_VIEW_BY_INVAILD");
			break;
		default:
			MSG_MAIN_HIGH("Invalid ViewBy : %d", param->view_by);
			break;

		}

		filter_str->db_filter = filter;
	}

	if (handle)
	{
		ivug_db_destroy_folder_handle(handle);
	}

	return filter_str;

}
