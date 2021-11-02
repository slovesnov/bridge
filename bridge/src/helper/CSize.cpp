/*
 * CSize.cpp
 *
 *  Created on: 02.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "CSize.h"

std::string CSize::toString()const{
	return std::to_string(cx)+"x"+std::to_string(cy);
}

std::ostream& operator<<(std::ostream& os, const CSize& a){
    os << a.toString();
    return os;
}

