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
#include "ivug-parameter.h"

#include "ivug-slideshow-view.h"
#include "ivug-main-view.h"

#include <device/power.h>

#include "ivug-filter.h"

#include "ivug-popup.h"

#include "ivug-language-mgr.h"


#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_MED

#undef LOG_CAT
#define LOG_CAT "IV-SLIDESHOW"


static void
_send_result(const char *key1, const char *val1, const char *key2, const char *val2)
{
	int ret = 0;
	app_control_h service = NULL;

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		MSG_SEC("app_control_create failed");
		return;
	}

	if (key1 && val1) {
		MSG_SEC("Bundle 1 : [%s = %s]", key1, val1);
		app_control_add_extra_data(service, key1, val1);
	}

	if (key2 && val2) {
		MSG_SEC("Bundle 2 : [%s = %s]", key2, val2);
		ret = app_control_add_extra_data(service, key2, val2);
		if (ret != APP_CONTROL_ERROR_NONE) {
			MSG_SEC("app_control add extra data failed");
			return;
		}
	}

	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);
}

static bool
_destory_slideshow(Ivug_SlideShowView *pSSView,
									  int state)
{
	IV_ASSERT(pSSView != NULL);
	evas_object_smart_callback_del_full(ivug_ss_object_get(pSSView->ssHandle),
										"slideshow,finished", on_slideshow_finished, pSSView);

//	ivug_allow_lcd_off();
	/* send msg to caller */
	if (state == SLIDE_SHOW_STOPPED) {
		_send_result("EXIT", "NORMAL", NULL, NULL);
	}

	/*from gallery ablum*/
	// when standalone, slideshow window have to be capture, so don't destroy here
	if (pSSView->bStandAlone == false) {
		MSG_HIGH("image viewer end cause slide show ended");
		ivug_ss_delete(pSSView->ssHandle);
		pSSView->ssHandle = NULL;
	}

	DESTROY_ME();
	return true;
}

void ivug_slideshow_view_on_mmc_state_changed(void *data)
{
	Ivug_SlideShowView *pSSView = static_cast<Ivug_SlideShowView *>(data);
	IV_ASSERT(pSSView != NULL);

	//ivug_timeout_popup_show(ivug_ss_object_get(pSSView->ssHandle), NULL, NULL, IDS_ERROR, IDS_SD_CARD_REMOVED);
	ivug_ss_set_stop(pSSView->ssHandle);
	_destory_slideshow(pSSView, SLIDE_SHOW_STOPPED);
}

void
ivug_slideshow_view_destroy(Ivug_SlideShowView *pSSView)
{
	IV_ASSERT(pSSView != NULL);

	MSG_IMAGEVIEW_HIGH("ENTER : SlideShow View Destroy. pMainView=0x%08x", pSSView);

	int ret = DEVICE_ERROR_NONE;
	ret = device_power_release_lock(POWER_LOCK_DISPLAY);

	if (ret != DEVICE_ERROR_NONE) {
		MSG_ERROR("Display Release could not be processed.");
	}

	MSG_IMAGEVIEW_HIGH("Unregister system notifications");
	if (pSSView->ssHandle) {
		MSG_IMAGEVIEW_HIGH("image viewer end cause slide show ended");
		ivug_ss_delete(pSSView->ssHandle);
		pSSView->ssHandle = NULL;
	}

	MSG_HIGH("Unregister system notifications");
	if (pSSView->layout) {
		evas_object_del(pSSView->layout);
		pSSView->layout = NULL;
	}

	if (pSSView->mList) {
		MSG_MAIN_HIGH("Remove media list");
		ivug_medialist_del(pSSView->mList);		// ivug_medialist_del() is not working on destroy cb.
		pSSView->mList = NULL;
	}

	if (pSSView->album_name) {
		free(pSSView->album_name);
		pSSView->album_name = NULL;
	}

	dump_obj(pSSView->parent, 0);
	free(pSSView);
	pSSView = NULL;
	MSG_HIGH("LEAVE : SlideShow View Destroy.");

	return ;
}


