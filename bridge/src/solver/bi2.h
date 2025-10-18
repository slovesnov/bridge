/*
 * bi2.h
 *
 *       Created on: 14.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

int i2, r2, rc2, a3;
USC sc2;

#ifdef BRIDGE_NODE_COUNT
	m_nodes++;
	#endif

for (i2 = 0; i2 < c2.length; i2++) {
	sc2=c2[i2];
	r2 = sc2.c;

#define r0 sc0.c
	ADJUST_RANK(2,0)
	ADJUST_RANK(2,1)
#undef r0

	REMOVE_CARD(2)

	a3 = a1;

#ifdef STOREBEST
#undef STOREBEST
#include "bi3.h"
#define STOREBEST
#else
#include "bi3.h"
#endif

	a3=-a3;

	RESTORE_CARD(2);

	if (a3 > a2) {
#ifdef STOREBEST
			SET_BEST(2)
#endif
		a2 = a3;
		break;

	}
}

