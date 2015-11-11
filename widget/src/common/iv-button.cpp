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
#include <Edje.h>
#include <Elementary.h>

#include "ivug-define.h"
#include "ivug-debug.h"

#include "iv-button.h"

#define EDJ_PATH PREFIX"/res/edje/"PACKAGE
#define IMG_PATH PREFIX"/res/images/"PACKAGE

namespace iv
{


template<>
const Evas_Smart_Cb_Description CEvasSmartObject<CButton>::_signals[] = {
	{"clicked", ""},
	{NULL, NULL}
};

Evas_Object *CButton::CreateObject(Evas_Object *parent)
{
	Evas_Object *sObj = CEvasSmartObject<CButton>::CreateObject(parent);
// Register signal
	evas_object_smart_callbacks_descriptions_set(sObj, _signals);

	m_edje = edje_object_add(evas_object_evas_get(parent));

	if (!edje_object_file_set(m_edje, EDJ_PATH"/ivug-widget-button.edj", "iv/button")) {
		Edje_Load_Error err = edje_object_load_error_get(m_edje);
		const char *errmsg = edje_load_error_str(err);
		MSG_ERROR("could not load 'group_name' from theme.edj: %s",  errmsg);
		evas_object_del(m_edje);
		return NULL;
	}

	evas_object_smart_member_add(m_edje, sObj);

	m_state = STATE_DEFAULT;
	return sObj;
};


void CButton::UpdateButtons(eButtonSate newstate)
{
	if (m_state == newstate) {
		return;
	}

	if (m_img[m_state] != NULL) {
		evas_object_hide(m_img[m_state]);
	}

	Evas_Object *obj;

	obj = m_img[newstate];

	m_state = newstate;

	if (obj == NULL) {
		MSG_WARN("No icon for state(%d)", newstate);
		return;
	}

	CRect rect = GetGeometry();

	MSG_HIGH("Edje XYWH(%d,%d,%d,%d)", rect.Left(), rect.Top(), rect.Width(), rect.Height());

	evas_object_move(obj, rect.Left(), rect.Top());
	evas_object_resize(obj, rect.Width(), rect.Height());

	evas_object_raise(obj);
	evas_object_show(obj);
}

void CButton::Draw()
{
	CRect rect = GetGeometry();

	MSG_LOW("Draw XYWH(%d,%d,%d,%d)", rect.Left(), rect.Right(), rect.Width(), rect.Height());
}

void CButton::OnMouseDown()
{
	MSG_MED("OnMouseDown");

	UpdateButtons(STATE_PRESS);
}

void CButton::OnMouseUp()
{
	MSG_MED("OnMouseUp");

	UpdateButtons(STATE_DEFAULT);
}

void CButton::OnMouseClick()
{
	MSG_MED("OnMouseClick");

	evas_object_smart_callback_call(GetObject(), "clicked", NULL);
}

void CButton::SetImage(Evas_Object *normal, Evas_Object *press, Evas_Object *dim)
{
// Remove Old icons
	if (m_img[STATE_DEFAULT] != NULL) {
		evas_object_del(m_img[STATE_DEFAULT]);
	}

	if (m_img[STATE_PRESS] != NULL) {
		evas_object_del(m_img[STATE_PRESS]);
	}

	if (m_img[STATE_DIM] != NULL) {
		evas_object_del(m_img[STATE_DIM]);
	}

	m_img[STATE_DEFAULT] = normal;
	m_img[STATE_PRESS] = press;
	m_img[STATE_DIM] = dim;

	MSG_HIGH("SetImage");

	if (m_img[STATE_DEFAULT] != NULL) {
		evas_object_pass_events_set(m_img[STATE_DEFAULT], EINA_TRUE);
		evas_object_smart_member_add(m_img[STATE_DEFAULT], GetObject());
	}

	if (m_img[STATE_PRESS] != NULL) {
		evas_object_pass_events_set(m_img[STATE_PRESS], EINA_TRUE);
		evas_object_smart_member_add(m_img[STATE_PRESS], GetObject());
	}

	if (m_img[STATE_DIM] != NULL) {
		evas_object_pass_events_set(m_img[STATE_PRESS], EINA_TRUE);
		evas_object_smart_member_add(m_img[STATE_DIM], GetObject());
	}

	edje_object_signal_callback_add(m_edje, "mouse,up,*", "event", _on_edje_up_cb, this);
	edje_object_signal_callback_add(m_edje, "mouse,down,*", "event", _on_edje_down_cb, this);
	edje_object_signal_callback_add(m_edje, "mouse,clicked,*", "event", _on_edje_click_cb, this);

	UpdateButtons(STATE_DEFAULT);

};


};
