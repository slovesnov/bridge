/*
 * CRect.h
 *
 *       Created on: 17.09.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef CRECT_H_
#define CRECT_H_

#include "CPoint.h"
#include "CSize.h"

class CRect {
public:
	int left;
	int top;
	int right;
	int bottom;

	CRect() {
		left = top = right = bottom = 0;
	}

	CRect(CPoint p, CSize s) {
		left = p.x;
		top = p.y;
		right = left + s.cx;
		bottom = top + s.cy;
	}

	CRect(int _left, int _top, int _right, int _bottom) {
		init(_left, _top, _right, _bottom);
	}

	CRect const& operator=(CRect const& r) {
		left = r.left;
		top = r.top;
		right = r.right;
		bottom = r.bottom;
		return *this;
	}

	inline void init(int _left, int _top, int _right, int _bottom) {
		left = _left;
		top = _top;
		right = _right;
		bottom = _bottom;
	}

	inline void join(CRect const& r) {
		if (r.left < left) {
			left = r.left;
		}
		if (r.top < top) {
			top = r.top;
		}
		if (r.right > right) {
			right = r.right;
		}
		if (r.bottom > bottom) {
			bottom = r.bottom;
		}
	}

	inline int width() const {
		return right - left;
	}

	inline int height() const {
		return bottom - top;
	}

	inline CSize size() const {
		return {width(),height()};
	}

	inline CPoint centerPoint() const {
		return CPoint((left + right) / 2, (top + bottom) / 2);
	}

	inline CPoint topLeft() const {
		return CPoint(left, top);
	}

	inline bool in(GdkEventButton*p) {
		return in(p->x, p->y);
	}

	inline bool in(double x, double y) {
		return x >= left && x < right && y >= top && y < bottom;
	}

};

#endif /* CRECT_H_ */
