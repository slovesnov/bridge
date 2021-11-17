/*
 * psolve.h
 *
 *       Created on: 08.10.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */


//Note. Inner representation player always=0
#ifdef TRUMP_INNNER0
	m_trumpOriginal=trump;
#else
	m_trump = trump;
#endif

#ifdef COMPARE_TABLE_2
	//cann't use NT variable because has macro with same name
	m_ct=compareTable[trump==4];
#else
	m_ct=compareTable[m_trump];
#endif

int a, i, j, k, l,m, n, pi[CARD_INDEX_WEST + 1], fi;
CARD_INDEX first1;
const CARD_INDEX *p;
USC sc0,sc1;

sc0.set(-1,-1);
sc1.set(-1,-1);

for (i = 0; i < 3 && preferansPlayer[i] != player; i++)
;
for (j = 0; j < 3; j++, i++) {
	pi[preferansPlayer[i % 3]] = j;
}
fi = pi[first];

for (i = 0; i < 3 && preferansPlayer[i] != first; i++)
;
first1 = preferansPlayer[(i + 1) % 3];

n = 0;
for (i = 0; i < 4; i++) {
	m = 0;
	k = 0;
#ifdef TRUMP_INNNER0
	p = c + swapTrumpIfNeeded(i) * 13;
#else
	p = c + i * 13;
#endif
	a = 0;
	for (j = 0; j < 8; j++) {
		l = *p++;

		if (l == CARD_INDEX_ABSENT) {
			a++;
		}
		else {
			if (l == first + 4) {
				l-=4;
				sc0.set(j - a,i);
			}
			else if (l == first1 +4) {
				l-=4;
				sc1.set(j - a,i);
			}
			m |= pi[l] << k;
			k += 2;
		}
	}
	n += k;
	m |= 3 << k;
	m_code[i] = m;
}

//cards in each hand
m_depth = m_cards = n / 6;

if (trumpChanged) {
	Hash* t;
	Hash* e = m_hashTable + m_hashSize;
	for (t = m_hashTable; t != e; t++) {
#ifdef PREFERANS_ONE_HASH_ITEM
		t->f = HASH_INVALID;
#else
		t->next=0;
		for(auto&a:t->i){
			a.f = HASH_INVALID;
		}
#endif
	}//for(t)
}
#ifdef PREFERANS_NODE_COUNT
m_nodes=0;
#endif
#ifndef NDEBUG
m_best = -1;
#endif
if (sc0.s == -1 && m_cards != 1) {
#ifdef MISERE
	#ifdef STOREBEST
		#define F ebMisere
	#else
		#define F eMisere
	#endif
#elif defined(NT)
	#ifdef STOREBEST
		#define F ebNT
	#else
		#define F eNT
	#endif
#else
	#ifdef STOREBEST
		#define F eb
	#else
		#define F e
	#endif
#endif

	i = F(m_w+fi, -m_cards, m_cards);

#undef F
}
else {
	int b=m_cards;
	int a=-b;
	int v;
	SC c1, c2;
	int a1, b1, a2, b2;

	const int *w=m_w+fi;

	if (m_cards == 1) {
		int i, j, t, c;

		//cards on table
		int n = sc0.s == -1 ? 0 : (sc1.s == -1 ? 1 : 2);
		USC sc1,sc2;

		for (i = 0; i < 4; i++) {
			for (j = 0, c = m_code[i]; c != 3; c >>= 2, j++) {
				t = c & 3;
				if (t == w[0]) {
					sc0.set(j,i);
					if(n==0){
						m_best=sc0.toIndex();
					}
				}
				else if (t == w[1]) {
					sc1.set(j,i);
					if(n==1){
						m_best=sc1.toIndex();
					}
				}
				else {
					sc2.set(j,i);
					if(n==2){
						m_best=sc2.toIndex();
					}
				}
			}
		}

		SETT;

		//taker n or neither t nor n are preferans players
		v=  t == n || (w[t] != 0 && w[n] != 0) ? 
#ifdef MISERE
			-1:1
#else
			 1:-1
#endif
			;
	}
else{
	if (sc1.s == -1) {
#ifdef MISERE
	suitableCards2Misere
#elif defined(NT)
	suitableCards2NT
#else
	suitableCards2
#endif
		(sc0.s, w, c1, c2);

		m_best = c1[0].toIndex();
#define r0 sc0.c
		REMOVE_CARD_NS(0)
#undef r0

		a1 = a;
		b1 = b;
		#include "pi1.h"
		v = a1;

	}
	else {
#ifdef MISERE
	suitableCardsMisere
#elif defined(NT)
	suitableCardsNT
#else
	suitableCards
#endif
		(sc0.s, w[2], c2);

		m_best = c2[0].toIndex();
		int r1 = sc1.c;
#define r0 sc0.c
		REMOVE_CARD_NS(0)
		ADJUST_RANK(1,0)
#undef r0

		assert(getW(sc1.s, r1) == w[1]);
		REMOVE_CARD_NS(1)

		a2 = a;
		b2 = b;
		#include "pi2.h"
		v = a2;

	}
}
	i=v;
}

#ifdef MISERE
m_e = (m_cards - i) / 2;
#else
m_e = (m_cards + i) / 2;
#endif

//cards on table
n = sc0.s == -1 ? 0 : (sc1.s == -1 ? 1 : 2);
m_playerTricks = (fi + n) % 3 == 0 ? m_e : m_cards - m_e;

#ifdef STOREBEST
//Note m_best always taken from code, so need to adjust always
	adjustBestMove(c,m_best,false);
#endif
