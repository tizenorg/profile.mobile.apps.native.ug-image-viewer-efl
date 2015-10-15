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

#include "ivug-common.h"

#include "ivug-main-view.h"
#include "ivug-main-view-menu.h"
#include "ivug-main-view-priv.h"

#include "EFLUtil.h"
#include <mime_type.h>
#include <ui-gadget-module.h>		// ug_destroy_me, ug_send_result

#define MIME_TYPE_LEN			(255)
#define MIME_TYPE_3GPP			"video/3gpp"
#define MIME_TYPE_MP4			"video/mp4"
#define PATH_CAMERA_LOCAL		"/opt/usr/media/DCIM/Camera"
#define PATH_CAMERA_SDCARD		"/opt/storage/sdcard/Camera"

static void _enable_button(Evas_Object *obj)
{
	IV_ASSERT(obj != NULL);

	edje_object_signal_emit(_EDJ(obj), "image,normal", "prog");

}


static void _disable_button(Evas_Object *obj)
{
	IV_ASSERT(obj != NULL);

	edje_object_signal_emit(_EDJ(obj), "image,dim", "prog");
}


static void _on_btn_share_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("Btn Share clicked");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	_on_mainview_share(pMainView);
}


static void _on_btn_delete_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("Btn Delete clicked");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	_on_mainview_delete(pMainView);
}


static void _on_btn_save_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("Btn Save clicked");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	_on_mainview_save(pMainView);
}

static void _on_btn_nearby_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("Btn AllShare clicked");
}

static void _on_btn_edit_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MSG_MAIN_HIGH("Btn Edit clicked");

	Ivug_MainView *pMainView = (Ivug_MainView *)data;

	_on_mainview_edit(pMainView);
}

/*
	Used for only view type is select
*/
static void _on_btn_selectok_clicked(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	Ivug_MainView *pMainView = (Ivug_MainView *)data;
	IV_ASSERT(pMainView != NULL);

	MSG_MAIN_HIGH("Select OK.");

	app_control_h service = NULL;
	int retcode = app_control_create(&service);
	if (retcode != APP_CONTROL_ERROR_NONE) {
		MSG_MAIN_HIGH("app_control_create failed");
		if (service != NULL)
			app_control_destroy(service);
		return;
	}

	retcode = app_control_add_extra_data(service, "Result", "Ok");
	if (retcode != APP_CONTROL_ERROR_NONE) {
		MSG_MAIN_HIGH("app_control_add_extra_data failed");
	}
	ug_send_result(gGetUGHandle(), (service_h)service);

	app_control_destroy(service);

	DESTROY_ME();

}


/* Video editor can start when video exist in camera folder and 3gp file format */
bool ivug_is_editable_video_file(char *filepath)
{
	MSG_UTIL_MED("path = %s", filepath);

	/*if(strncmp(filepath, PATH_CAMERA_LOCAL, strlen(PATH_CAMERA_LOCAL)) == 0
		|| strncmp(filepath, PATH_CAMERA_SDCARD, strlen(PATH_CAMERA_SDCARD)) == 0)*/
	{
		int retcode = -1;

		char *type = NULL;
		char *ext = NULL;

		ext = strrchr(filepath, '.');
		if (ext != NULL) {
			ext++;
			retcode = mime_type_get_mime_type(ext, &type);
		} else {
			retcode = mime_type_get_mime_type(filepath, &type);
		}

		if ((type == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
			MSG_UTIL_ERROR("Fail to get mime type with return value [%d]", retcode);
			return false;
		}

		MSG_UTIL_MED("mime type = %s", type);

		if (strcmp(type, MIME_TYPE_MP4) == 0 || strcmp(type, MIME_TYPE_3GPP) == 0) {
			free(type);
			type = NULL;
			return true;
		}
		free(type);
		type = NULL;
	}
	return false;
}

ivug_ctrlbar
_get_menu_type(ivug_mode mode, Media_Type slide_type, const char *filepath)
{
	// this is temporary code.
	ivug_ctrlbar type = CTRL_BAR_TYPE_FILE;

	switch (mode)
	{
		case IVUG_MODE_NORMAL:
		case IVUG_MODE_CAMERA:
		case IVUG_MODE_SINGLE:
		case IVUG_MODE_FILE:
			if(ivug_is_facebook_image(filepath) == true)
			{
				type = CTRL_BAR_TYPE_READ_ONLY;
				return type;
			}
			type = CTRL_BAR_TYPE_FILE;
		break;
		case IVUG_MODE_DISPLAY:
			type = CTRL_BAR_TYPE_EMPTY;
		break;
		case IVUG_MODE_SAVE:
			type = CTRL_BAR_TYPE_SAVE;
		break;
		case IVUG_MODE_SETAS:
			type = CTRL_BAR_TYPE_EMPTY;
		break;
		default:
			MSG_MAIN_ERROR("Unhandled mode : %d", mode);
			type = CTRL_BAR_TYPE_FILE;
	}

	MSG_MAIN_MED("Mode=%d Slide=%d", mode, slide_type);

	return type;
}

static void _create_tool_menu(Ivug_MainView *pMainView)
{
	MSG_MAIN_HIGH("Create sub menu. pMainView(0x%08x)", pMainView);

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if(mdata == NULL)
	{
		MSG_MAIN_ERROR("mdata is NULL");
		return;
	}

	if(pMainView->mode == IVUG_MODE_DISPLAY)
	{
// Display only mode does not need Toolbar
		return;
	}

	if(pMainView->mode == IVUG_MODE_SAVE)
	{
		Evas_Object *btnSave;

		btnSave = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.save");
		elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn1", btnSave); //swallow
		pMainView->toolbtns.download = btnSave;

		elm_layout_signal_callback_add(btnSave, "image_click", "ivug.btn.save", _on_btn_selectok_clicked, pMainView);
	}
	else if(pMainView->view_by == IVUG_VIEW_BY_HIDDEN_ALL || pMainView->view_by == IVUG_VIEW_BY_HIDDEN_FOLDER)
	{
		Evas_Object *btnDelete;

		btnDelete = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.delete");
		elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn1", btnDelete); //swallow
		pMainView->toolbtns.del = btnDelete;

		elm_layout_signal_callback_add(btnDelete, "image_click", "ivug.btn.delete", _on_btn_delete_cb, pMainView);

		Evas_Object *btnShare;
		btnShare = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.share");
		elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn2", btnShare); //swallow
		pMainView->toolbtns.share = btnShare;

		elm_layout_signal_callback_add(btnShare, "image_click", "ivug.btn.share", _on_btn_share_cb, pMainView);

		Evas_Object *btnNearBy;

		btnNearBy = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.nearby");
		elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn3", btnNearBy); //swallow
		pMainView->toolbtns.nearby = btnNearBy;

		elm_layout_signal_callback_add(btnNearBy, "image_click", "ivug.btn.nearby", _on_btn_nearby_cb, pMainView);
	}
	else
	{
		Ivug_DB_Stroge_Type storage_type = IV_STORAGE_TYPE_INTERNAL;
		ivug_db_get_file_storage_type(mdata->m_handle, &storage_type);
		if(storage_type != IV_STORAGE_TYPE_WEB)		// not Facebook image
		{
			Evas_Object *btnDelete;

			btnDelete = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.delete");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn1", btnDelete); //swallow
			pMainView->toolbtns.del = btnDelete;

			elm_layout_signal_callback_add(btnDelete, "image_click", "ivug.btn.delete", _on_btn_delete_cb, pMainView);

			Evas_Object *btnEdit;

			btnEdit = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.edit");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn2", btnEdit); //swallow
			pMainView->toolbtns.edit = btnEdit;

			elm_layout_signal_callback_add(btnEdit, "image_click", "ivug.btn.edit", _on_btn_edit_cb, pMainView);

			Evas_Object *btnShare;
			btnShare = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.share");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn3", btnShare); //swallow
			pMainView->toolbtns.share = btnShare;

			elm_layout_signal_callback_add(btnShare, "image_click", "ivug.btn.share", _on_btn_share_cb, pMainView);

			Evas_Object *btnNearBy;

			btnNearBy = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.nearby");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn4", btnNearBy); //swallow
			pMainView->toolbtns.nearby = btnNearBy;

			elm_layout_signal_callback_add(btnNearBy, "image_click", "ivug.btn.nearby", _on_btn_nearby_cb, pMainView);

		}
		else	// Facebook image
		{
			Evas_Object *btnShare;

			btnShare = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.share");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn1", btnShare); //swallow
			pMainView->toolbtns.share = btnShare;

			elm_layout_signal_callback_add(btnShare, "image_click", "ivug.btn.share", _on_btn_share_cb, pMainView);

			Evas_Object *btnSave;

			btnSave = EFL::create_layout(pMainView->lyContent, EDJ_PATH"/ivug-button.edj", "ivug.btn.save");
			elm_object_part_content_set(pMainView->lyContent, "ivug.swallow.btn2", btnSave); //swallow
			pMainView->toolbtns.download = btnSave;

			elm_layout_signal_callback_add(btnSave, "image_click", "ivug.btn.save", _on_btn_save_cb, pMainView);
		}
	}
}

static void _delete_tool_menu(Ivug_MainView *pMainView)
{
	if (pMainView->toolbtns.nearby )
	{
		evas_object_del(pMainView->toolbtns.nearby);
		pMainView->toolbtns.nearby = NULL;
	}

	if (pMainView->toolbtns.more )
	{
		evas_object_del(pMainView->toolbtns.more);
		pMainView->toolbtns.more = NULL;
	}

	if (pMainView->toolbtns.share )
	{
		evas_object_del(pMainView->toolbtns.share);
		pMainView->toolbtns.share = NULL;
	}

	if (pMainView->toolbtns.download )
	{
		evas_object_del(pMainView->toolbtns.download);
		pMainView->toolbtns.download = NULL;
	}

	if (pMainView->toolbtns.del )
	{
		evas_object_del(pMainView->toolbtns.del);
		pMainView->toolbtns.del = NULL;
	}

}

void _update_toolbuttons(Ivug_MainView *pMainView)
{

/*
   |------------------|
   |elm_navigation_bar|
   |------------------|
   |				  |
   |				  |
   |				  |
   |				  |
   |------------------|
*/

	Media_Item *mitem = ivug_medialist_get_current_item(pMainView->mList);
	Media_Data *mdata = ivug_medialist_get_data(mitem);
	if(mdata == NULL)
	{
		MSG_MAIN_ERROR("mdata is NULL");
		return;
	}

	MSG_MAIN_HIGH("Update toolbutton. mdata=0x%08x", mdata);

	if(pMainView->mode == IVUG_MODE_DISPLAY)
	{
		return;
	}

	ivug_ctrlbar ctrl_bar_type = _get_menu_type(pMainView->mode, mdata->slide_type, mdata->filepath);

	if (pMainView->ctrl_bar_type == CTRL_BAR_TYPE_UNDEFINED )
	{
		MSG_MAIN_HIGH("Creating ctrlbar");
		pMainView->ctrl_bar_type = ctrl_bar_type;
		_create_tool_menu(pMainView);
	}
	else
	{
		if(pMainView->ctrl_bar_type != ctrl_bar_type)
		{
			MSG_MAIN_HIGH("Ctrl bar type changed. %d->%d", pMainView->ctrl_bar_type, ctrl_bar_type);
			pMainView->ctrl_bar_type = ctrl_bar_type;
			_delete_tool_menu(pMainView);
			_create_tool_menu(pMainView);
		}
	}

	Data_State state = ivug_mediadata_get_file_state(mdata);
	if(state == DATA_STATE_ERROR
		|| state == DATA_STATE_NO_PERMISSION)
	{
		MSG_MAIN_HIGH("slide state is invalid %d", state);
		if(pMainView->toolbtns.share)
		{
			_disable_button(pMainView->toolbtns.share);
		}

		if(pMainView->toolbtns.download)
		{
			_disable_button(pMainView->toolbtns.download);
		}

		return;
	}
	//else
	{
		if(state == DATA_STATE_LOADED)
		{
			if(pMainView->toolbtns.share)
			{
				_enable_button(pMainView->toolbtns.share);
			}

			if(pMainView->toolbtns.download)
			{
				_enable_button(pMainView->toolbtns.download);
			}


		}
	}
}

