/*
 * DealResult.cpp
 *
 *  Created on: 23.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include <cstring>
#include <cassert>

#include "DealResult.h"

bool DealResult::operator <(const DealResult &dr) const {
	int i=strcmp(a[0],dr.a[0]);
	assert(i!=0);
	return i<0;
}
