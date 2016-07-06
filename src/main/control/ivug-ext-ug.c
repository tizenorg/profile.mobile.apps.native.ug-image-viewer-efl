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

#include <app.h>
#include <Elementary.h>

#include "ivug-ext-ug.h"
#include "ivug-util.h"
#include "ivug-debug.h"
#include "ivug-db.h"
#include "ivug-context.h"

/*
	ug itself has responsibility to set indicator status. so I undefined this feature
*/
#undef USE_INDICATOR_STATE_SET

//definition
#define MESSAGE_UG_NAME 		"msg-composer-efl"
#define S_NOTE_UG_NAME 		    "smemo-efl"
#define EMAIL_UG_NAME 			"email-composer-efl"
#define CONTACT_UG_NAME 		"contacts-list-efl"
#define MYFILE_DETAIL_UG_NAME	"myfile-detail-efl"
#define BLUETOOTH_UG_NAME 		"setting-bluetooth-efl"
#define MYFILE_UG_NAME			"myfile-efl"
#define GALLERY_UG_NAME 		"gallery-efl"

#define CONTACT_SELECT_UG_NAME	"contacts-tabui-efl"
#define CONTACT_EDIT_UG_NAME	"contacts-details-efl"
#define GL_UG_PKG_TAGBUDDY_SETTING	"setting-tagbuddy-efl"


#define VIDEOPLAYER_PKG_NAME 	"com.samsung.video-player"
#define BLUETOOTH_PKG_NAME 		"com.samsung.bluetooth"
#define BROWSER_PKG_NAME 		"com.samsung.browser"
#define MESSAGE_PKG_NAME 		"com.samsung.message"
#define EMAIL_PKG_NAME 			"com.samsung.email"
#define FACEBOOK_PKG_NAME 		"com.samsung.facebook"
#define IMAGE_EDITOR_PKG_NAME 	"com.samsung.image-editor"
#define IMAGE_VIEWER_PKG_NAME 	"org.tizen.image-viewer"

#define WIFI_FILE_TRANSFER_UG_NAME	"fileshare-efl"

#define PRINT_PKG_NAME "com.samsung.mobileprint"

#define ALLSHARE_CAST_APP_NAME		"com.samsung.allshare-cast-popup"

#define OPERATION_NAME_PRINT "http://tizen.org/appcontrol/operation/print"

#define APP_CONTROL_PRINT_FILES_TYPE "app_control_print_files_type"

#define MIME_TYPE_LEN			255

#define FILE_PREFIX		"file://"

void ivug_ext_app_control_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	MSG_IMAGEVIEW_HIGH("ivug_ext_app_control_reply_cb");
	switch (result)
	{
		case APP_CONTROL_RESULT_SUCCEEDED:
			MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_SUCCEEDED");
			break;
		case APP_CONTROL_RESULT_FAILED:
			MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_FAILED");
			break;
		case APP_CONTROL_RESULT_CANCELED:
			MSG_IMAGEVIEW_HIGH("APP_CONTROL_RESULT_CANCELED");
			break;
		default:
			MSG_IMAGEVIEW_ERROR("unhandled value %d", result);
			break;
	}
}

bool ivug_ext_launch_videoeditor(const char *uri, app_control_reply_cb callback, void *data)
{
/*
	Replay callback is not called when exit videoeditor.
	Callee app should call app_control_reply_request(). then Caller app can get reply callback.
*/
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	ret = app_control_add_extra_data(handle, "path", uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	ret = app_control_add_extra_data(handle, "launching_application", "image_viewer");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	ret = app_control_add_extra_data(handle, "edit_mode", "trim");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	const char *pkgname = VIDEOPLAYER_PKG_NAME;

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

	if (callback == NULL)
	{
		callback = ivug_ext_app_control_reply_cb;
		data = NULL;
	}

	ret = app_control_send_launch_request(handle, callback, data);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto VIDEO_EDITOR_END;
	}

VIDEO_EDITOR_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

#ifdef IV_EXTENDED_FEATURES
bool  ivug_ext_launch_email(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, "http://tizen.org/appcontrol/operation/compose");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto EMAIL_END;
	}

	ret = app_control_set_uri(handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto EMAIL_END;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	ret = app_control_set_window(handle, win_id);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto EMAIL_END;
	}

	ret = app_control_add_extra_data(handle, "RUN_TYPE", "5");
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
			goto EMAIL_END;
		}

	if (ivug_is_web_uri(uri) == true)
	{
		ret = app_control_add_extra_data(handle, "BODY", uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
			goto EMAIL_END;
		}
	}
	else
	{
		ret = app_control_add_extra_data(handle, "ATTACHMENT", uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
			goto EMAIL_END;
		}
	}

	const char *pkgname = EMAIL_UG_NAME;

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_app_id failed, %d", ret);
		goto EMAIL_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto EMAIL_END;
	}

EMAIL_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}
#endif

bool ivug_ext_launch_videoplayer(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, 0x%08x", ret);
		return false;
	}

	ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, 0x%08x", ret);
		goto VIDEO_PLAYER_END;
	}

	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, 0x%08x", ret);
		goto VIDEO_PLAYER_END;
	}

	ret = app_control_set_launch_mode(handle, APP_CONTROL_LAUNCH_MODE_GROUP);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control GROUP_MODE Failed");
	}

	ret = app_control_add_extra_data(handle, "launching_application", "image_viewer");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, 0x%08x", ret);
		goto VIDEO_PLAYER_END;
	}

	//do not set package
	/*const char *pkgname = VIDEOPLAYER_PKG_NAME;

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto VIDEO_PLAYER_END;
	}*/

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, 0x%08x", ret);
		goto VIDEO_PLAYER_END;
	}

VIDEO_PLAYER_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, 0x%08x", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

bool ivug_ext_launch_videoplayer_simple(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

	ret = app_control_add_extra_data(handle, "path", uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

	// Camera -> Image Viewer -> Video player, In this case, launching_application bundle's value should be "light_play_view"
	ret = app_control_add_extra_data(handle, "launching_application", "light_play_view");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

	const char *pkgname = VIDEOPLAYER_PKG_NAME;

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto VIDEO_PLAYER_SIMPLE_END;
	}

VIDEO_PLAYER_SIMPLE_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

bool ivug_ext_launch_twitter(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	if (ivug_is_web_uri(uri) == true)
	{
		ret = app_control_set_operation(handle, "http://tizen.org/appcotnrol/operation/share_text");
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
			goto TWITTER_END;
		}
		ret = app_control_add_extra_data(handle, APP_CONTROL_DATA_TEXT, uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
			goto TWITTER_END;
		}
	}
	else
	{
		ret = app_control_set_operation(handle, "http://tizen.org/appcotnrol/operation/share");
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
			goto TWITTER_END;
		}
		ret = app_control_set_uri (handle, uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
			goto TWITTER_END;
		}
	}

	const char *pkgname = "com.samsung.twitter";

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto TWITTER_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto TWITTER_END;
	}

TWITTER_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

bool ivug_ext_launch_facebook(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	if (ivug_is_web_uri(uri) == true)
	{
		ret = app_control_set_operation(handle, "http://tizen.org/appcotnrol/operation/share_text");
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
			goto FACEBOOK_END;
		}
		ret = app_control_add_extra_data(handle, APP_CONTROL_DATA_TEXT, uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
			goto FACEBOOK_END;
		}
	}
	else
	{
		ret = app_control_set_operation(handle, "http://tizen.org/appcotnrol/operation/share");
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
			goto FACEBOOK_END;
		}
		ret = app_control_set_uri (handle, uri);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
			goto FACEBOOK_END;
		}
		media_handle file_handle = ivug_db_get_file_handle(uri);
		if (file_handle)
		{
			char *mime = ivug_db_get_mime_type(file_handle);
			ivug_db_destroy_file_handle(file_handle);
			if (mime)
			{
				ret = app_control_set_mime(handle, mime);
				free(mime);
				if (ret != APP_CONTROL_ERROR_NONE)
				{
					MSG_IMAGEVIEW_ERROR("app_control_set_mime failed, %d", ret);
					goto FACEBOOK_END;
				}
			}
		}
	}

	const char *pkgname = "com.samsung.facebook";

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto FACEBOOK_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto FACEBOOK_END;
	}

FACEBOOK_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

bool ivug_ext_launch_picasa(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;
	char xwin_id_str[12] = {0,};

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, "http://tizen.org/appcotnrol/operation/share");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto PICASA_END;
	}
	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto PICASA_END;
	}

	const char *pkgname = "com.samsung.picasa";

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto PICASA_END;
	}

//	Ecore_X_Window xwin_id = elm_win_xwindow_get(ug_get_window()); [ToDo] Check Appropriate replacement
	Ecore_X_Window xwin_id = -1;
	eina_convert_itoa(xwin_id, xwin_id_str);
	ret = app_control_add_extra_data(handle, "XWINDOW_ID", xwin_id_str);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto PICASA_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto PICASA_END;
	}

PICASA_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

#ifdef IV_EXTENDED_FEATURES
bool ivug_ext_launch_sns(const char *pkgname, const char *uri)
{
	if (strcmp(pkgname, "com.samsung.twitter") == 0)
	{
		return ivug_ext_launch_twitter(uri);
	}
	else if (strcmp(pkgname, "com.samsung.facebook") == 0)
	{
		return ivug_ext_launch_facebook(uri);
	}
	else if (strcmp(pkgname, "com.samsung.picasa") == 0)
	{
		return ivug_ext_launch_picasa(uri);
	}
	else if (strcmp(pkgname, "com.samsung.youtube") == 0)
	{
		return ivug_ext_launch_default(uri, "http://tizen.org/appcotnrol/operation/share", pkgname, NULL, NULL);
	}
	else
	{
		MSG_IMAGEVIEW_ERROR("Unknown package name:%s", pkgname);
	}

	return true;
}
#endif

bool ivug_ext_launch_browser(const char *uri)
{
	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto BROWSER_END;
	}

	ret = app_control_add_extra_data(handle, "url", uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto BROWSER_END;
	}

	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto BROWSER_END;
	}

	const char *pkgname = BROWSER_PKG_NAME;

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto BROWSER_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto BROWSER_END;
	}

BROWSER_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

bool ivug_ext_launch_print(const char *uri)
{
#define APP_CONTROL_PRINT_TITLE			"Title"
#define APP_CONTROL_PRINT_CONTENT		"ContentPath"
#define APP_CONTROL_PRINT_CONTENT_TYPE	"ContentType"
#define APP_CONTROL_PRINT_CONTENT_COUNT	"ContentCount"

	MSG_IMAGEVIEW_HIGH("%s. URI=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;
	const char *pkgname = PRINT_PKG_NAME;

	if (uri == NULL)
	{
		MSG_IMAGEVIEW_ERROR("URI is NULL");
		return false;
	}

	app_control_h handle;
	const char **files = NULL;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, OPERATION_NAME_PRINT);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto PRINT_END;
	}

	ret = app_control_set_app_id(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_app_id failed, %d", ret);
		goto PRINT_END;
	}

	/*ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto PRINT_END;
	}*/

	ret = app_control_add_extra_data(handle, APP_CONTROL_PRINT_TITLE, "From Gallery");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto PRINT_END;
	}

	files = (const char **)malloc(sizeof(char *));
	if (files == NULL)
	{
		MSG_IMAGEVIEW_ERROR("malloc has error");
		app_control_destroy(handle);
		return false;
	}
	files[0] = uri;

	ret = app_control_add_extra_data_array(handle, APP_CONTROL_PRINT_CONTENT, (const char **)files, 1);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto PRINT_END;
	}

	ret = app_control_add_extra_data(handle, APP_CONTROL_PRINT_CONTENT_COUNT, "1");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto PRINT_END;
	}

	ret = app_control_add_extra_data(handle, APP_CONTROL_PRINT_CONTENT_TYPE, "IMG");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto PRINT_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto PRINT_END;
	}

PRINT_END:
	if (files)
		free(files);

	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}
#ifdef USE_EXT_SLIDESHOW
ui_gadget_h  ivug_ext_launch_select_music(ug_result_cb result_func, ug_destroy_cb destroy_func, void *data)
{
	MSG_IMAGEVIEW_HIGH("%s", __func__);

	const char *pa_cur_ringtone = NULL;
	const char *dir_path = NULL;

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;
	ui_gadget_h ug = NULL;

	pa_cur_ringtone = "/opt/share/settings/Ringtones/Over the horizon.mp3";
	dir_path = "/opt/share/settings/Ringtones/";

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return NULL;
	}

	/*ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, %d", ret);
		goto MYFILE_END;
	}*/

	ret = app_control_add_extra_data(handle, "marked_mode", pa_cur_ringtone);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto MYFILE_END;
	}

	ret = app_control_add_extra_data(handle, "path", dir_path);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto MYFILE_END;
	}

	ret = app_control_add_extra_data(handle, "select_type", "SINGLE_FILE");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto MYFILE_END;
	}

	ret = app_control_add_extra_data(handle, "drm_type", "DRM_ALL");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto MYFILE_END;
	}

	/*const char *pkgname = MYFILE_UG_NAME;

	ret = app_control_set_package(handle, pkgname);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto MYFILE_END;
	}*/

	ug = _ivug_ext_launch_ug_with_result(MYFILE_UG_NAME, handle, result_func, destroy_func, data);

MYFILE_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return NULL;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? ug : NULL);
}
#endif

#ifdef IV_EXTENDED_FEATURES
app_control_h ivug_ext_launch_image_editor(const char *uri, app_control_reply_cb callback, void *data)
{
	MSG_IMAGEVIEW_HIGH("%s, uri=%s", __func__, uri);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	ret = app_control_set_window(handle, win_id);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto IE_END;
	}

	ret = app_control_set_app_id(handle, IMAGE_EDITOR_PKG_NAME);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_app_id failed, %d", ret);
		goto IE_END;
	}

	ret = app_control_set_uri (handle, uri);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, %d", ret);
		goto IE_END;
	}

	ret = app_control_send_launch_request(handle, callback, data);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto IE_END;
	}

	return handle;

IE_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return NULL;
}


bool ivug_ext_launch_default(const char *uri, const char *operation, const char *pkg, const char *mime, void *data)
{
// Share Panel!

	MSG_IMAGEVIEW_SEC("operation = %s, uri = %s, mime = %s", operation, uri, mime);

	int ret = -1;
	int destroy_ret = -1;

	char buf[IVUG_MAX_FILE_PATH_LEN] = {0, };

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	ret = app_control_set_operation(handle, operation);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation %s failed, %d", operation, ret);
		goto LAUNCH_END;
	}

	snprintf(buf, (size_t)sizeof(buf), "%s", uri);
	if (ivug_is_web_uri(uri) == false && ivug_is_tel_uri(uri) == false)
	{
		if (strncmp(uri, FILE_PREFIX, strlen(FILE_PREFIX)) != 0)
		{
			snprintf(buf, (size_t)sizeof(buf), "%s%s", FILE_PREFIX, uri);
			MSG_IMAGEVIEW_SEC("changed uri = %s", buf);
		}
	}

	ret = app_control_set_uri (handle, buf);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri %s failed, %d", uri, ret);
		goto LAUNCH_END;
	}

	if (pkg)
	{
		ret = app_control_set_app_id(handle, pkg);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_app_id %s failed, %d", pkg, ret);
			goto LAUNCH_END;
		}
	}
	else
	{
		Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
		ret = app_control_set_window(handle, win_id);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_window failed, %d", ret);
			goto LAUNCH_END;
		}
	}

	if (mime)
	{
		ret = app_control_set_mime(handle, mime);
		if (ret != APP_CONTROL_ERROR_NONE)
		{
			MSG_IMAGEVIEW_ERROR("app_control_set_mime failed, %d", ret);
			goto LAUNCH_END;
		}
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto LAUNCH_END;
	}

LAUNCH_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}
#endif


#ifdef IV_EXTENDED_FEATURES
bool ivug_ext_launch_allshare_cast(const char *szMacAddr, void *pUserData)
{
	app_control_h pService = NULL;

	int nRet = 0;
	nRet  = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_create is fail [0x%x]", nRet);
		goto Execption;
	}

	MSG_IMAGEVIEW_ERROR("Mac Addr : %s", szMacAddr);

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	nRet = app_control_set_window(pService, win_id);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_set_window is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_add_extra_data(pService,"-t","connection_req");
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_add_extra_data(pService,"mac",szMacAddr);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_set_app_id(pService, ALLSHARE_CAST_APP_NAME);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_set_app_id is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_send_launch_request(pService, ivug_ext_app_control_reply_cb, pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request is fail [0x%x]", nRet);
		goto Execption;
	}

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

Execption:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;

}

bool ivug_ext_launch_map(double latitude, double longitude, void *pUserData)
{
	app_control_h pService = NULL;

	char buf[IVUG_MAX_FILE_PATH_LEN] = {0, };

	int nRet = 0;
	nRet  = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_create is fail [0x%x]", nRet);
		goto MAP_END;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	nRet = app_control_set_window(pService, win_id);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_set_window is fail [0x%x]", nRet);
		goto MAP_END;
	}

	nRet = app_control_set_operation(pService, "http://tizen.org/appcontrol/operation/view");
	if (nRet != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, [0x%x]", nRet);
		goto MAP_END;
	}

	snprintf(buf, (size_t)sizeof(buf), "geo:%lf,%lf", latitude, longitude);
	MSG_IMAGEVIEW_HIGH("app_control_set_uri %s", buf);

	nRet = app_control_set_uri(pService, buf);
	if (nRet != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, [0x%x]", nRet);
		goto MAP_END;
	}

	nRet = app_control_send_launch_request(pService, ivug_ext_app_control_reply_cb, pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request is fail [0x%x]", nRet);
		goto MAP_END;
	}

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

MAP_END:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;

}


bool ivug_ext_launch_map_direction(double d_latitude, double d_longitude, void *pUserData)
{
	app_control_h pService = NULL;

	char buf[IVUG_MAX_FILE_PATH_LEN] = {0, };

	int nRet = 0;
	nRet  = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_create is fail [0x%x]", nRet);
		goto MAP_END;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	nRet = app_control_set_window(pService, win_id);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_set_window is fail [0x%x]", nRet);
		goto MAP_END;
	}

	nRet = app_control_set_operation(pService, "http://tizen.org/appcontrol/operation/search_directions");
	if (nRet != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_operation failed, [0x%x]", nRet);
		goto MAP_END;
	}

	snprintf(buf, (size_t)sizeof(buf), "geo:0,0?dest_addr=%lf:%lf", d_latitude, d_longitude);
	MSG_IMAGEVIEW_HIGH("app_control_set_uri %s", buf);

	nRet = app_control_set_uri(pService, buf);
	if (nRet != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, [0x%x]", nRet);
		goto MAP_END;
	}

	nRet = app_control_send_launch_request(pService, ivug_ext_app_control_reply_cb, pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request is fail [0x%x]", nRet);
		goto MAP_END;
	}

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

MAP_END:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;

}

bool ivug_ext_launch_share_text(const char *text)
{
	MSG_IMAGEVIEW_HIGH("%s, text=%s", __func__, text);

	int ret = -1;
	int destroy_ret = -1;

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	ret = app_control_set_window(handle, win_id);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto SHARE_TEXT_END;
	}

	ret = app_control_set_operation(handle, "http://tizen.org/appcontrol/operation/share_text");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		goto SHARE_TEXT_END;
	}

	ret = app_control_add_extra_data(handle, APP_CONTROL_DATA_TEXT, text);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto SHARE_TEXT_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto SHARE_TEXT_END;
	}

SHARE_TEXT_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
}

static bool _data_set(app_control_h service, const char *key, void *user_data)
{
	char *value;

	app_control_h handle = (app_control_h)user_data;

	app_control_get_extra_data(service, key, &value);
	MSG_IVUG_HIGH("  %s : %s", key, value);

	int ret = app_control_add_extra_data(handle, key, value);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
	}

	free(value);

	return true;
}
#endif

bool ivug_ext_launch_slideshow(const char *path, int index)
{
#ifndef USE_THUMBLIST
	MSG_IMAGEVIEW_HIGH("%s, path=%s, index=%d", __func__, path, index);

	int ret = -1;
	int destroy_ret = -1;
	char str_index[12] = {0,};

	app_control_h handle;

	ret = app_control_create(&handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		return false;
	}

	Ecore_X_Window win_id = elm_win_xwindow_get(ug_get_window());
	ret = app_control_set_window(handle, win_id);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_create failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_foreach_extra_data(handle, _data_set, handle);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_foreach_extra_data failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_add_extra_data(handle, "View Mode", "SLIDESHOW");
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_set_uri(handle, path);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_uri failed, [0x%x]", ret);
		goto EXT_SLIDESHOW_END;
	}

	eina_convert_itoa(index, str_index);
	ret = app_control_add_extra_data(handle, "Index", str_index);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_add_extra_data failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_set_app_id(handle, IMAGE_VIEWER_PKG_NAME);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_set_app_id failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

	ret = app_control_send_launch_request(handle, ivug_ext_app_control_reply_cb, NULL);
	if (ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_send_launch_request failed, %d", ret);
		goto EXT_SLIDESHOW_END;
	}

EXT_SLIDESHOW_END:
	destroy_ret = app_control_destroy(handle);
	if (destroy_ret != APP_CONTROL_ERROR_NONE)
	{
		MSG_IMAGEVIEW_ERROR("app_control_destroy failed, %d", destroy_ret);
		return false;
	}

	return (ret == APP_CONTROL_ERROR_NONE ? true : false);
#else
	return false;
#endif
}

