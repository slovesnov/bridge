/*
 * Preferans.cpp
 *
 *       Created on: 16.09.2019
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2019-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Preferans.h"
#include "Permutations.h"

#ifdef CONSOLE
#include <cstdio>
#endif

int32_t*Preferans::m_r;
int Preferans::m_oc=0;//object counter
const int MAX_SUIT_CODE=0x3aaaa;//240 298
const int MAX_SUIT_CODE_ARRAY_SIZE=MAX_SUIT_CODE+1;//240 299

int Preferans::m_w[];

int8_t (*Preferans::m_moves)[3*5];

#define CUT3LASTLAYERS
#define REMOVE_CARD_NS(i) m_code[sc##i.s]=m_r[ (m_code[sc##i.s]<<3) | r##i ];
#define REMOVE_CARD(i) rc##i=m_code[sc##i.s];REMOVE_CARD_NS(i);
#define RESTORE_CARD(i) m_code[sc##i.s]=rc##i;

#define	RECORD_HASH_C(e, flag) for (i = 0; i < 3; i++) {h.code[i] = m_code[i];}h.v = e;h.f = flag;

#ifdef PREFERANS_ONE_HASH_ITEM
	#define	RECORD_HASH(e, flag) RECORD_HASH_C(e, flag)
#else
	#define	RECORD_HASH(e, flag) HashItem& h=he.i[he.next];RECORD_HASH_C(e, flag);if(he.next==HASH_ITEMS-1){he.next=0;}else{he.next++;}
#endif

#define ADJUST_RANK(i,j) if (sc##i.s == sc##j.s && r##i > r##j) {r##i--;}

//	prevents gcc warnings for file pi2.h with m_depth==2 may be used uninitialized
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#define COM(i, j) m_ct[(sc##i.sc<<10)|sc##j.sc]
#pragma GCC diagnostic pop

//set "t" variable which is taker from sc0,sc1,sc2 variables
#define SETT \
if (COM(0, 1)) {\
	t = COM(0, 2) ? 0 : 2;\
}\
else {\
	t = COM(1, 2) ? 1 : 2;\
}

#define MOVES_INIT(suit,w,p,q) q=m_moves[m_code[suit]]+w*5;p=q+*q;q++;

Preferans::Preferans(bool smallHash) {
//	println("create preferans object %llx",uint64_t(this))
	if(m_oc++==0){
		staticInit();
	}
	int hashBits = smallHash ? 18 + 1 : HASH_BITS;
	m_hashSize = 1 << hashBits;
	m_andKey = m_hashSize - 1;
	m_hashTable = new Hash[m_hashSize];
}

Preferans::~Preferans() {
//	println("destroy preferans object %llx",uint64_t(this))
	if(--m_oc==0){
		staticDeinit();
	}
	delete[] m_hashTable;
}

void Preferans::staticInit(){
	BridgePreferansBase::staticInit();

	int i, j, k, l, d, c;
	constexpr int s = MAX_SUIT_CODE_ARRAY_SIZE * 8;
	m_r = new int32_t[s]; //240 299*8*4/1024/1024=7.33mb
	for (d = 1; d < 9; d++) {
		Permutations p(d, 3, PERMUTATIONS_WITH_REPLACEMENTS);
		do {
			c = 3 << (2 * d);
			for (i = 0; i < d; i++) {
				c |= p.getIndex(i) << (2 * i);
			}
			for (i = 0; i < d; i++) {
				j = (c<<3) | i;
				assert(j<s);

				k=(i<<1);
				l = ((c >> (k + 2)) << k) | (c & ((1 << k) - 1));

				m_r[j] = l;
			}
		} while (p.next());
	}


	i=0;
	for(int& a:m_w){
		a=i%3;
		i++;
	}

	Permutations p;
	int w,o[3];
	const int sz=3*5;
	int8_t t[sz];

	m_moves=new int8_t[MAX_SUIT_CODE_ARRAY_SIZE][sz];
	/* 240299*15/1024/1024=3.437504768371582
	 * 1<<18 = 262144
	 */

	c = 3;
	for (w = 0; w < 3; w++) {
		t[w * 5] = 0;
	}
	memcpy(m_moves + c, t, sz * sizeof(int8_t));

	for(k=1;k<9;k++){
		p.init(k,3,PERMUTATIONS_WITH_REPLACEMENTS);
		for (auto& z : p) {
			for(i=0;i<3;i++){
				o[i]=0;
			}

			i = 0;
			w=-1;
			c=3<<(k*2);
			for (int v : z) {
				c|=v<<(i*2);

				if(v!=w){
					w=v;
					t[w*5+o[w]+1]=i;
					o[w]++;
				}
				i++;
			}
			for(w=0;w<3;w++){
				t[w*5]=o[w];
			}
			memcpy(m_moves+c,t,sz*sizeof(int8_t));
		}
	}

}

void Preferans::staticDeinit(){
	delete[]m_r;
	delete[]m_moves;
}

#ifndef NDEBUG
int Preferans::getW(int suit, int pos) {
	pos <<= 1;
	int c = m_code[suit];
	return (c >> pos) & 3;
}
#endif

#ifndef CONSOLE
void Preferans::solve(const Problem& p, bool trumpChanged) {
	CARD_INDEX cid[52];
	p.getClearCid(cid);
	CARD_INDEX first = p.getFirst();
	if (p.m_misere) {
		solvebMisere(cid, p.m_trump, first, p.m_player, p.m_preferansPlayer,
				trumpChanged);
	}
	else if(p.m_trump==NT) {
		solvebNT(cid, p.m_trump, first, p.m_player, p.m_preferansPlayer,
				trumpChanged);
	}
	else {
		solveb(cid, p.m_trump, first, p.m_player, p.m_preferansPlayer,
				trumpChanged);
	}
}
#endif

#ifndef CONSOLE
void Preferans::estimateAll(const Problem& pr, ESTIMATE estimateType,
		SET_ESTIMATION_FUNCTION estimationFunction) {

	const int toIndex = m_best;
	Problem z = pr;
	State& st = z.getState();
	if (pr.isTableFull()) {
		st.clearInner();
	}
	State so = st; //save cleared state
	CARD_INDEX* cid = st.m_cid;
	CARD_INDEX c=cid[toIndex];
	const bool pl=c==pr.m_player;
	const int bestE = pl ? m_playerTricks : m_cards - m_playerTricks;
	CARD_INDEX next = pr.getNextMove();
	CARD_INDEX first = pr.getFirst();
	int i, j, e;
	CARD_INDEX l;
	VSC v;

	i=st.findInner(first);

	if (i == -1) {//no cards on table
		//any card
		for (i = 0; i < 4; i++) {
			addSuitableGroups(i, cid, next, v, toIndex);
		}
	}
	else {
		j=i/13;
		//little difference from bridge, because should play trump
		//if has no suit, and has trump
		addSuitableGroups(j, cid, next, v, toIndex);
		if (v.empty()) {
			if (m_trumpOriginal != NT && j != m_trump) {
				addSuitableGroups(m_trumpOriginal, cid,next, v, toIndex);
			}
			if (v.empty()) {
				for (i = 0; i < 4; i++) {
					if (i != j) {
						addSuitableGroups(i, cid, next, v, toIndex);
					}
				}
			}
		}
	}

	//set best
	for(SC const& x: v){
		if(x.o){
			for(i=0;i<x.length;i++){
				estimationFunction(x[i].toIndex(), bestE);
			}
			break;
		}
	}

	if (estimateType == ESTIMATE_ALL_LOCAL || estimateType == ESTIMATE_ALL_TOTAL){
		//skip set question for all card groups, except bestmove, because count very fast

		//now estimate
		for(SC const& x: v){
			if(x.o){
				continue;
			}
			st = so;
			j = x[0].toIndex();
			CARD_INDEX c = cid[j];
			cid[j] = getInner(c);		//make move
			e = 0;
			if (st.countInnerCards() == pr.maxTableCards()) {
				l = z.getNextMove();
				st.clearInner();		//clear table
				/* in case of taker!=c find player which isn't l & not c
				 * if this player is m_player then c & l whist players
				 */
				if (l == c || (l != z.m_player && c != z.m_player)) {
					e = 1;		//extra trick
				}
			}
			else {
				l = first;
			}

			/* Note empty position couldn't appear, because in case
			 * when only one card left, we have only one group in vector (it's best move)
			 * and already setup estimation
			 * so don't care about empty position
			 */
			solveEstimateOnly(cid, pr.m_trump, l, pr.m_player, pr.m_misere,
					pr.m_preferansPlayer, false);
			//m_cards-m_playerTricks = whist tricks
			e += pl ? m_playerTricks : m_cards - m_playerTricks;

			for (i = 0; i < x.length; i++) {
				estimationFunction(x[i].toIndex(), e);
			}
		}
	}

}
#endif

void Preferans::solveFull(const CARD_INDEX c[52], int trump,
		CARD_INDEX first, CARD_INDEX player, bool misere,
		const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
	if (misere) {
		solvebMisere(c, trump, first, player, preferansPlayer, trumpChanged);
	}
	else if(trump==NT){
		solvebNT(c, trump, first, player, preferansPlayer, trumpChanged);
	}
	else{
		solveb(c, trump, first, player, preferansPlayer, trumpChanged);
	}
}

void Preferans::solveEstimateOnly(const CARD_INDEX c[52], int trump,
		CARD_INDEX first, CARD_INDEX player, bool misere,
		const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
	if (misere) {
		solveMisere(c, trump, first, player, preferansPlayer, trumpChanged);
	}
	else if(trump==NT){
		solveNT(c, trump, first, player, preferansPlayer, trumpChanged);
	}
	else{
		solve(c, trump, first, player, preferansPlayer, trumpChanged);
	}
}

//BEGIN AUTOMATICALLY GENERATED TEXT
int Preferans::e(const int* w, int a, int b) {
#include "pi.h"
}

int Preferans::eb(const int* w, int a, int b) {
#define STOREBEST
#include "pi.h"
#undef STOREBEST
}

int Preferans::eNT(const int* w, int a, int b) {
#define NO_TRUMP
#include "pi.h"
#undef NO_TRUMP
}

int Preferans::ebNT(const int* w, int a, int b) {
#define STOREBEST
#define NO_TRUMP
#include "pi.h"
#undef NO_TRUMP
#undef STOREBEST
}

int Preferans::eMisere(const int* w, int a, int b) {
#define MISERE
#include "pi.h"
#undef MISERE
}

int Preferans::ebMisere(const int* w, int a, int b) {
#define STOREBEST
#define MISERE
#include "pi.h"
#undef MISERE
#undef STOREBEST
}

void Preferans::solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#include "psolve.h"
}

void Preferans::solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define STOREBEST
#include "psolve.h"
#undef STOREBEST
}

void Preferans::solveNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define NO_TRUMP
#include "psolve.h"
#undef NO_TRUMP
}

void Preferans::solvebNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define STOREBEST
#define NO_TRUMP
#include "psolve.h"
#undef NO_TRUMP
#undef STOREBEST
}

void Preferans::solveMisere(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define MISERE
#include "psolve.h"
#undef MISERE
}

void Preferans::solvebMisere(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	CARD_INDEX player, const CARD_INDEX preferansPlayer[3], bool trumpChanged) {
#define STOREBEST
#define MISERE
#include "psolve.h"
#undef MISERE
#undef STOREBEST
}

void Preferans::suitableCards2(int suit, const int* w, SC& c1, SC& c2) {
	suitableCards(suit, w[1], c1);
	suitableCards(suit, w[2], c2);
}

void Preferans::suitableCards2NT(int suit, const int* w, SC& c1, SC& c2) {
	suitableCardsNT(suit, w[1], c1);
	suitableCardsNT(suit, w[2], c2);
}

void Preferans::suitableCards2Misere(int suit, const int* w, SC& c1, SC& c2) {
	suitableCardsMisere(suit, w[1], c1);
	suitableCardsMisere(suit, w[2], c2);
}
//END AUTOMATICALLY GENERATED TEXT

void Preferans::bestLine(const CARD_INDEX c[52], CARD_INDEX first,
		CARD_INDEX player, bool misere, const CARD_INDEX preferansPlayer[3]){
	int i, j, k = 0, t, l, m = 0, fi;
	CARD_INDEX o[52];
	const CARD_INDEX *p;
	USC sc0, sc1, sc2;

	/* store variable for estimateAll function,
	 * not need now because estimatAall calls before bestLine
	 */
	m_bestLine.clear();

	USC* ps[]={&sc0,&sc1,&sc2};

	for(i=0;i<3 &&preferansPlayer[i]!=first;i++);
	fi=i;

	for (i = 0; i < 4; i++) {
		p = c + i * 13;
		for (j = 0; j < 8; j++) {
			l = *p++;
			if(l>0 && l<5){
				m++;
			}
			else if(l>=5){
				for (t = 0; t < 3; t++) {
					if (l == preferansPlayer[(fi+t)%3] + 4) {
						ps[t]->set(j, i);
					}
				}

				k++;
			}
		}
	}

	for(i=0;i<52;i++){
		o[i]=c[i];
	}

	if (k == 3) {		//full table
		/* in preferans not need to use fast search option,
		 * because problems are solved too fast.
		 * so code is differ with bridge, no et, es, mc needed
		 */
		k = 0;
		t=getTaker(ps, SIZE(ps));
		fi += t;
		fi %= 3;

		//clear inner cards
		for (i = 0; i < 52; i++) {
			if (c[i] >= 5) {
				o[i] = CARD_INDEX_ABSENT;
			}
		}
	}


	for (j = 0; j < (m + k) / 3; j++) {
		for (i = (j==0 ? k: 0); i < 3; i++) {
			if (misere) {
				solvebMisere(o, NT, preferansPlayer[fi], player, preferansPlayer, false);
			}
			else if (m_trumpOriginal == NT) {
				solvebNT(o, NT, preferansPlayer[fi], player, preferansPlayer, false);
			}
			else {
				solveb(o, m_trumpOriginal, preferansPlayer[fi], player, preferansPlayer, false);
			}

			m_bestLine.push_back(m_best);
			o[m_best] = CARD_INDEX(o[m_best] + 4);

			//need for SETT macro
			ps[i]->fromIndex(m_best);

		}
		if(j==0){
			for(l=0;l<k;l++){
				o[ps[l]->toIndex()]= CARD_INDEX_ABSENT;
			}
		}
		for (i = 0; i < (j==0 ? 3-k: 3); i++) {
			o[m_bestLine[m_bestLine.size() - 1 - i]] = CARD_INDEX_ABSENT;
		}
		t=getTaker(ps, SIZE(ps));

		fi+=t;
		fi%=3;
	}

}

void Preferans::suitableCards(int suit, int w, SC& c) {
#define ORDER PREFERANS_ORDER_OTHER_MOVES
#define PREFERANS_TRUMP_GAME
#include "moves.h"
#undef PREFERANS_TRUMP_GAME
#undef ORDER
}

void Preferans::suitableCardsNT(int suit, int w, SC& c) {
#define ORDER PREFERANS_ORDER_OTHER_MOVES_NT
#include "moves.h"
#undef ORDER
}

void Preferans::suitableCardsMisere(int suit, int w, SC& c) {
#define ORDER PREFERANS_ORDER_OTHER_MOVES_MISERE
#include "moves.h"
#undef ORDER
}

#ifndef FINAL_RELEASE
void Preferans::printCode(int suit){
	auto c=m_code[suit];
	println("%s %x\n",binaryCodeString(c).c_str(),c);
}
#endif
