/*
 * CSize.h
 *
 *       Created on: 17.09.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef CSIZE_H_
#define CSIZE_H_

#include <string>
#include <ostream>

class CSize {
public:
	int cx, cy;
	CSize() {
		cx = cy = 0;
	}
	CSize(int _cx, int _cy) {
		cx = _cx;
		cy = _cy;
	}

	CSize(const CSize& size) {
		cx = size.cx;
		cy = size.cy;
	}

	bool operator!=(const CSize& size) const {
		return cx != size.cx || cy != size.cy;
	}

	std::string toString()const;
};

std::ostream& operator<<(std::ostream& os, const CSize& a);


#endif /* CSIZE_H_ */
