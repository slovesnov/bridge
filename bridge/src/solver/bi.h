/*
 * bi.h
 *
 *       Created on: 14.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

	int i, rc0, a1;
#if !defined(STOREBEST) || BRIDGE_MAX_PRECOUNT_SUIT_CARDS==11
	int j;
#endif

#ifndef CONSOLE
		if(m_depth == 8 && g_atomic_int_get (&m_stop)){
//			println("user break %llx base%llx",uint64_t(this),uint64_t(gBase));
			//gBase=0 estimateAll wasn't called
			if(gBase && this==gBase){
				finishEstimateAll();
			}
			g_thread_exit (0);
		}
#endif

	USC sc0;

	const int b=a+2;

#ifdef BRIDGE_NODE_COUNT
	m_nodes++;
#endif

#ifndef CUT4LASTLAYERS
	if (m_depth == 1) {
		int c, t,l;
		USC sc1,sc2,sc3;

		for (i = 0; i < 4; i++) {
			l=m_code[i]&15;
			for (j = 0, c = m_code[i]>>4; j<l; c >>= 2, j++) {
				t = c & 3;
				if (t == w[0]) {
					sc0.set(j,i);
				}
				else if (t == w[1]) {
					sc1.set(j,i);
				}
				else if (t == w[2]) {
					sc2.set(j,i);
				}
				else {
					sc3.set(j,i);
				}
			}
		}
		SETT;
		return t%2==0 ?	1 : -1;
	}
#endif //CUT4LASTLAYERS

	SC c;

	if (a >= m_depth) {
		#include "bbest.h"
		return a;
	}
	if (	b <= -m_depth	) {
		#include "bbest.h"
		return b;
	}

/*
25nov2020 never happens, with zero window search
	if (a < -m_depth) {
		a = -m_depth;
	}
	if (b > m_depth) {
		b = m_depth;
	}
*/

	//probeHash
#ifndef STOREBEST
/*
	hashBits=24bits
	shift>=2 (because of w)
	m_code[3]<<shift
	shift=0 highest24bit
	shift=1 highest23bit=24-shift
	max highest=16
	24-shift<=16
	shift<=8

	so shift of m_code[3] should be >=2 & <=8
*/

	i=HASH_KEY;
//			i = (((m_code[0] +m_code[1]) << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w[0])
//					& m_andKey;

//original
//		i = ((m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w[0])
//				& m_andKey;

//	i = ( (m_code[0] *m_code[1]+ m_code[2]) ^ (m_code[3]>>16) ^ (m_code[3] << 3) ^ w[0])
//			& m_andKey;
//from BridgePosition #define KeyZob ((code[0]*code[1]+code[2])^_lowcode)

//	i = ((m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w[0])
//			& m_andKey;

		Hash& he = m_hashTable[i];
		for(j=0;j<HASH_ITEMS;j++){
			HashItem& h = he.i[j];
			if (h.f != HASH_INVALID && (m_code[3]>>16) == h.code3 ) {
				for (i = 0; i < 3 && h.code[i] == m_code[i]; i++)
					;
				if (i == 3) {
					if (h.f == HASH_ALPHA && h.v <= a) {
						return a;
					}
					if (h.f == HASH_BETA && h.v >= b) {
						return b;
					}
				}
			}

		}

#endif
		SC c1, c2,c3;
		int jj;

#ifdef NEW_MOVES_ORDER

		int k,l,m,n;
		int8_t*z;
		int8_t m_mi[MAX_MOVES];

#ifdef NT
	#define O BRIDGE_ORDER_FIRST_MOVE_NT
#else
	#define O BRIDGE_ORDER_FIRST_MOVE
#endif
#define w w[0]
#define SKIP_SUITS 0
#include "moves_many_suits.h"
#undef SKIP_SUITS
#undef w
#undef O

#else//NEW_MOVES_ORDER

	SC p[4];
	for (i = 0; i < 4; i++) {
		suitableCardsOneSuit(i, w[0], p[i]);
	}

	//this order different with old version but it's faster for file GeorgeCoffin.bts 1-68 total 14.93
	for (auto const& t : p) {
		if (t.length ==1) {
			c.push(t[0]); //highest card in suit
		}
		else if(t.length >1) {
			c.push(t[0]); //highest card in suit
			c.push(t[t.length-1]); //lowest
		}
	}
	for (auto const& t : p){
		for (j = 1; j < t.length-1; j++) { //all others
			c.push(t[j]);
		}
	}
#endif//NEW_MOVES_ORDER

#ifdef STOREBEST
	m_best = c[0].toIndex();
#endif

	//became slower
//#define USE_PRESUITS

#ifdef USE_PRESUITS
	bool fo[]={false,false,false,false};
	SC fc[4][3];
	SC*pfc;
#else
	jj=-1;
#endif

	for (i = 0; i < c.length; i++) {
		sc0=c[i];
#ifdef USE_PRESUITS
		pfc=fc[sc0.s];
		if(fo[sc0.s]){
			c1.copy(*pfc++);
			c2.copy(*pfc++);
			c3.copy(*pfc);
		}
		else{
			c1.length = c2.length =  c3.length = 0;
			suitableCards3(sc0.s, w, c1, c2,c3);
			pfc->copy(c1);pfc++;
			pfc->copy(c2);pfc++;
			pfc->copy(c3);
			fo[sc0.s]=true;
		}
#else
		if(jj!=sc0.s){
			c1.length = c2.length =  c3.length = 0;

		#ifdef NT
			suitableCards3NT
		#else
			suitableCards3
		#endif
			(sc0.s, w, c1, c2,c3);

			jj=sc0.s;
		}
#endif

#undef USE_PRESUITS

#define r0 sc0.c
		REMOVE_CARD(0)
#undef r0

		a1 = -b;

#ifdef STOREBEST
#undef STOREBEST
#include "bi1.h"
#define STOREBEST
#else
#include "bi1.h"
#endif

		a1=-a1;

		RESTORE_CARD(0)

		if (a1 > a) {
#ifdef STOREBEST
			SET_BEST(0)
#endif
			a = a1;
#ifndef STOREBEST
				RECORD_HASH(b, HASH_BETA);
#endif
			return b;
		}
	}

#ifndef STOREBEST
	RECORD_HASH(a, HASH_ALPHA);
#endif

	return a;
