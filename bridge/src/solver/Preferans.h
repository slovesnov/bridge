/*
 * Preferans.h
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef PREFERANS_H_
#define PREFERANS_H_


#include "BridgePreferansBase.h"

#ifdef CONSOLE
	#include <cmath>
#else
	#include "../problem/Problem.h"
#endif

/* denote O1=MOVES_ONE_SUIT_OPTIONS
 * denote OM=MOVES_MANY_SUIT_OPTIONS
 * denote OM_NT=MOVES_MANY_SUIT_OPTIONS_NT
 * then
 *
 * 0 <= PREFERANS_ORDER_FIRST_MOVE < OM
 * 0 <= PREFERANS_ORDER_OTHER_MOVES < O1^2*OM
 * O1^2*OM power 2 because if player has no suit it must play trump card if he/she has it
 *
 * 0 <= PREFERANS_ORDER_FIRST_MOVE_NT < OM_NT
 * 0 <= PREFERANS_ORDER_OTHER_MOVES_NT < O1*OM_NT
 *
 * misere parameters bounds are the same with NT parameters
 * 0 <= PREFERANS_ORDER_FIRST_MOVE_MISERE < OM_NT
 * 0 <= PREFERANS_ORDER_OTHER_MOVES_MISERE < O1*OM_NT
 *
 */

//#define _O 3
//#define _O1 0
//#define _O2 2
//#define	PREFERANS_ORDER_OTHER_MOVES	((_O2*MOVES_ONE_SUIT_OPTIONS+_O1)*MOVES_ONE_SUIT_OPTIONS+_O)

#ifdef CONSOLE

extern int PREFERANS_ORDER_FIRST_MOVE;
extern int PREFERANS_ORDER_OTHER_MOVES;

extern int PREFERANS_ORDER_FIRST_MOVE_NT;
extern int PREFERANS_ORDER_OTHER_MOVES_NT;

extern int PREFERANS_ORDER_FIRST_MOVE_MISERE;
extern int PREFERANS_ORDER_OTHER_MOVES_MISERE;

#else//CONSOLE

const int PREFERANS_ORDER_FIRST_MOVE=0;
const int PREFERANS_ORDER_OTHER_MOVES=13;

const int PREFERANS_ORDER_FIRST_MOVE_NT=0;
const int PREFERANS_ORDER_OTHER_MOVES_NT=0;

const int PREFERANS_ORDER_FIRST_MOVE_MISERE=4;
const int PREFERANS_ORDER_OTHER_MOVES_MISERE=2;

#endif//CONSOLE


#ifndef FINAL_RELEASE
//Note. Not use NODE_COUNT macro because Bridge class has its own macro
//	#define PREFERANS_NODE_COUNT
#endif

class Preferans: public BridgePreferansBase {
	static const int8_t HASH_EXACT = 0;
	static const int8_t HASH_ALPHA = 1;
	static const int8_t HASH_BETA = 2;
	static const int8_t HASH_INVALID = 3;
	int m_andKey;
	int m_hashSize;
	int m_trump, m_cards, m_depth;
	int32_t m_code[4];
	static int32_t*m_r;

	/* start three elements 0 1 2
	 * for one trick need
	 * first move max=2 + 2 elements for next trick so need 5 elements for 1 trick
	 * need additional 2 elements for every trick, so for 9 additional tricks need 18 items
	 * max size 5+18=23
	 */
	static int m_w[23];

	bool* m_ct;

	/* int8_t*p=m_moves[m_code[suit]]+w*5;
	 * p[0] = length
	 * p[1..] = high -> low
	 */
	static int8_t (*m_moves)[3*5];

	static int m_oc;//object counter

	/* sizeof(HashItem)=8
	 *
	 * total size
	 * if(HASH_ITEMS==1){
	 * 	 Hash = HashItem
	 * 	 8*2^HASH_BITS/2^20=2^(HASH_BITS-17)mb
	 *
	 * 	 HASH_BITS=23 64mb //PREFERANS_HASH_TYPE=0
	 * 	 HASH_BITS=25 256mb //PREFERANS_HASH_TYPE=1
	 * }
	 * else{
	 * 	 sizeof(Hash)=sizeof(HashItem)*HASH_ITEMS+4=4*(1+2*HASH_ITEMS)
	 * 	 2^HASH_BITS*4*(1+2*HASH_ITEMS)/2^20mb=(1+2*HASH_ITEMS)*2^(HASH_BITS-18)
	 * 	 assert(hashBits>=18+1)
	 *
	 * 	 HASH_ITEMS=4 9*2^(HASH_BITS-18)
	 *
	 * 	 HASH_ITEMS=4 & HASH_BITS=21 9*2^3=72mb //PREFERANS_HASH_TYPE=2
	 * 	 HASH_ITEMS=4 & HASH_BITS=23 9*2^5=288mb //PREFERANS_HASH_TYPE=3
	 * }
	 */
#define PREFERANS_HASH_TYPE 3

#if PREFERANS_HASH_TYPE==0
	static const int HASH_BITS = 23;
#elif PREFERANS_HASH_TYPE==1
	static const int HASH_BITS = 25;
#elif PREFERANS_HASH_TYPE==2
	static const int HASH_ITEMS = 4;
	static const int HASH_BITS = 21;
#else
	static const int HASH_ITEMS = 4;
	static const int HASH_BITS = 23;
#endif

	#pragma pack (4)
	struct HashItem {
		int16_t code[3];
		int8_t f;
		int8_t v;
	};

#if PREFERANS_HASH_TYPE<2
	#define PREFERANS_ONE_HASH_ITEM
	using Hash = HashItem;
#else
	#pragma pack (4)
	struct Hash{
		HashItem i[HASH_ITEMS];
		int32_t next;
	};
#endif
	Hash*m_hashTable;


	int e(const int* w, int a, int b);
	int eb(const int* w, int a, int b);
	int eNT(const int* w, int a, int b);
	int ebNT(const int* w, int a, int b);
	int eMisere(const int* w, int a, int b);
	int ebMisere(const int* w, int a, int b);

	//find all suitable cards of one/two player(s) for game with trump
	void suitableCards(int suit, int w, SC& c);
	void suitableCards2(int suit, const int* w, SC& c1, SC& c2);

	//find all suitable cards of one/two player(s) for no trump and no misere game
	void suitableCardsNT(int suit, int w, SC& c);
	void suitableCards2NT(int suit, const int* w, SC& c1, SC& c2);

	//find all suitable cards of one/two player(s) for misere game
	void suitableCardsMisere(int suit, int w, SC& c);
	void suitableCards2Misere(int suit, const int* w, SC& c1, SC& c2);

#ifndef NDEBUG
	int getW(int suit, int pos);
#endif
	static void staticInit();
	static void staticDeinit();

	/* solve/solveb - find [only estimate]/[estimate+best move] for game with trump
	 * solveNT/solvebNT - the same for no misere game without trump
	 * solveMisere/solvebMisere - the same for misere game
	 */
	void solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solveNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solvebNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solveMisere(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);
	void solvebMisere(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);

public:

#ifdef PREFERANS_NODE_COUNT
	int m_nodes;
#endif

	int m_e; //estimate from player who do move
	int m_playerTricks; //estimate of player
	int m_best;
	std::vector<int> m_bestLine;

	/* if smallHash=true then one problem counts much faster 0.02 seconds
	 * but if smallHash=false the one problem counts about 0.3 seconds
	 * if smallHash=false then count solve_all_deals 184,756 positions much faster
	 */
	Preferans(bool smallHash=true);
	~Preferans();

	int whistersTricks(){
		return m_cards-m_playerTricks;
	}
	/* c [0-12 - spades A-2], [13-25 hearts A-2], [26-38 diamonds A-2], [39-51 clubs A-2]
	 * trump - like in bridge 0-spades, 1-hearts, 2-diamonds, 3-clubs, 4-NT
	 * firstmove
	 * player
	 * preferansPlayer for example {CARD_INDEX_NORTH,CARD_INDEX_EAST,CARD_INDEX_WEST };
	 */
	//estimate + best move
	void solveFull(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, bool misere, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);

	//only estimate
	void solveEstimateOnly(const CARD_INDEX c[52], int trump, CARD_INDEX first,
			CARD_INDEX player, bool misere, const CARD_INDEX preferansPlayer[3],
			bool trumpChanged);

	//	function works correct when table is full
	void bestLine(const CARD_INDEX c[52], CARD_INDEX first,
			CARD_INDEX player, bool misere, const CARD_INDEX preferansPlayer[3]);


#ifndef CONSOLE
	void solve(const Problem& p, bool trumpChanged);
	void estimateAll(const Problem& p, ESTIMATE estimateType,
			SET_ESTIMATION_FUNCTION estimationFunction);
#endif

#ifndef FINAL_RELEASE
	void printCode(int suit);
#endif

};

#endif /* PREFERANS_H_ */
