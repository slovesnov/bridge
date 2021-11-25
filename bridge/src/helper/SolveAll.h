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
#include "DealResult.h"

class SolveAll {
	VDealResult vDealResult;
	GMutex dealsMutex;
public:
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
	SolveAll(SolveAll&&) = default;
	void init(int k, int n, CARD_INDEX first, int trump,
			CARD_INDEX p[2], CARD_INDEX cid[52]);

	SolveAll(SolveAll const&)=delete;
	void operator=(SolveAll const&)=delete;
	void copyParameters(SolveAll const& source);

	void addDealResult(DealResult const& deal);
	void add(VDealResult& v);
	int dealResultSize();
};

#endif /* HELPER_SOLVEALL_H_ */
