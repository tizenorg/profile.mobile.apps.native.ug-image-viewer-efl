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

#include <Elementary.h>
#include "Object.h"

//#include "Scroller.h"

#undef DISABLE_ORIGIN

class CScrollerClient : public CObject {
	friend class CScroller;
public:
	CScrollerClient(Evas_Object *parent) : m_zoom(0.0f), m_size(0,0), m_update(0), m_origin(0,0), m_scroller(NULL) {
		CreateObject(parent);
	};

	virtual ~CScrollerClient(){
		//MSG_HIGH("CScrollerClient destructor");
	};

public:
	const CRect GetClientRect() const {
		// Returns window rect(LCD coordinate)
		return CRect(topleft, m_extent);
	};

	virtual void Move(int x, int y) { topleft.MoveTo(x,y);	};
	virtual void MoveBy(int dx, int dy) { topleft.MoveBy(dx,dy); };

	virtual void PanX(int x) { topleft.X(x);	};
	virtual void PanY(int y) { topleft.Y(y);	};

	virtual void Draw() { //MSG_HIGH("Scroller client Draw"); };

	virtual void BeginUpdate() {
		m_update++;

		//MSG_HIGH("Begin update. Count=%d", m_update);
	};
	virtual void EndUpdate() {
		m_update--;

		//MSG_HIGH("End update. Count=%d", m_update);
	};

	// Scroller client original size
	virtual void SetSize(const CSize &size) {
		//MSG_HIGH("CScrollerClient SetSize(%d,%d)", size.Width(), size.Height());

		m_size = size;
	};
	virtual const CSize &GetClientSize() const {
		// Returns client's original size.
		return m_size;
	};
	virtual double Zoom() const { return m_zoom; };
	virtual void Zoom(double zoom) {
		m_zoom = zoom;

		m_extent = m_size * m_zoom;

		//MSG_LOW("Set Zoom=%f", m_zoom);
	};

	CPoint &GetOrigin() { return m_origin; };
	void SetOrigin(int x, int y) { m_origin = CPoint(x,y); };

	void Changed() {
		// Inform clicent changed to scroller
		m_scroller->OnChanged(this);
	};

private:
	void SetScroller(CScroller *scroller) {
		m_scroller = scroller;
	};

	void UnSetScroller() {
		m_scroller = NULL;
	};

// Need OnResized??
protected:
	double m_zoom;

	CPoint topleft;
	CSize  m_size;			// original size

	int m_update;

private:
	CPoint m_origin;	// Currently unused.
	CSize  m_extent;		// display size

	CScroller *m_scroller;

};


