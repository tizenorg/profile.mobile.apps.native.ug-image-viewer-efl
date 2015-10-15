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

#include <Evas.h>
#include <ui-gadget-module.h>	// ug_send_result

#include "ivug-image.h"
#include "ivug-string.h"
#include "ivug-debug.h"
#include "ivug-popup.h"

#include "ivug-buddy-view.h"

#include "ivug-language-mgr.h"

#define LOG_LVL DBG_MSG_LVL_HIGH
#define LOG_CAT "IV-BUDDY"

#define EDJ_PATH PREFIX"/res/edje/"PACKAGE
#define BUDDY_VIEW_EDJ_FILE EDJ_PATH"/ivug-buddy-view.edj"

#define IVUG_RESULT_BUNDLE_KEY_ERROR			"Error"
#define IVUG_RESULT_BUNDLE_VALUE_NOT_SUPPORTED	"not_supported_file_type"

typedef enum {
	BUDDY_ERROR_TYPE_NONE,
	BUDDY_ERROR_TYPE_UNKNOWN_FORMAT,
	BUDDY_ERROR_TYPE_PERMISSION_DENIED,
	BUDDY_ERROR_TYPE_INVALID_FILE,
	BUDDY_ERROR_TYPE_GENERAL,
} Buddy_Error;


static void
_on_photocam_loaded(void *data, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_BuddyView *pBuddyView = (Ivug_BuddyView *)data;
	MSG_HIGH("Photocam preloaded");

	Evas_Load_Error error = (int)event_info;

	if (error != EVAS_LOAD_ERROR_NONE)
	{
		MSG_ERROR("Error occured during decoding. Error=%d", error);

		if (error == EVAS_LOAD_ERROR_UNKNOWN_FORMAT)
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_UNKNOWN_FORMAT);
		}
		else if (error == EVAS_LOAD_ERROR_PERMISSION_DENIED)
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_PERMISSION_DENIED);
		}
		else
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_INVALID_FILE);
		}

		return;
	}

	MSG_HIGH("Photocam loaded.");
	//elm_object_disabled_set(pBuddyView->btn_ok, EINA_FALSE);

	//evas_object_smart_callback_call(pBuddyView->layout, "loaded", NULL);
	int w, h;

	ivug_image_image_size_get(pBuddyView->photocam, &w, &h);

	/*bool bSendEnable = _ivug_buddy_can_be_sent(data);
	if (bSendEnable)
	{
		MSG_HIGH("Enable Send btn");
		elm_object_item_disabled_set(pBuddyView->btn_send, EINA_FALSE);
	}*/
}

static Evas_Event_Flags _n_finger_tap_end(void *data , void *event_info)
{
	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;

	MSG_HIGH("Finger tab end. evt=%d", p->timestamp);

	if (p->n != 1) return EVAS_EVENT_FLAG_NONE;

	Ivug_BuddyView *pBuddyView = (Ivug_BuddyView *)data;

	if (pBuddyView->photocam == NULL)
	{
		MSG_WARN("photocam is not exist");
		return EVAS_EVENT_FLAG_NONE;
	}

	if (pBuddyView->bShowMenu)
	{
		evas_object_smart_callback_call(pBuddyView->layout, "clicked", (void *)false);
		pBuddyView->bShowMenu = false;
	}
	else
	{
		evas_object_smart_callback_call(pBuddyView->layout, "clicked", (void *)true);
		pBuddyView->bShowMenu = true;
	}

	return EVAS_EVENT_FLAG_NONE;
}


static void _on_layout_resized(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	IV_ASSERT(data != NULL);

	Ivug_BuddyView *pBuddyView = (Ivug_BuddyView *)data;

	int x, y, w, h;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

	MSG_HIGH("Layout Geometry : %d,%d,%d,%d", x, y, w, h);

	evas_object_move(pBuddyView->event_obj, x, y);
	evas_object_resize(pBuddyView->event_obj, w, h);
}

static bool _ivug_buddy_view_create_layout(Ivug_BuddyView *pBuddyView, Evas_Object *parent)
{
	IV_ASSERT(pBuddyView != NULL);
/*
	pBuddyView->layout = ivug_layout_add2(parent, BUDDY_VIEW_EDJ_FILE, "buddy_view");
	if (pBuddyView->layout == NULL)
	{
		MSG_ERROR("Cannot create layout.");
		free(pBuddyView);
		return false;
	}
*/
	pBuddyView->layout = ivug_layout_add2(parent, BUDDY_VIEW_EDJ_FILE, "navi_content");
	if (pBuddyView->layout == NULL)
	{
		MSG_ERROR("layout swallow failed");
		ivug_buddy_view_destroy(pBuddyView);
		return false;
	}

	evas_object_name_set(pBuddyView->layout, "Empty layout");

	//photocam
	Evas_Object *photocam = NULL;
	photocam = ivug_image_create(pBuddyView->layout);
	ivug_image_animated_set(photocam, EINA_FALSE);	// Show first frame only when AGIF
	evas_object_name_set(photocam, "buddy_photocam");

	evas_object_smart_callback_add(photocam, "loaded", _on_photocam_loaded, (void *)pBuddyView);
	elm_object_part_content_set(pBuddyView->layout, "photocam", photocam);
	evas_object_show(photocam);

	pBuddyView->photocam = photocam;

	evas_object_event_callback_add(pBuddyView->layout, EVAS_CALLBACK_RESIZE, _on_layout_resized, pBuddyView);

	evas_object_show(pBuddyView->layout);

	return true;
}

Ivug_BuddyView * ivug_buddy_view_create(Evas_Object *parent)
{
	Ivug_BuddyView *pBuddyView = (Ivug_BuddyView *)calloc(1, sizeof(Ivug_BuddyView));

	if (pBuddyView == NULL)
	{
		MSG_ERROR("Cannot allocate memory");
		return NULL;
	}

	if (_ivug_buddy_view_create_layout(pBuddyView, parent) == false)
	{
		MSG_ERROR("Cannot create layout");
		return NULL;
	}

	pBuddyView->gesture = elm_gesture_layer_add(pBuddyView->layout);
	elm_gesture_layer_hold_events_set(pBuddyView->gesture, EINA_FALSE);

	elm_gesture_layer_cb_set(pBuddyView->gesture, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END,
		_n_finger_tap_end, pBuddyView);

	if (elm_gesture_layer_attach(pBuddyView->gesture, pBuddyView->layout) == EINA_FALSE)
	{
		MSG_ERROR("Cannot attach event rect");
	}

	pBuddyView->bShowMenu = true;

	return pBuddyView;
}


bool ivug_buddy_view_destroy(Ivug_BuddyView *pBuddyView)
{
	if (pBuddyView == NULL)
	{
		MSG_ERROR("already destroyed");
		return false;
	}

	MSG_HIGH("ivug_buddy_view_destroy");

	if (pBuddyView->event_obj)
	{
		evas_object_del(pBuddyView->event_obj);
		pBuddyView->event_obj = NULL;
	}

	if (pBuddyView->filepath)
	{
		free(pBuddyView->filepath);
		pBuddyView->filepath = NULL;
	}

	if (pBuddyView->mediaID)
	{
		free(pBuddyView->mediaID);
		pBuddyView->mediaID = NULL;
	}

	if (pBuddyView->photocam)
	{
		evas_object_del(pBuddyView->photocam);
		pBuddyView->photocam = NULL;
	}

	if (pBuddyView->navi_bar)
	{
		evas_object_del(pBuddyView->navi_bar);
		pBuddyView->navi_bar = NULL;
	}

	if (pBuddyView->layout)
	{
		evas_object_del(pBuddyView->layout);
		pBuddyView->layout = NULL;
	}

	free(pBuddyView);

	return true;
}



bool ivug_buddy_view_load_file(Ivug_BuddyView *pBuddyView, const char *filepath, const char *mediaID)
{
	IV_ASSERT(pBuddyView != NULL);

	pBuddyView->filepath = strdup(filepath);
	pBuddyView->mediaID = strdup(mediaID);

	MSG_SEC("Load image file : %s", filepath);

	Evas_Load_Error error = EVAS_LOAD_ERROR_NONE;

	error = ivug_image_file_set(pBuddyView->photocam, filepath, "noAnim");

	if (error != EVAS_LOAD_ERROR_NONE)
	{
		TODO("What is good for error handing??????")
		MSG_ERROR("FileSet Error=%d", error);

		if (error == EVAS_LOAD_ERROR_UNKNOWN_FORMAT)
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_UNKNOWN_FORMAT);
			return false;
		}
		else if (error == EVAS_LOAD_ERROR_PERMISSION_DENIED)
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_PERMISSION_DENIED);
			return false;
		}
		else
		{
			evas_object_smart_callback_call(pBuddyView->layout, "load,failed", (void *)BUDDY_ERROR_TYPE_INVALID_FILE);
			return false;
		}

		return false;
	}

	return true;
}

