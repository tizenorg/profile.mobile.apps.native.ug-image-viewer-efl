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

#include <Ecore.h>
#include "Transit.h"

#include <list>
#include "Exception.h"

using namespace std;

// Composite pattern
class CAnimator
{
public:
	CAnimator() {};

	virtual ~CAnimator() {};

	virtual void Update(double delta_t) = 0;
	virtual bool IsFinished() = 0;

	virtual void AddAnimator(CAnimator *animator) {
		throw new CException(CException::IO_ERROR);
	};

protected:
};


class CSingleAnimator : public CAnimator
{
private:
	CSingleAnimator() {};
public:
// TODO. Using factory
	CSingleAnimator(double first, double last, double duration, TranstionFunc transition, AnimationCB callback, void *data) {
		m_actor = new CActor(first, last, duration, transition, callback, data);
	};

	virtual ~CSingleAnimator() { delete m_actor;};

// Overrides
	void Update(double delta_t) {
		m_actor->Update(delta_t);
	};

	bool IsFinished() { return m_actor->IsFinished(); };

private:
	CActor *m_actor;
};

class CGroupAnimator : public CAnimator {
public:
	CGroupAnimator() {};
	virtual ~CGroupAnimator() { ClearAll(); };

	void AddAnimator(CAnimator *animator) {
		m_actorlist.push_back(animator);

		m_actorlist.back()->Update(0);
	};

	void ClearAll() {
		for ( list<CAnimator *>::iterator iter = m_actorlist.begin(); iter != m_actorlist.end(); iter++ )
		{
			delete *iter;
		}

		m_actorlist.clear();

	};

	bool IsFinished() {
		return m_actorlist.empty();
	};

	void Update(double delta_t) {
		for ( list<CAnimator *>::iterator iter = m_actorlist.begin(); iter != m_actorlist.end(); )
		{
			(*iter)->Update(delta_t);

			if ( (*iter)->IsFinished() == true )
			{
				iter = m_actorlist.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	};

private:
	list<CAnimator *> m_actorlist;

};

