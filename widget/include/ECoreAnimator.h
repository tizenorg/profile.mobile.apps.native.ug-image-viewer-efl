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

typedef bool (*ProgressCBFUNC)(double delta, void *data);
typedef void (*EndCBFUNC)(void *data);


template <class ObjType, class FunType, class ParmType>
class UserMemFunctor
{
public:
    UserMemFunctor(ObjType& obj, FunType fun) : m_obj(obj), m_func(fun) {}
    void operator()(ParmType& p) { (m_obj.*m_func)(p); }
private:
    ObjType& m_obj;
    FunType m_func;
};

// TODO : Implement using Functor.
#undef LOG_CAT
#define LOG_CAT "IV-ANIM"

class ECoreAnimator {
	static Eina_Bool _on_anim(void *data)
	{
		ECoreAnimator *thiz = static_cast<ECoreAnimator *>(data);

		return thiz->OnAnimation();
	}

public:
	ECoreAnimator() : m_animator(NULL), m_bStarted(false) {};
	~ECoreAnimator() {
		Stop();
	};

	void Start(ProgressCBFUNC prog, EndCBFUNC end, void *data)
	{
		m_endcb = end;
		m_progcb = prog;

		m_data = data;

		m_count = 0;
		m_animator = ecore_animator_add(_on_anim, this);
		MSG_ASSERT(m_animator != NULL);
		m_bStarted = true;
	}

	void Stop() {
		if ( m_bStarted )
		{
			ecore_animator_del(m_animator);
			m_bStarted = false;
			m_animator = NULL;

			if ( m_endcb )
			{
				m_endcb(m_data);
			}
		}
	};

	Eina_Bool OnAnimation() {
		double current;

		MSG_ASSERT(m_bStarted != false);

		if ( m_count == 0 )
		{
			current = m_prev = ecore_loop_time_get();
		}
		else
		{
			current = ecore_loop_time_get();
		}

		m_count++;

		if ( m_progcb(current - m_prev, m_data) == false)
		{
			Stop();
			return ECORE_CALLBACK_CANCEL;
		}

		m_prev = current;
		return ECORE_CALLBACK_RENEW;
	};

	bool IsRunning() const {
		return m_bStarted;
	};

private:
	Ecore_Animator *m_animator;

//	template <typename T, bool (T::*callback)()>
//	func_bind<T, callback> m_f;
	ProgressCBFUNC m_progcb;
	EndCBFUNC m_endcb;

	void *m_data;

	double m_prev;
	bool m_bStarted;

	int m_count;

};
