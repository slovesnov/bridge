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

#include "DealResult.h"

bool DealResult::operator <(const DealResult &dr) const {
	int i=strcmp(a[0],dr.a[0]);
	if(i!=0){
		return i<0;
	}
	else{
		return strcmp(a[1],dr.a[1])<0;
	}
}
