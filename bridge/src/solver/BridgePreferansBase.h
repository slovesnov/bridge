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

#include <vector>

#include "BridgeCommon.h"
#include "SC.h"

typedef std::vector<int> VInt;
typedef std::vector<VInt> VVInt;

//BEGIN common macros for bridge and preferans
const int MOVES_ONE_SUIT_OPTIONS=6;
const int MOVES_MANY_SUITS_OPTIONS_NT=10;
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

class BridgePreferansBase {
	int getAdjustedCard(const int i);
protected:
	static const int8_t HASH_EXACT = 0;
	static const int8_t HASH_ALPHA = 1;
	static const int8_t HASH_BETA = 2;
	static const int8_t HASH_INVALID = 3;

	static void staticInit();

	static bool compareTableInited;
	const bool* m_ct;

	static bool compareTable[2][781 * 1024 + 781];

	int m_trumpOriginal;
	static const int m_trump=0;

	int getAdjustedTrump(const int i);
	void adjustCard(int& i);

#ifndef CONSOLE
	//add all cards in suit for player=next to vector v, each v[i] is a card sequence
	void static addSuitableGroups(int suit, const CARD_INDEX*cid,
			const CARD_INDEX next, VSC&v, int best);

#endif

	void adjustBestMove(const CARD_INDEX c[52], bool bridge);
	bool compare2Cards(USC **ps, int i, int j);
	int getTaker(USC **ps, int size);

public:
	//endgame function (need also for test project)
	static VVInt suitLengthVector(bool bridge,EndgameType option);
	static int endgameGetN(bool bridge,bool total=false);
	static VVInt suitLengthVector(const int n,bool bridge,EndgameType option);
	static int endgameCm(bool bridge);
	static int endgameCm(const int n,bool bridge);
	static int bitCode(bool bridge, VInt const &p0, VInt const &p1, VInt const &p2);
	static void endgameRotate(bool bridge,const int mw[],int n,int bits,int a[]);
	static int getMinBijectionMultiplier(bool bridge);


	static void endgameInit(bool bridge,
			int32_t* endgameLength[],
			int32_t* endgameIndex[],
			int8_t* endgameEstimate[],
		#ifndef NDEBUG
			int endgameEstimateLength[],
		#endif
			const int endgameMultiplier,
			const int endgameTypes,
			const int mw[]
			);

	int m_best;
	std::vector<int> m_bestLine;

#ifndef CONSOLE
	static gint m_stop;
#endif

};

#endif /* SOLVER_BRIDGEPREFERANSBASE_H_ */
