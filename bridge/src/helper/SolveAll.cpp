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
}

SolveAll::~SolveAll() {
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

void SolveAll::operator=(SolveAll const& s){
#define A(a) a=s.a;
	A(k)A(n)A(first)A(trump)A(misere)A(player)A(ns)A(id)A(positions)
#undef A

		int i;

#define A(a) for(i=0;i<SIZEI(a);i++){a[i]=s.a[i];}
	A(p)A(cid)A(o)A(preferansPlayer)
#undef A

}



