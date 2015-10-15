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

#include <algorithm>

class CSize {
public:
	static const CSize Zero;

	CSize(int _w = 0, int _h = 0) {
		w = _w;
		h = _h;
	};

	CSize(const CSize &rhs) {
		SetSize(rhs.w, rhs.h);
	};

	void GetSize(int &w, int &h) const {
		w = this->w;
		h = this->h;
	};

	void SetSize(int _w, int _h) {
		w = _w;
		h = _h;
	};

	const CSize operator *(double mul) const {
		return CSize(w * mul, h *mul);
	}

	const CSize operator /(double mul) const {
		return CSize(w / mul, h / mul);
	}

	const CSize operator +(const CSize &rhs) const {
		return CSize(w + rhs.Width(), h + rhs.Height());
	}

	const CSize operator -(const CSize &rhs) const {
		return CSize(w - rhs.Width(), h - rhs.Height());
	}

	int Width() const { return w;};
	int Height() const { return h;};

	void Width(int _w) { w = _w; };
	void Height(int _h) { h = _h; };

	void Swap() { std::swap(w,h); };

private:
	int w, h;
};


class CPoint {
public:
	static const CPoint Zero;

	CPoint(int _x = 0, int _y = 0) {
		x = _x;
		y = _y;
	};

	void GetPoint(int &x, int &y) const {
		x = this->x;
		y = this->y;
	};

	void MoveTo(int x_, int y_) {
		x = x_;
		y = y_;
	};

	void MoveBy(int x_, int y_) {
		x += x_;
		y += y_;
	};

	const CPoint operator -(const CPoint &rhs) const {
		return CPoint(x - rhs.X(), y - rhs.Y());
	};

	const CPoint operator +(const CPoint &rhs) const {
		return CPoint(x + rhs.X(), y + rhs.Y());
	};

	const CPoint operator +(const CSize &rhs) const {
		return CPoint(x + rhs.Width(), y + rhs.Height());
	};

	bool operator !=(const CPoint &rhs) const {
		return (y != rhs.Y()) ||  (y != rhs.Y()) ;
	};


	int X() const { return x; };
	int Y() const { return y; };

	void X(int _x) { x = _x; };
	void Y(int _y) { y = _y; };

private:
	int x, y;
};



class CRect {
public:
	static const CRect Zero;
//	with north-west gravity default.
public:

	CRect(int _x = 0, int _y = 0, int _w = 0, int _h = 0) {
		SetRect(_x, _y, _w, _h);
	};

	CRect(const CRect &rhs) {
		SetRect(rhs.Left(), rhs.Top(), rhs.Width(), rhs.Height());
	}

	CRect(const CPoint &_topleft, const CSize &_size) {
		SetRect(_topleft.X(), _topleft.Y(), _size.Width(), _size.Height());
	}

	CRect(const CPoint &_topleft, const CPoint &_bottomright) {
			topleft = _topleft;
			bottomright = _bottomright;
	}

	~CRect() {};

	const CRect operator /(double mul) const {
		return CRect(Left(), Top(), Width() / mul, Height() / mul);
	}

	bool operator !=(const CRect &rhs) const {
		return ( topleft.X() != rhs.Left() ) || ( topleft.Y() != rhs.Top() ) ||
			( bottomright.X() != rhs.Right() ) || ( bottomright.Y() != rhs.Bottom() );
	};


	void SetRect(int _x, int _y, int _w, int _h) {
		topleft.X(_x);
		topleft.Y(_y);

		SetSize(_w,_h);
	};

	void MoveTo(const CPoint &_topleft)
	{
		CSize cSize = GetSize();

		topleft = _topleft;
		SetSize(cSize);
	}

	void MoveTo(int _x, int _y)
	{
		CSize cSize = GetSize();

		topleft = CPoint(_x,_y);
		SetSize(cSize);
	}

	void MoveBy(const CPoint &_dpoint)		// Difference move
	{
		topleft = topleft + _dpoint;
		bottomright = bottomright + _dpoint;
	}

	void MoveBy(int dx, int dy)		// Difference move
	{
		topleft = topleft + CPoint(dx, dy);
		bottomright = bottomright + CPoint(dx, dy);
	}

	void Resize(int _w, int _h)
	{
		SetSize(_w, _h);
	}

	const CPoint GetCenter() const { return CPoint(topleft.X() + Width() / 2, topleft.Y() + Height() / 2); };
// Getter
	const CSize GetSize() const { return CSize(Width() , Height() ); };

	int Left() const { return topleft.X(); };
	int Right() const { return bottomright.X(); };
	int Top() const { return topleft.Y(); };
	int Bottom() const { return bottomright.Y(); };

	const CPoint &TopLeft() const { return topleft; };
	const CPoint &BottomRight() const { return bottomright; };

	bool IsValid() {
		return !( Width() < 0 || Height() < 0 );
	};

// Setter
	void SetSize(const CSize &newsize) {
		Width(newsize.Width());
		Height(newsize.Height());
	};

	void SetSize(int w, int h) {
		Width(w);
		Height(h);
	};

	void Left(int _V) { int W = Width(); topleft.X(_V); bottomright.X(_V + W); };
	void Right(int _V) { int W = Width(); bottomright.X(_V); topleft.X(_V - W); };
	void Top(int _V) { int H = Height(); topleft.Y(_V); bottomright.Y(_V + H); };
	void Bottom(int _V) { int H = Height(); bottomright.Y(_V); topleft.Y(_V - H);};


	int Width() const { return bottomright.X() - topleft.X(); };
	int Height() const { return bottomright.Y() - topleft.Y(); };
	void Width(int _W) { bottomright.X(topleft.X() + _W); };
	void Height(int _H) { bottomright.Y(topleft.Y() + _H); };

	bool Intersect(const CRect &rhs) const {
		return ( ( Left() < rhs.Right() ) && ( Top() < rhs.Bottom() ) && ( Right() > rhs.Left() ) && ( Bottom() > rhs.Top() ));
	};

	void Inflate(int x1, int y1, int x2, int y2) {
		topleft.X( topleft.X() + x1);
		topleft.Y( topleft.Y() + y1);

		bottomright.X( bottomright.X() + x2);
		bottomright.Y( bottomright.Y() + y2);
	};

	friend CRect UnionRect(const CRect &rect1, const CRect &rect2);
	friend CRect IntersectRect(const CRect &rect1, const CRect &rect2);

	bool Inside(const CPoint &rhs) const {
		return (Left() < rhs.X()) && (rhs.X() < Left() + Width() ) && (Top() < rhs.Y()) && (rhs.Y() < Top() + Height() );
	};

	bool Inside(const CRect &rhs) const {
		return (Left() <= rhs.Left()) && (rhs.Right() <= Right() ) && (Top() <= rhs.Top()) && (rhs.Bottom() <= Bottom() );
	};

	const CRect &Scale(double ratio_x, double ratio_y) {
		int w = Width();
		int h = Height();

		topleft.X( topleft.X() * ratio_x);
		topleft.Y( topleft.Y() * ratio_y);

		w = w * ratio_x;
		h = h * ratio_y;
		bottomright.X( topleft.X() + w);
		bottomright.Y( topleft.Y() + h);

		return *this;
	};

private:
	CPoint topleft;
	CPoint bottomright;
};


