/*
 * bbest.h
 *
 *       Created on: 23.02.2021
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2021-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

#ifdef STOREBEST

#ifdef NEW_MOVES_ORDER
		int k,l,m,n;
		int8_t*z;

#ifdef NO_TRUMP
	#define O BRIDGE_ORDER_FIRST_MOVE_NT
#else
	#define O BRIDGE_ORDER_FIRST_MOVE
#endif

#define w w[0]
#define SKIP_SUITS 0
#include "moves_many_suits.h"
#undef SKIP_SUITS
#undef w
#undef O
		m_best = c[0].toIndex();

#else// NEW_MOVES_ORDER
		//for bestLine search could return here, need to count move
		for (i = 0; i < 4; i++) {
			suitableCardsOneSuit(i, w[0], c);
			if(c.length!=0){
				m_best = c[0].toIndex();
				break;
			}
		}
#endif// NEW_MOVES_ORDER

#endif//STOREBEST

