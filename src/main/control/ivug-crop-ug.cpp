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

#include <Elementary.h>
//#include <ui-gadget.h>
//#include <ui-gadget-module.h>		// ug_destroy_me

#include "ivug-crop-ug.h"
#include "ivug-crop-view.h"

#include "ivug-debug.h"
#include "ivug-string.h"
#include "ivug-context.h"
#include "ivug-db.h"

#include "ivug-language-mgr.h"
#include "ivug-common.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_MED

#undef LOG_CAT
#define LOG_CAT "IV-CROP-UG"



/*
Setting->Wallpaper setting(UG)
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/image9.jpg
	W/IV-COMMON( 4050): 0:00:00.003[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 4050): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Setas type : Wallpaper
	W/IV-COMMON( 4050): 0:00:00.004[F:ivug-parameter.c L:  446][HIGH] **********************************

Setting -> Lockscreen setting(UG)
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/Home_default.jpg
	W/IV-COMMON( 4050): 0:02:42.425[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 4050): 0:02:42.426[F:ivug-parameter.c L:  370][HIGH]   Setas type : Lockscreen
	W/IV-COMMON( 4050): 0:02:42.426[F:ivug-parameter.c L:  446][HIGH] **********************************

Contacts -> CallerID(UG)
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Downloads/aaa_1_1_1.jpg
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 7761): 0:00:00.010[F:ivug-parameter.c L:  370][HIGH]   Setas type : CallerID
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  370][HIGH]   Area Size : 100
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  370][HIGH]   Resolution : 480x480
	W/IV-COMMON( 7761): 0:00:00.011[F:ivug-parameter.c L:  446][HIGH] **********************************

Long Press in homescreen -> Menu -> Change wallpaper
	W/IV-COMMON( 7229): 0:00:00.012[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 7229): 0:00:00.013[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 7229): 0:00:00.013[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/image2.jpg
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Setas type : Wallpaper Crop
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Fixed ratio : TRUE
	W/IV-COMMON( 7229): 0:00:00.014[F:ivug-parameter.c L:  370][HIGH]   Resolution : 720x1280
	W/IV-COMMON( 7229): 0:00:00.015[F:ivug-parameter.c L:  446][HIGH] **********************************

Gallery -> Image Viewer -> Menu -> Print
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Images/Home_default.jpg
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Setas type : Crop
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Fixed ratio : TRUE
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  370][HIGH]   Resolution : 450x300
	W/IV-COMMON( 5951): 0:00:00.004[F:ivug-parameter.c L:  446][HIGH] **********************************

SMEMO -> Add image

	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  444][HIGH] **********************************
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   Path : /opt/usr/media/Camera/20130525-193717.jpg
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   View Mode : SETAS
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  370][HIGH]   Setas type : Crop
	W/IV-COMMON( 8138): 0:00:00.008[F:ivug-parameter.c L:  446][HIGH] **********************************

--> On Save��
	W/IV-CROP-UG( 7925): 0:00:33.557[F:ivug-crop-ug.cpp L:   94][HIGH] Bundle 1 : [crop_image_path = /opt/usr/media/Images/image2_1.jpg]
	W/IV-CROP-UG( 7925): 0:00:33.557[F:ivug-crop-ug.cpp L:   99][HIGH] Bundle 2 : [http://tizen.org/appcontrol/data/selected = /opt/usr/media/Images/image2_1.jpg]
*/


struct _IvugCropUG {

	Evas_Object *layout;
	Evas_Object *parent;

// Not using navi_bar
//	Evas_Object *navi_bar;
//	Elm_Object_Item *navi_it;

	char *filepath;

	bool bAddtoDB;
} ;

static void
_send_result(const char *key1, const char *val1, const char *key2, const char *val2)
{
	app_control_h service = NULL;
	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_ERROR("app_control_create failed");
	}

	if (key1 && val1) {
		MSG_SEC("Bundle 1 : [%s = %s]", key1, val1);
		ret = app_control_add_extra_data(service, key1, val1);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_HIGH("app_control_add_extra_data failed");
		}
	}

	if (key2 && val2) {
		MSG_SEC("Bundle 2 : [%s = %s]", key2, val2);
		ret = app_control_add_extra_data(service, key2, val2);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_HIGH("app_control_add_extra_data failed");
		}
	}

	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);
}

static void  _ivug_crop_view_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	IvugCropUG *crop_ug = (IvugCropUG *)data;

	char *path = (char *)event_info;

	evas_object_smart_callback_del(obj, "ok,clicked", _ivug_crop_view_ok_clicked_cb);

	if (crop_ug->bAddtoDB == true) {
		media_handle m_handle = NULL;

		m_handle = ivug_db_insert_file_to_DB(path);
		if (m_handle == NULL) {
			MSG_ERROR("ivug_db_insert_file_to_DB failed %s", path);
		} else {
			ivug_db_destroy_file_handle(m_handle);
		}
	}

	_send_result("crop_image_path", path, "http://tizen.org/appcontrol/data/selected", path);

	MSG_HIGH("Start destroy crop ug. bAddToDB=%d", crop_ug->bAddtoDB);

//	ivug_set_indicator_overlap_mode(false);
//	ug_destroy_me(gGetUGHandle());
	DESTROY_ME();

}

static void _ivug_setas_crop_view_ok_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	char *path = (char *)event_info;

	MSG_MAIN_HIGH("_ivug_setas_crop_view_ok_clicked_cb path = %s", path);

	evas_object_smart_callback_del(obj, "ok,clicked", _ivug_setas_crop_view_ok_clicked_cb);

	// do not insert to db

	int setas_data = (int)evas_object_data_get(obj, "setas_type");
	ivug_set_screen_type type = (ivug_set_screen_type)setas_data;

	const char* homescreen_path = IVUG_HOME_SCREEN_PATH;
	const char* lockscreen_path = IVUG_LOCK_SCREEN_PATH;

	if (type == IVUG_CTRLBAR_SET_SCREEN_HOME) {
		ivug_copy_file(path, homescreen_path);

		_send_result("homescreen_path", homescreen_path, NULL, NULL);
	} else if (type == IVUG_CTRLBAR_SET_SCREEN_LOCK) {
		ivug_copy_file(path, lockscreen_path);

		_send_result("lockscreen_path", lockscreen_path, NULL, NULL);
	} else if (type == IVUG_CTRLBAR_SET_SCREEN_BOTH) {
		ivug_copy_file(path, homescreen_path);
		ivug_config_set_homescreen_image(homescreen_path);
		ivug_copy_file(path, lockscreen_path);

		_send_result("homescreen_path", homescreen_path, "lockscreen_path", lockscreen_path);
	}

	MSG_HIGH("Start destroy ug");

//	ivug_set_indicator_overlap_mode(false);
//	ug_destroy_me(gGetUGHandle());
	DESTROY_ME();

}


static void _ivug_crop_view_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	//ivug_crop_ug_destroy((IvugCropUG *)data);

	evas_object_smart_callback_del(obj, "cancel,clicked", _ivug_crop_view_cancel_clicked_cb);

	_send_result("crop_image_path", NULL, "http://tizen.org/appcontrol/data/selected", NULL);

	MSG_HIGH("Start destroy ug.");

//	ivug_set_indicator_overlap_mode(false);
//	ug_destroy_me(gGetUGHandle());
	DESTROY_ME();

}

Evas_Object * ivug_crop_ug_get_layout(IvugCropUG * crop_ug)
{
	IV_ASSERT(crop_ug != NULL);

	return crop_ug->layout;
}

bool ivug_crop_ug_destroy(IvugCropUG * crop_ug)
{
	IV_ASSERT(crop_ug != NULL);

	if (crop_ug->filepath) {
		free(crop_ug->filepath);
		crop_ug->filepath = NULL;
	}

	crop_ug->layout = NULL;

	free(crop_ug);

	return true;
}

bool ivug_crop_ug_start(IvugCropUG * crop_ug)
{
	MSG_HIGH("ivug_crop_ug_start");
	IV_ASSERT(crop_ug != NULL);

	return true;
}

