
	int i, rc0, a1, b1;
	USC sc0;
#if !defined(STOREBEST)
	int j;
#endif


#ifdef PREFERANS_NODE_COUNT
	m_nodes++;
#endif

#ifndef CUT3LASTLAYERS
	if (m_depth == 1) {
		int c,t;
		USC sc1,sc2;

		for (i = 0; i < 4; i++) {
			for (j = 0, c = m_code[i]; c != 3; c >>= 2, j++) {
				t = c & 3;
				if (t == w[0]) {
					sc0.set(j,i);
				}
				else if (t == w[1]) {
					sc1.set(j,i);
				}
				else {
					sc2.set(j,i);
				}
			}
		}

		SETT;

		return t == 0 || (t == 1 && w[2] == 0) || (t == 2 && w[1] == 0) ?
#ifdef MISERE
		-1 : 1
#else
		1 : -1
#endif
		;
	}
#endif //CUT3LASTLAYERS

	if (a >= m_depth) {
		return a;
	}
	if (b <= -m_depth) {
		return b;
	}

	if (a < -m_depth) {//can happens, but in bridge never, may be because of only zero window in bridge is used
		a = -m_depth;
	}
	if (b > m_depth) {//can happens, but in bridge never, may be because of only zero window in bridge is used
		b = m_depth;
	}

	//probeHash


#ifndef STOREBEST

	//original
	//	i = ((m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w[0])
	//			& m_andKey;

	/* Note m_code[3] is stored in index so should be i=...^(m_code[3] << v)
	 * where v>=2 && 16+v<=HASH_BITS
	 */
#if PREFERANS_HASH_TYPE==0
	//total 23 bits
	i = (m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3);

#elif PREFERANS_HASH_TYPE==1
	//total 25 bits
	i = ( (m_code[0]  ^ m_code[1]) << 12) ^ m_code[2] ^ (m_code[3] << 3);

#elif PREFERANS_HASH_TYPE==2
	//total 21 bits
	i = (m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3);

#else
	//total 23 bits
	i = (m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 3);

	//i = (m_code[0] << 9) ^ (m_code[1] << 6) ^ m_code[2] ^ (m_code[3] << 2);

	//i = ((m_code[0]*m_code[1]) << 6) ^ m_code[2] ^ (m_code[3] << 2);
#endif
	i = (i ^ w[0]) & m_andKey;

#ifdef PREFERANS_ONE_HASH_ITEM
	Hash& h = m_hashTable[i];
#else
	Hash& he = m_hashTable[i];
	for(j=0;j<HASH_ITEMS;j++){
		HashItem& h = he.i[j];
#endif
	if (h.f != HASH_INVALID) {
		//int16_t(m_code[i]) clear two high bits if cards in suit0=8
		for (i = 0; i < 3 && h.code[i] == int16_t(m_code[i]); i++)
			;
		if (i == 3) {
			if (h.f == HASH_EXACT) {
				return h.v;
			}
			if (h.f == HASH_ALPHA && h.v <= a) {
				return a;
			}
			if (h.f == HASH_BETA && h.v >= b) {
				return b;
			}
		}
	}

#ifdef PREFERANS_ONE_HASH_ITEM
#else
	}
#endif
	int8_t f = HASH_ALPHA;
#endif

	SC c;

#ifdef MISERE
	#define O PREFERANS_ORDER_FIRST_MOVE_MISERE
#elif defined(NO_TRUMP)
	#define O PREFERANS_ORDER_FIRST_MOVE_NT
#else
	#define O PREFERANS_ORDER_FIRST_MOVE
#endif
#define w w[0]
#define SKIP_SUITS 0
#include "moves_many_suits.h"
#undef SKIP_SUITS
#undef w
#undef O

#ifdef STOREBEST
	m_best = c[0].toIndex();
#endif

	SC c1, c2;
	for (i = 0; i < c.length; i++, c1.length = c2.length = 0) {
		sc0 = c[i];

#ifdef MISERE
		suitableCards2Misere
#elif defined(NO_TRUMP)
		suitableCards2NT
#else
		suitableCards2
#endif
		(sc0.s, w, c1, c2);


#define r0 sc0.c
		REMOVE_CARD(0)
#undef r

		if (w[2] == 0) {
			a1 = a;
			b1 = b;
#ifdef STOREBEST
#undef STOREBEST
#include "pi1.h"
#define STOREBEST
#else
#include "pi1.h"
#endif
		}
		else {
			a1 = -b;
			b1 = -a;
#ifdef STOREBEST
#undef STOREBEST
#include "pi1.h"
#define STOREBEST
#else
#include "pi1.h"
#endif
			a1 = -a1;
		}

		RESTORE_CARD(0)

		if (a1 > a) {
#ifdef STOREBEST
			m_best = sc0.toIndex();
#endif
			if ((a = a1) >= b) {
#ifndef STOREBEST
				RECORD_HASH(b, HASH_BETA);
#endif
				return b;
			}
#ifndef STOREBEST
			f = HASH_EXACT;
#endif
		}
	}
#ifndef STOREBEST
	RECORD_HASH(a, f);
#endif

	return a;

