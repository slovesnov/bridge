/*
 * moves_one_suit.h
 *
 *       Created on: 07.02.2021
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2021-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.rf.gd
 */

/* O should be defined
 total MOVES_ONE_SUIT_OPTIONS options

 0 high->low
 1 low, high->low
 2 high, low, high->low

 3 low->high
 4 high, low->high
 5 low, high, low->high

 */

#ifndef NO_DECLARATION
int8_t *p, *q;
#endif
#ifdef BRIDGE_H_
	int8_t _mm[MAX_MOVES];
#endif

#ifdef BRIDGE_H_
	MOVES_INIT_BRIDGE(suit,w,p,q,_mm)
#else
MOVES_INIT_PREFERANS(suit,w,p,q)
#endif

if( O==1 || O==5) {
	MOVES_LOW(suit,w,c,p,q)
}
else if( O==2 || O==4) {
	MOVES_HIGH(suit,w,c,p,q)
}

if (O==2) {
	MOVES_LOW(suit,w,c,p,q)
}
else if( O==5) {
	MOVES_HIGH(suit,w,c,p,q)
}

if (O < MOVES_ONE_SUIT_OPTIONS/2) {
	MOVES_HIGH_LOW_OTHERS(suit,w,c,p,q)
}
else {
	MOVES_LOW_HIGH_OTHERS(suit,w,c,p,q)
}
