/*
 * BridgePreferansBase.h
 *
 *       Created on: 19.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef SOLVER_BRIDGEPREFERANSBASE_H_
#define SOLVER_BRIDGEPREFERANSBASE_H_

#include "BridgeCommon.h"
#include "SC.h"

//BEGIN common macros for bridge and preferans
const int MOVES_ONE_SUIT_OPTIONS=6;
const int  MOVES_MANY_SUITS_OPTIONS_NT=10;
//TODO
const int MOVES_MANY_SUITS_OPTIONS=10;

//MOVES_INIT macros are different for Bridge and Preferans games and defined in Bridge.cpp & Preferans.cpp

//lowest card in suit
#define MOVES_LOW(suit,w,c,p,q) if(p>=q){c.push(*p--,suit);}
//highest card in suit
#define MOVES_HIGH(suit,w,c,p,q) if(p>=q){c.push(*q++,suit);}
//all recent moves from low to high
#define MOVES_LOW_HIGH_OTHERS(suit,w,c,p,q) while(p>=q){c.push(*p--,suit);}
//all recent moves from high to low
#define MOVES_HIGH_LOW_OTHERS(suit,w,c,p,q) while(p>=q){c.push(*q++,suit);}

//same macros with initialization
//#define MOVES_LOW_I(suit,w,c,p,q) MOVES_INIT(suit,w,p,q)MOVES_LOW(suit,w,c,p,q)
//#define MOVES_HIGH_I(suit,w,c,p,q) MOVES_INIT(suit,w,p,q)MOVES_HIGH(suit,w,c,p,q)
//#define MOVES_LOW_HIGH_OTHERS_I(suit,w,c,p,q) MOVES_INIT(suit,w,p,q)MOVES_LOW_HIGH_OTHERS(suit,w,c,p,q)
//#define MOVES_HIGH_LOW_OTHERS_I(suit,w,c,p,q) MOVES_INIT(suit,w,p,q)MOVES_HIGH_LOW_OTHERS(suit,w,c,p,q)
//END common macros for bridge and preferans

#define TRUMP_INNNER0
#define COMPARE_TABLE_2

class BridgePreferansBase {
protected:
	static void staticInit();

	static bool compareTableInited;
	bool* m_ct;

#ifdef COMPARE_TABLE_2
	static const int compareTableSize=2;
#else
	static const int compareTableSize=5;
#endif
	static bool compareTable[compareTableSize][781 * 1024 + 781];

#ifdef TRUMP_INNNER0
	int m_trumpOriginal;
	static const int m_trump=0;
#else
	int m_trump;
#endif
	int adjustTrump(const int i);

#ifndef CONSOLE
	//add all cards in suit for player=next to vector v, each v[i] is a card sequence
	void static addSuitableGroups(int suit, const CARD_INDEX*cid,
			const CARD_INDEX next, VSC&v, int best);

#endif

	void adjustBestMove(const CARD_INDEX c[52], int& best, bool bridge);
public:
#ifndef CONSOLE
	static gint m_stop;
#endif

};

#endif /* SOLVER_BRIDGEPREFERANSBASE_H_ */
