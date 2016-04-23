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

//#include <ui-gadget-module.h>
#include <device/power.h>

#include "ivug-filter.h"

#include "ivug-popup.h"

#include "ivug-language-mgr.h"

//#include "EFLUtil.h"

#undef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_MED

#undef LOG_CAT
#define LOG_CAT "IV-SLIDESHOW"



static void
_on_slideshow_finished(void *data, Evas_Object *obj, void *event_info);

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
		app_control_add_extra_data(service, key2, val2);
	}

	app_control_reply_to_launch_request(service, gGetServiceHandle(), APP_CONTROL_RESULT_SUCCEEDED);

	app_control_destroy(service);
}

static bool _destory_slideshow_and_ug(Ivug_SlideShowView *pSSView,
                                      int state,
                                      bool bMmc_out)
{
	IV_ASSERT(pSSView != NULL);
	evas_object_smart_callback_del_full(ivug_ss_object_get(pSSView->ssHandle),
	                                    "slideshow,finished", _on_slideshow_finished, pSSView);

//	ivug_allow_lcd_off();
	/* send msg to caller */
	if (state == SLIDE_SHOW_STOPPED) {
		_send_result("EXIT", "NORMAL", NULL, NULL);
	}

	/*from gallery ablum*/
	// when standalone, slideshow window have to be capture, so don't destroy here
	if (pSSView->bStandAlone == false) {
		if (pSSView->ssHandle) {
			MSG_HIGH("image viewer end cause slide show ended");
			ivug_ss_delete(pSSView->ssHandle);
		}
		pSSView->ssHandle = NULL;
	}

	DESTROY_ME();
	return true;
}

static void
_on_slideshow_finished(void *data, Evas_Object *obj, void *event_info)
{
	MSG_HIGH("_on_slideshow_finished");
	IV_ASSERT(data != NULL);

	Ivug_SlideShowView *pSSView = static_cast<Ivug_SlideShowView *>(data);

	int ss_state = (int)event_info;
	_destory_slideshow_and_ug(pSSView, ss_state, false);
}

void _ivug_slideshow_view_on_mmc_state_changed(void *data)
{
	Ivug_SlideShowView *pSSView = static_cast<Ivug_SlideShowView *>(data);
	IV_ASSERT(pSSView != NULL);

	//ivug_timeout_popup_show(ivug_ss_object_get(pSSView->ssHandle), NULL, NULL, IDS_ERROR, IDS_SD_CARD_REMOVED);
	ivug_ss_set_stop(pSSView->ssHandle);
	_destory_slideshow_and_ug(pSSView, SLIDE_SHOW_STOPPED, true);
}
/*
__attribute__((used)) void dump_obj(Evas_Object *obj, int lvl)
{
	Eina_List *list = evas_object_smart_members_get(obj);

	if (lvl == 0) {
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(obj, &mW, &mH);
		evas_object_size_hint_max_get(obj, &MW, &MH);

		MSG_HIGH("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);
		lvl++;
	}

	Evas_Object *data;
	Eina_List *l;

	for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l)) {
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(data, &mW, &mH);
		evas_object_size_hint_max_get(data, &MW, &MH);

		char *space = new char[lvl * 2 + 1];

		for (int i = 0; i < lvl * 2; i++) {
			space[i] = ' ';
		}

		space[lvl * 2] = '\0';

		MSG_HIGH("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", space, evas_object_name_get(data), evas_object_type_get(data), data, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);

		delete[] space;

		dump_obj(data, lvl + 1);

	}
}
*/
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


