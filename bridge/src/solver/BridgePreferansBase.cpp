/*
 * BridgePreferansBase.cpp
 *
 *       Created on: 19.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "BridgePreferansBase.h"

#ifndef CONSOLE
gint BridgePreferansBase::m_stop;
#endif

bool BridgePreferansBase::compareTable[2][781 * 1024 + 781];

bool BridgePreferansBase::compareTableInited=false;

void BridgePreferansBase::staticInit() {
#ifndef CONSOLE
	g_atomic_int_set (&m_stop,0);
#endif

	if(compareTableInited){
		return;
	}
	compareTableInited=true;
	//fill table of comparison
	int s1, c1, s2, c2, t;
	for (t = 0; t < 2; t++){
		for (s1 = 0; s1 < 4; s1++){
			for (s2 = 0; s2 < 4; ++s2){
				for (c1 = 0; c1 < 13; c1++){
					for (c2 = 0; c2 < 13; c2++) {
						compareTable[t][(((s1 << 8) | c1) << 10) | (s2 << 8)
								| c2] =
								s1 == s2 ? c1 < c2 : s2 != (t == 0 ? 0 : NT);
					}
				}
			}
		}
	}

}

#ifndef CONSOLE
void BridgePreferansBase::addSuitableGroups(int suit, const CARD_INDEX*cid,
		const CARD_INDEX next, VSC&v, int best) {
	USC u;
	u.fromIndex(best);
	auto p = cid + suit * 13;
	int j;
	for(j=0;j<13 && p[j]!=next;j++);

	while(j<13){
		SC g;
		g.o=0;
		g.push(j,suit);
		if(suit==u.s && j==u.c){
			g.o=1;
		}
		for (j++; j < 13 && (p[j]==next || p[j]==0); j++) {
			if(p[j]!=0){
				g.push(j,suit);
				if(suit==u.s && j==u.c){
					g.o=1;
				}
			}
		}
		v.push_back(g);
		for (j++; j < 13 && p[j]!=next; j++);
	}
}
#endif

void BridgePreferansBase::adjustBestMove(const CARD_INDEX c[52],bool bridge){
	int i = m_best % 13;
	auto p = c + getAdjustedTrump(m_best / 13) * 13;
	int a = 0;
	int j;
	for (j = 0; j < (bridge ? 13 : 8); j++, p++) {
		if (*p == CARD_INDEX_ABSENT) {
			a++;
		}
		else {
			if (i-- == 0) {
				m_best += a;
				break;
			}
		}
	}

	adjustCard(m_best);
}

int BridgePreferansBase::getAdjustedTrump(const int i){
	if(m_trumpOriginal!=NT && m_trumpOriginal!=0 && (i==0 || i==m_trumpOriginal)){
		return i ? 0 : m_trumpOriginal;
	}
	else{
		return i;
	}
}

int BridgePreferansBase::getAdjustedCard(const int i){
	return i%13+getAdjustedTrump(i/13)*13;
}

void BridgePreferansBase::adjustCard(int& i){
	i=getAdjustedCard(i);
}

void BridgePreferansBase::adjustBestLine() {
	for(int&a:m_bestLine){
		adjustCard(a);
	}
}

bool BridgePreferansBase::compare2Cards(USC **ps,int i,int j) {
	return ps[i]->s==ps[j]->s ? ps[i]->c < ps[j]->c : ps[j]->s != m_trumpOriginal;
}

int BridgePreferansBase::getTaker(USC **ps, int size) {
	int t = 0;
	for (int i = 1; i < size; i++) {
		if (!compare2Cards(ps, t, i)) {
			t = i;
		}
	}
	return t;
}
