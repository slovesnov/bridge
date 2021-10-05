/*
 * SolveAll.h
 *
 *       Created on: 18.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef HELPER_SOLVEALL_H_
#define HELPER_SOLVEALL_H_

#include "../solver/BridgeCommon.h"

struct SolveAll {
	int k,n;
	CARD_INDEX first;
	int trump;
	CARD_INDEX p[2];
	CARD_INDEX cid[52];
	int o[26];
	gint64 id;//dialog id
	clock_t begin,end;
	int positions;


	//special for bridge
	bool ns;

	//special for preferans
	bool misere;
	CARD_INDEX preferansPlayer[3];
	CARD_INDEX player;

	SolveAll();
	~SolveAll();
	void init(int k, int n, CARD_INDEX first, int trump,
			CARD_INDEX p[2], CARD_INDEX cid[52], gint64 id);

	void operator=(SolveAll const&);
};

#endif /* HELPER_SOLVEALL_H_ */
