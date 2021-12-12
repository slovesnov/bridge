/*
 * Bridge.h
 *
 *       Created on: 10.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef BRIDGE_H_
#define BRIDGE_H_

#include <cmath>

#include "BridgePreferansBase.h"
#ifndef CONSOLE
#include "../problem/Problem.h"
#endif

//TODO NEW_MOVES_ORDER always defined?
#define NEW_MOVES_ORDER

/* denote O1=MOVES_ONE_SUIT_OPTIONS
 * denote OM=MOVES_MANY_SUIT_OPTIONS
 * denote OM_NT=MOVES_MANY_SUIT_OPTIONS_NT
 * then
 *
 * 0 <= BRIDGE_ORDER_FIRST_MOVE < OM
 * 0 <= BRIDGE_ORDER_OTHER_MOVES < O1*OM
 *
 * 0 <= BRIDGE_ORDER_FIRST_MOVE_NT < OM_NT
 * 0 <= BRIDGE_ORDER_OTHER_MOVES_NT < O1*OM_NT
 */

#ifdef CONSOLE
/* order variables defined in main.cpp file.
 * It needs to find optimal order parameters
 */

extern int BRIDGE_ORDER_FIRST_MOVE;
extern int BRIDGE_ORDER_OTHER_MOVES;

extern int BRIDGE_ORDER_FIRST_MOVE_NT;
extern int BRIDGE_ORDER_OTHER_MOVES_NT;

#else//CONSOLE

//const int BRIDGE_ORDER_FIRST_MOVE = 8;//old
//const int BRIDGE_ORDER_OTHER_MOVES = 0;//old
const int BRIDGE_ORDER_FIRST_MOVE = 2;
const int BRIDGE_ORDER_OTHER_MOVES = 6;

const int BRIDGE_ORDER_FIRST_MOVE_NT = 0;
const int BRIDGE_ORDER_OTHER_MOVES_NT = 36;

#endif//CONSOLE

#define BRIDGE_MAX_PRECOUNT_SUIT_CARDS 11

#ifdef FINAL_RELEASE

#else//FINAL_RELEASE
//Note. Not use NODE_COUNT macro because Preferans class has its own macro
//	#define BRIDGE_NODE_COUNT
#endif

/* option BRIDGE_NODE_COUNT not tested at all
 * */

class Bridge: public BridgePreferansBase {
	static const int8_t HASH_ALPHA = 1;
	static const int8_t HASH_BETA = 2;
	static const int8_t HASH_INVALID = 3;

	/*
	 * HASH_ITEMS=4
	 * hashBits=22
	 * sizeof(HashItem)=16
	 * sizeof(Hash)=sizeof(HashItem)*HASH_ITEMS+4=68 //+4 because of next
	 * size of memory 68*2^hashBits
	 * in megabytes 68*2^(hashBits-20)=68*4=272mb
	*/
	//HASH_ITEMS=4 hashBits = 22
	static const int HASH_ITEMS=4;//HASH_ITEMS=2^n=2^2
	static const int HASH_BITS = 24-int(log2(HASH_ITEMS));//24-n
	static_assert( 1<<int(log2(HASH_ITEMS)) == HASH_ITEMS);
	static const int HASH_SIZE = 1 << HASH_BITS;
	static const int AND_KEY = HASH_SIZE - 1;
	static_assert(HASH_BITS>=18);
	int m_cards, m_depth;
	int m_code[4];
	/* start four elements 0 1 2 3
	 * for one trick need
	 * first move max=3 + 3 elements for next trick so need 7 elements for 1 trick
	 * need additional 3 elements for every trick, so for 12 additional tricks need 36 items
	 * max size 7+36=43
	 */
	static int m_w[43];

	static const int MAX_MOVES =8;//1 byte for length+up to seven bytes for moves, need for MOVES_INIT macro (defined in Bridge.cpp) see bi.h
	static int8_t **m_moves;

	int8_t m_mi[MAX_MOVES];

	/* cann't use -2, because from bestline sometimes calls [-2, 0]
	 *
	 */
	static const int DEFAULT_TRICKS=-30;
#ifdef BRIDGE_NODE_COUNT
	int m_nodes;
#endif

	static int m_oc;//object counter

	struct HashItem {//2^4=16 bytes, assume structure alignment=4
		int32_t code[3];
		int16_t code3;
		int8_t f;
		int8_t v;
	};

	struct Hash{
		HashItem i[HASH_ITEMS];
		int32_t next;
	}*m_hashTable;

	int e(const int* w, int a);
	int eb(const int* w, int a);
	int eNT(const int* w, int a);
	int ebNT(const int* w, int a);

	int ep(const int* w, int a);

#ifndef NEW_MOVES_ORDER
	void suitableCardsOneSuit(int suit, int w, SC& a);
	void suitableCardsInverseOneSuit(int suit, int w, SC& a);
#endif

	//find all suitable cards of one/three player(s) for game with trump
	void suitableCards(int suit, int w, SC& c);
	void suitableCards3(int suit, const int* w, SC& c1, SC& c2, SC& c3);

	//find all suitable cards of one/three player(s) for no trump game
	void suitableCardsNT(int suit, int w, SC& c);
	void suitableCards3NT(int suit, const int* w, SC& c1, SC& c2, SC& c3);

	int removeCard(int suit, int pos);

#ifndef FINAL_RELEASE
	void printCode(int suit);
#endif

	static void staticInit();
	static void staticDeinit();

	void solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS, int highTricks =
					DEFAULT_TRICKS);
	void solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS, int highTricks =
					DEFAULT_TRICKS);
	void solveNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS, int highTricks =
					DEFAULT_TRICKS);
	void solvebNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS, int highTricks =
					DEFAULT_TRICKS);

public:

	int m_e; //estimate from player who do move
	int m_ns,m_ew;
	double m_time;//seconds do not remove need for estimateAllInner function

	Bridge();
	~Bridge();

	//c [0-12 - spades A-2], [13-25 hearts A-2], [26-38 diamonds A-2], [39-51 clubs A-2]
	//estimate + best move
	void solveFull(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS,
			int highTricks = DEFAULT_TRICKS);

	//only estimate
	void solveEstimateOnly(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			bool trumpChanged, int lowTricks = DEFAULT_TRICKS,
			int highTricks = DEFAULT_TRICKS);

	void bestLine(const CARD_INDEX c[52], CARD_INDEX first);

#ifndef CONSOLE

//	void estimateAllMultithread(const Problem& p,ESTIMATE estimateType,
//			SET_ESTIMATION_FUNCTION estimationFunction,bool trumpChanged);

	void solve(const Problem& p, bool trumpChanged);
	void estimateAll(const Problem& p, ESTIMATE estimateType,
			SET_ESTIMATION_FUNCTION estimationFunction,bool beforeBest,bool trumpChanged);

	void estimateAllInner(const Problem& p, ESTIMATE estimateType,
			SET_ESTIMATION_FUNCTION estimationFunction,bool beforeBest,bool trumpChanged);

	void estimateAllThread();
	static void finishEstimateAll();

	//for estimateAll function
	bool m_estimateTrumpChanged;
#ifndef FINAL_RELEASE
	int m_threadIndex;
#endif

#endif

};

#endif /* BRIDGE_H_ */
