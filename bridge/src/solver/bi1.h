/*
 * bi1.h
 *
 *       Created on: 14.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

	int i1, r1, rc1, a2;
	USC sc1;
#ifdef BRIDGE_NODE_COUNT
	m_nodes++;
#endif

	for (i1 = 0; i1 < c1.length; i1 ++) {
		sc1=c1[i1];
		r1 = sc1.c;

#define r0 sc0.c
		ADJUST_RANK(1,0)
#undef r0

		REMOVE_CARD(1)

		a2 = a;

#ifdef STOREBEST
#undef STOREBEST
#include "bi2.h"
#define STOREBEST
#else
#include "bi2.h"
#endif

		a2=-a2;

		RESTORE_CARD(1);

		if (a2 > a1) {
#ifdef STOREBEST
			SET_BEST(1)
#endif

			a1 = a2;
			break;
		}
	}

