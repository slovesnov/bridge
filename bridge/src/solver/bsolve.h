/*
 * bsolve.h
 *
 *       Created on: 23.02.2021
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2021-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

m_trumpOriginal=trump;
m_ct=compareTable[trump==NT];

int i, j, k, l, m, n, a,t;
const CARD_INDEX* p;
USC sc0,sc1,sc2,sc3;
SC c1, c2, c3;
const int fi=first-1;
int y[4];

clock_t begin=clock();

USC* ps[]={&sc0,&sc1,&sc2,&sc3};
for(USC*p:ps){
	p->set(-1,-1);
}

for(i=0;i<4;i++){
	y[i] =(first-1+i) % 4 + 1;
}

n = 0;
for (i = 0; i < 4; i++) {
	m = 0;
	k = 4;
	p = c + getAdjustedTrump(i) * 13;
	a = 0;
	for (j = 0; j < 13; j++) {
		l = *p++;

		if (l == CARD_INDEX_ABSENT) {
			a++;
		}
		else {
			for (t = 0; t < 3; t++) {
				if (l == y[t] + 4) {
					l -= 4;
					ps[t]->set(j - a, i);
				}
			}
			assert(l>0);
			assert(l-1<4);
			m |= (l-1) << k;
			k += 2;
		}
	}
	k-=4;
	k/=2;
	n += k;
	m |= k;
	m_code[i] = m;

//	println("0x%x len%d",m_code[i]>>4,m_code[i]&15)
//	printCode(i);
//	printf("%d %d,\n",i,m);
}

//cards in each hand
m_depth = m_cards = n / 4;

if (trumpChanged) {
	Hash* t = m_hashTable;
	Hash* e = m_hashTable + HASH_SIZE;
	for (t = m_hashTable; t != e; t++) {
		t->next=0;
		for(auto&a:t->i){
			a.f = HASH_INVALID;
		}
	}
}

#ifdef BRIDGE_NODE_COUNT
m_nodes=0;
#endif

	if(m_depth==1){
#ifdef STOREBEST
		for (i = j = 0; i < 52; i++) {
			l=c[i];
			if (l >= 5) {
				l-=4;
				j++;
			}
			if(l!=0){
				ps[l-1]->fromIndex(i);
			}
		}
		for (i = 0; i < 52 && c[i] != y[j]; i++)
			;
		m_best = i;
#endif

		SETT
		i = t%2==j%2 ? 1:-1;
	}
	else{
		int low = 0, high = m_cards + 1, goal, alpha;
		if (lowTricks != DEFAULT_TRICKS) {
			low = lowTricks;
		}
		if (highTricks != DEFAULT_TRICKS) {
			high = highTricks + 1;
		}
		const int *w = m_w + fi;

#ifdef STOREBEST
		l = -1;
#endif

		if (sc0.s == -1) {
			//zero window search
			while (low + 1 < high) {
				goal = (low + high) / 2;
				alpha = 2 * goal - m_cards - 2;

				int a = alpha;

#ifdef STOREBEST
	#ifdef NO_TRUMP
		#define F ebNT
	#else
		#define F eb
	#endif
#else//STOREBEST
	#ifdef NO_TRUMP
		#define F eNT
	#else
		#define F e
	#endif
#endif//STOREBEST

				i = F(m_w + fi, a	);
//#ifdef NO_TRUMP
//printl(m_w[fi])
//#endif

#undef F

				if (i >= alpha + 2) {
					low = goal;
#ifdef STOREBEST
					l = m_best;
#endif
				}
				else {
#ifdef STOREBEST
					if (l == -1) {
						l = m_best;
					}
#endif
					high = goal;
				}
			}

		}
		else if (sc1.s == -1) {
		#ifdef NO_TRUMP
			suitableCards3NT
		#else
			suitableCards3
		#endif
			(sc0.s, w, c1, c2, c3);

#ifdef STOREBEST
			l = c1[0].toIndex();
#endif
			removeCard(sc0.s, sc0.c);

			//zero window search
			while (low + 1 < high) {
				goal = (low + high) / 2;
				alpha = 2 * goal - m_cards - 2;

				int a = -(alpha + 2);
				int a1 = alpha;

				#include "bi1.h"

				i = a1;

				if (i >= alpha + 2) {
					low = goal;
#ifdef STOREBEST
					l = m_best;
#endif
				}
				else {
					high = goal;
				}
			}

		}
		else if (sc2.s == -1) {
		#ifdef NO_TRUMP
			suitableCards3NT
		#else
			suitableCards3
		#endif
			(sc0.s, w, c1, c2, c3);

#ifdef STOREBEST
			l = c2[0].toIndex();
#endif
			removeCard(sc0.s, sc0.c);

			int r1 = sc1.c;

#define r0 sc0.c
			ADJUST_RANK(1, 0)
#undef r0
			REMOVE_CARD_NS(1)

			//zero window search
			while (low + 1 < high) {
				goal = (low + high) / 2;
				alpha = 2 * goal - m_cards - 2;

				int a2 = alpha;
				int a1 = -alpha - 2;
				#include "bi2.h"
				i = a2;
				if (i >= alpha + 2) {
					low = goal;
#ifdef STOREBEST
					l = m_best;
#endif
				}
				else {
					high = goal;
				}
			}

		}
		else {
		#ifdef NO_TRUMP
			suitableCards3NT
		#else
			suitableCards3
		#endif
			(sc0.s, w, c1, c2, c3);

#ifdef STOREBEST
			l = c3[0].toIndex();
#endif
			removeCard(sc0.s, sc0.c);

			int r1 = sc1.c;
#define r0 sc0.c
			ADJUST_RANK(1, 0)
#undef r0
			REMOVE_CARD_NS(1)

			int r2 = sc2.c;
#define r0 sc0.c
			ADJUST_RANK(2, 0)
			ADJUST_RANK(2, 1)
#undef r0
			REMOVE_CARD_NS(2)

			//zero window search
			while (low + 1 < high) {
				goal = (low + high) / 2;
				alpha = 2 * goal - m_cards - 2;

				int a3 = alpha;
				int a2 = -(alpha + 2);
				#include "bi3.h"
				i = a3;

				if (i >= alpha + 2) {
					low = goal;
#ifdef STOREBEST
					l = m_best;
#endif
				}
				else {
					high = goal;
				}
			}
		}
#ifdef STOREBEST
		m_best = l;
#endif
	}//else from if(m_depth==1)

	m_e = (m_cards + i) / 2;


	n = sc0.s == -1 ? 0 : (sc1.s == -1 ? 1 : (sc2.s == -1 ? 2 : 3) );
	m_ns= (fi+n)%2==0 ? m_e : m_cards-m_e;
	m_ew=m_cards-m_ns;

	printl(m_e,i,m_ns,m_ew,"fi",fi)

#ifdef STOREBEST
	//Note m_best taken from code, only if m_depth!=1
	if(m_depth==1){
//		printl(m_best/13,m_best%13,m_best);
	}
	else{
		adjustBestMove(c,true);
	}
#endif

	m_time=double(clock()-begin)/CLOCKS_PER_SEC;
