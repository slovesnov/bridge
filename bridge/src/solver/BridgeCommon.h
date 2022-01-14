/*
 * BridgeCommon.h
 *
 *       Created on: 28.11.2020 (29.11.2014)
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef SOLVER_BRIDGECOMMON_H_
#define SOLVER_BRIDGECOMMON_H_

#include <cstdarg>
#include "aslov.h"

const int NT = 4;
const int MISERE = NT + 1;
const int UNKNOWN_ESTIMATE = 100;
const int MAX_TABLE_CARDS[] = { 4, 3 };

enum ESTIMATE {
	ESTIMATE_NONE,
	ESTIMATE_BEST_LOCAL,
	ESTIMATE_BEST_TOTAL,
	ESTIMATE_ALL_LOCAL,
	ESTIMATE_ALL_TOTAL
};

enum CARD_INDEX {
	CARD_INDEX_INVALID = -1,
	CARD_INDEX_ABSENT,
	CARD_INDEX_NORTH,
	CARD_INDEX_EAST,
	CARD_INDEX_SOUTH,
	CARD_INDEX_WEST,
	CARD_INDEX_NORTH_INNER,
	CARD_INDEX_EAST_INNER,
	CARD_INDEX_SOUTH_INNER,
	CARD_INDEX_WEST_INNER
};

enum class EndgameType{
	ALL,NT,TRUMP
};

const char SUITS_CHAR[] = "shdcn";
const char RANK[] = "akqjt98765432";

typedef void (*SET_ESTIMATION_FUNCTION)(int index, int value);

//======================== BEGIN MACROS ===========================================================
#ifdef FINAL_RELEASE
#define START_TIMER ((void)0);
#define OUT_TIMER ((void)0);
#else
#define START_TIMER clock_t __begin=clock();
#define OUT_TIMER println("time%.3lf",double(clock()-__begin)/CLOCKS_PER_SEC);
#endif
//======================== END MACROS =============================================================

bool eastOrWest(CARD_INDEX i);
bool east(CARD_INDEX i);
bool west(CARD_INDEX i);
bool northOrSouth(CARD_INDEX i);
bool north(CARD_INDEX i);
bool south(CARD_INDEX i);
std::string binaryCodeString(int c, int miminumPrintBits = 0);

#endif /* SOLVER_BRIDGECOMMON_H_ */
