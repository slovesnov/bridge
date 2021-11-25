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
		VCardIndex p, CARD_INDEX cid[52],gint64 id,VInt const (&fixed)[2]) {
	//cann't use k it's parameter
	int i, j, l, m;
#define A(a) this->a=a;
	A(k)
	A(n)
	A(first)
	A(trump)
	A(id)
#undef A
#define A(a) for(i=0;i<SIZEI(this->a);i++){this->a[i]=a[i];}
	A(p)
	A(cid)
#undef A

#ifndef NDEBUG
	int c[2];
	for (j = 0; j < 2; j++) {
		c[j]=0;
	}
#endif
	l = 0;
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 52; i++) {
			m=cid[i];
			if (m == p[j]) {
				if(oneOf(i,fixed[j])){
#ifndef NDEBUG
					c[j]++;
#endif
				}
				else{
					o[l++] = i;
				}
			}
		}
	}
	vDealResult.clear();
	positions=0;

#ifndef NDEBUG
	for (j = 0; j < 2; j++) {
		assert(c[j]==int(fixed[j].size()));
	}
#endif

}

void SolveAll::copyParametersClearDealResult(SolveAll const& source){
	int i;
#define A(a) a=source.a;
	A(k)A(n)A(first)A(trump)A(misere)A(player)A(ns)A(id)A(positions)
#undef A

#define A(a) for(i=0;i<SIZEI(a);i++){a[i]=source.a[i];}
	A(p)A(cid)A(o)A(preferansPlayer)
#undef A
	vDealResult.clear();
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

