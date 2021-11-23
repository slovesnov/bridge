/*
 * SolveAll.cpp
 *
 *       Created on: 01.12.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "SolveAll.h"
#include "../base/Base.h"

SolveAll::SolveAll() {
	g_mutex_init(&dealsMutex);
}

SolveAll::~SolveAll() {
	g_mutex_clear(&dealsMutex);
}

void SolveAll::init(int k, int n, CARD_INDEX first, int trump,
		CARD_INDEX p[2], CARD_INDEX cid[52], gint64 id) {
	int i,j,l;
#define A(a) this->a=a;
	A(k)A(n)A(first)A(trump)A(id)
#undef A
#define A(a) for(i=0;i<SIZEI(this->a);i++){this->a[i]=a[i];}
	A(p)A(cid)
#undef A
	l = 0;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 52; i++) {
			if (cid[i] == p[j]) {
				o[l++] = i;
			}
		}
	}
	positions=0;

}

void SolveAll::copyParameters(SolveAll const& source){
	int i;
#define A(a) a=source.a;
	A(k)A(n)A(first)A(trump)A(misere)A(player)A(ns)A(id)A(positions)
#undef A

#define A(a) for(i=0;i<SIZEI(a);i++){a[i]=source.a[i];}
	A(p)A(cid)A(o)A(preferansPlayer)
#undef A
}

void SolveAll::addDealResult(const DealResult &deal) {
	g_mutex_lock(&dealsMutex);
	vDealResult.push_back(deal);
	g_mutex_unlock(&dealsMutex);
}

void SolveAll::add(VDealResult &v) {//v=v+deals
	g_mutex_lock(&dealsMutex);
	v.insert(v.end(), vDealResult.begin(), vDealResult.end());
	g_mutex_unlock(&dealsMutex);
}

int SolveAll::dealResultSize() {
	g_mutex_lock(&dealsMutex);
	int i=vDealResult.size();
	g_mutex_unlock(&dealsMutex);
	return i;
}

