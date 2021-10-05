/*
 * CPoint.h
 *
 *       Created on: 17.09.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef CPOINT_H_
#define CPOINT_H_

#include <gtk/gtk.h>

class CPoint {
public:
	int x, y;
	CPoint() {
		x = y = 0;
	}
	CPoint(int _x, int _y) {
		x = _x;
		y = _y;
	}
	CPoint(GdkEventButton*p) {
		x = p->x;
		y = p->y;
	}
	void operator+=(const CPoint& p) {
		x += p.x;
		y += p.y;
	}
	void operator-=(const CPoint& p) {
		x -= p.x;
		y -= p.y;
	}

	bool operator==(const CPoint& p) const {
		return x == p.x && y == p.y;
	}
};

#endif /* CPOINT_H_ */
