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

#include <Evas.h>
#include "Primitives.h"
#include "ivug-debug.h"

template<typename T>
class CEvasSmartObject {
protected:
	static const Evas_Smart_Cb_Description _signals[];

public:
	CEvasSmartObject() : m_obj(NULL) {};
	virtual ~CEvasSmartObject() {
		if ( m_obj )
			evas_object_del(m_obj);
		m_obj= NULL;
	};

	static void SetClassName(const char *classname)
	{
		if (_smart_ == NULL )
		{	// Damm buggy! Implement using template
			const char* name = eina_stringshare_add(classname);
			MSG_HIGH("Create smart class name=%s", name);
			static Evas_Smart_Class sc =
			{
				NULL,
				EVAS_SMART_CLASS_VERSION,
				NULL,
				_del,
				_move,
				_resize,
				_show,
				_hide,
				_color_set,
				_clip_set,
				_clip_unset,
				_calculate,
				NULL,
				NULL,
				NULL,		// Parent
				NULL,
				NULL,
				NULL
			};	//Evas_smart_Class
			sc.name = name;
			_smart_ = evas_smart_class_new(&sc);
		}
	}

	virtual Evas_Object *CreateObject(Evas_Object *parent)
	{
		m_obj = evas_object_smart_add(evas_object_evas_get(parent), _smart_);	//create smart object.
		evas_object_smart_data_set(m_obj, this);

		MSG_ASSERT(m_obj != NULL);

		return m_obj;
	}

private:
	static void _del(Evas_Object * obj)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->remove();
	};

	static void _move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->move(x, y);
	};

	static void _resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->resize(w, h);
	};

	static void _show(Evas_Object *obj)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->show();
	};

	static void _hide(Evas_Object *obj)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->hide();
	};

	static void _calculate(Evas_Object *obj)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->draw();
	};

	static void _color_set(Evas_Object *obj, int r, int g, int b, int a)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->color_set(r,g,b,a);
	};

	static void _clip_set(Evas_Object *obj, Evas_Object * clip)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->clip_set(clip);
	};

	static void _clip_unset(Evas_Object *obj)
	{
		CEvasSmartObject *thiz = static_cast<CEvasSmartObject *>(evas_object_smart_data_get(obj));
		IV_ASSERT(thiz != NULL);

		thiz->clip_unset();
	};

protected:
	virtual void move(int x, int y) {
		m_rect.Left(x);
		m_rect.Top(y);
	};

	virtual void resize(int w, int h) {
		m_rect.Width(w);
		m_rect.Height(h);
	};

	virtual void remove() {};
	virtual void show() {};
	virtual void hide() {};
	virtual void draw() {};
	virtual void color_set(int r, int g, int b, int a) {};
	virtual void clip_set(Evas_Object *clipper) {};
	virtual void clip_unset() {};

public:
	Evas_Object *GetObject() const { return m_obj; };

	const CRect &GetGeometry() const { return m_rect; };

public:
	CRect m_rect;

	Evas_Object *m_obj;

	static Evas_Smart *_smart_;

};

template<typename T> Evas_Smart *CEvasSmartObject<T>::_smart_ = NULL;

