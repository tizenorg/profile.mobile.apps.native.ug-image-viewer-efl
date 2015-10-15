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


#include "ivug-smart-event-box.h"
#include <ui-gadget-module.h>

#include "ivug-common.h"
#include "ivug-parameter.h"

#define IF_FREE(p) ({if (p) {free(p);p=NULL;}})

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

typedef struct _EventCallbackData_t
{
	void (*func)(void *);
	void *data;
}EventCallbackData_t;

typedef struct _IvugSmartEventBoxData_t
{
	Evas_Coord x, y, w, h;
	Evas_Coord down_x;
	Evas_Coord down_y;

	EventCallbackData_t cbs[IVUG_EVENT_MAX];

} IvugSmartEventBoxData_t;


static void
__mouse_down_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;
	IvugSmartEventBoxData_t *box_d = data;
	box_d->down_x = ev->canvas.x;
	box_d->down_y = ev->canvas.y;

	return;
}

static void
__mouse_up_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Coord minw = 0, minh = 0, diff_x = 0, diff_y = 0;
	Evas_Event_Mouse_Up *mu = (Evas_Event_Mouse_Up *) event_info;
	IvugSmartEventBoxData_t *box_d = data;
	elm_coords_finger_size_adjust(1, &minw, 1, &minh);

	diff_x = box_d->down_x - mu->canvas.x;
	diff_y = box_d->down_y - mu->canvas.y;

	if ((ABS(diff_x) > minw) || (ABS(diff_y) > minh))
	{			// dragging
		if (ABS(diff_y) > ABS(diff_x))
		{
			if (diff_y < 0)	//down
				goto flick_down;
			else	//up
				goto flick_up;
		}
		else
		{
			if (diff_x < 0)
			{	//right
				goto flick_right;
			}
			else
			{	//left
				goto flick_left;
			}
		}
	}

	box_d->cbs[IVUG_EVENT_CLICK].func(box_d->cbs[IVUG_EVENT_CLICK].data);
	return;

      flick_up:
	return;

      flick_down:
	return;

      flick_left:
      	box_d->cbs[IVUG_EVENT_LEFT].func(box_d->cbs[IVUG_EVENT_LEFT].data);
	return;

      flick_right:
	box_d->cbs[IVUG_EVENT_RIGHT].func(box_d->cbs[IVUG_EVENT_RIGHT].data);
	return;
}

static void
_ivug_smart_event_box_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	IvugSmartEventBoxData_t *b_data= data;
	IF_FREE(b_data);
}

Evas_Object *
ivug_smart_event_box_add(Evas_Object * parent)
{
	IvugSmartEventBoxData_t *data = NULL;
	Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	data = calloc(1, sizeof(IvugSmartEventBoxData_t));
	evas_object_data_set(rect, "obj_data", data);
	//evas_object_size_hint_min_set(rect, 0, 15);
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_size_hint_fill_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, 0.0);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_DEL, _ivug_smart_event_box_del_cb, data);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb, data);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb, data);
	return rect;
}

void ivug_smart_event_box_callback_add(Evas_Object *event_box, IvugEventCallback_e event, void (*event_cb)(void *), void *user_data)
{
	IvugSmartEventBoxData_t *data = evas_object_data_get(event_box, "obj_data");

	if (data != NULL) {
		data->cbs[event].func= event_cb;
		data->cbs[event].data = user_data;
	}
}


