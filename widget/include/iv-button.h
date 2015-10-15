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

#pragma once

#include "EvasSmartObj.h"

/*
	Signals
		"clicked"
*/

namespace iv {

#ifndef LOG_LVL
#define LOG_LVL DBG_MSG_LVL_HIGH
#endif

#ifndef LOG_CAT
#define LOG_CAT "IV-WIDGET"
#endif

class CButton : public CEvasSmartObject<CButton>
{
private:
	enum eButtonSate{
		STATE_UNDEFINED,
		STATE_DEFAULT,
		STATE_PRESS,
		STATE_DIM,
		STATE_MAX,
	} ;

	static void _on_edje_down_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
	{
		CButton *thiz = static_cast<CButton *>(data);

		thiz->OnMouseDown();
	}

	static void _on_edje_up_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
	{
		CButton *thiz = static_cast<CButton *>(data);

		thiz->OnMouseUp();

	}

	static void _on_edje_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
	{
		CButton *thiz = static_cast<CButton *>(data);

		thiz->OnMouseClick();

	}

private:
	void UpdateButtons(eButtonSate newstate);

	void OnMouseDown();
	void OnMouseUp();
	void OnMouseClick();

	Evas_Object *CreateObject(Evas_Object *parent);
public:
	static CButton *ObjectFactory(Evas_Object *parent) {
		CButton *pInstance = new CButton;

		pInstance->CreateObject(parent);

		return pInstance;
	};

	CButton() : CEvasSmartObject<CButton>(), m_state(STATE_UNDEFINED)
	{
		CEvasSmartObject<CButton>::SetClassName("IVButton");

		m_img[STATE_DEFAULT] = NULL;
		m_img[STATE_PRESS] = NULL;
		m_img[STATE_DIM] = NULL;

		MSG_MED("CButton constructor");
	};

	virtual ~CButton() {
		MSG_MED("CButton destructor");

		evas_object_del(m_edje);
//prevent issue fix
//		delete this;
	};

	void SetImage(Evas_Object *normal, Evas_Object *press, Evas_Object *dim);

	void Draw();

	void resize(int w, int h) {
		CEvasSmartObject<CButton>::resize(w,h);

		if ( m_edje )
		{
			MSG_MED("Resize WH(%d,%d)", w, h);
			evas_object_resize(m_edje, w + 10 , h + 10);
		}

		if ( m_img[m_state] != NULL )
		{
			evas_object_resize(m_img[m_state], w, h);
		}
	};

	void move(int x, int y) {
		CEvasSmartObject<CButton>::move(x,y);

		if ( m_edje )
		{
			MSG_MED("Move XY(%d,%d)", x, y);
			evas_object_move(m_edje, x - 5, y - 5);
		}

		if ( m_img[m_state] != NULL )
		{
			evas_object_move(m_img[m_state], x, y);
		}
	};

	void show() {
		if ( m_edje )
			evas_object_show(m_edje);

		if ( m_img[m_state] != NULL )
		{
			evas_object_show(m_img[m_state]);
		}

	};

	void hide() {
		if ( m_edje )
			evas_object_hide(m_edje);

		if ( m_img[m_state] != NULL )
		{
			evas_object_hide(m_img[m_state]);
		}


	};

private:
	Evas_Object *m_edje;

	Evas_Object *m_img[STATE_MAX];
	eButtonSate m_state;

};

};

