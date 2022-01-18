/*
 * Bridge.cpp
 *
 *       Created on: 10.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "Bridge.h"

#ifdef CONSOLE
#include <ctime>
#else
#include "../base/Base.h"
#include "../base/Config.h"
#endif

#ifdef BRIDGE_ENDGAME
#include "Permutations.h"
#endif

int Bridge::m_w[];

int8_t **Bridge::m_moves;

#ifdef BRIDGE_ENDGAME
int32_t* Bridge::endgameLength[2];
int32_t* Bridge::endgameIndex[4];
int8_t* Bridge::endgameEstimate[2];
#ifndef NDEBUG
int Bridge::endgameEstimateLength[2];//NT+ trump
#endif
const int Bridge::endgameCN=endgameCm(true);
#endif

int Bridge::m_oc=0;

#ifndef CONSOLE

struct SolveParameters {
	CARD_INDEX c[52];
	int trump;
	CARD_INDEX first;
	int lowTricks;
	int highTricks;
	int e;
	SC sc;/*indexes of turns*/
	bool ns;
};

//next variables should be one instance for one process, not member function
static int solveParametersIndex;
static std::vector<SolveParameters> solveParameters;
static GMutex mutex;
static SET_ESTIMATION_FUNCTION gEstimationFunction;
static std::vector<GThread*>gEstimateThreadV;
static Bridge* gBase;
static Bridge*gb;

static gpointer estimate_all_thread(gpointer d) {
	((Bridge*) d)->estimateAllThread();
	return NULL;
}
#endif

static_assert(sizeof(int)>=4);

#define CUT4LASTLAYERS
#define REMOVE_CARD_NS(i) removeCard(sc##i.s,r##i);
#define REMOVE_CARD(i) rc##i=removeCard(sc##i.s,r##i);
#define RESTORE_CARD(i) m_code[sc##i.s]=rc##i;
#define	RECORD_HASH(e, flag) HashItem& h=he.i[he.next];\
	h.code3=m_code[3]>>16;for (i = 0; i < 3; i++) {h.code[i] = m_code[i];}h.v = e;h.f = flag;\
	if(he.next==HASH_ITEMS-1){he.next=0;}else{he.next++;}
#define SET_BEST(i) m_best=sc##i.toIndex();
#define ADJUST_RANK(i,j) if (sc##i.s == sc##j.s && r##i > r##j) {r##i--;}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#define COM(i, j) m_ct[(sc##i.sc<<10)|sc##j.sc]
#pragma GCC diagnostic pop

#define HASH_KEY (((m_code[0] +m_code[1]) << 6) ^ m_code[2] ^ (m_code[3] << 3) ^ w[0]) & AND_KEY

//set "t" variable which is taker from sc0,sc1,sc2,sc3 variables
#define SETT \
if (COM(0,1)) {\
	if(COM(0,2)){\
		t = COM(0,3) ? 0 : 3;\
	}\
	else{\
		t = COM(2,3) ? 2 : 3;\
	}\
}\
else {\
	if(COM(1,2)){\
		t = COM(1,3) ? 1 : 3;\
	}\
	else{\
		t = COM(2,3) ? 2 : 3;\
	}\
}

#if BRIDGE_MAX_PRECOUNT_SUIT_CARDS==12

#define MOVES_INIT(suit,w,p,q) 	k = m_code[suit];\
l = k & 15;\
k >>= 4;\
if ( l == BRIDGE_MAX_PRECOUNT_SUIT_CARDS+1) {\
	m = k & ((1 << (2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS)) - 1);\
	z = m_moves[BRIDGE_MAX_PRECOUNT_SUIT_CARDS] + (m * 4 + w) * MAX_MOVES;\
	n = *z;\
	z++;\
	p = q = z;\
	p += n;\
	k >>= 2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS;\
	z = m_moves[1] + (k * 4 + w) * MAX_MOVES;\
	l = *z;\
	m = (k & 3) == w && ((m >> (2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS - 2)) & 3) == w;\
	if(l==1 && m==0){\
		//TODO like for BRIDGE_MAX_PRECOUNT_SUIT_CARDS==11
		memcpy(m_mi, q, n);\
		p = q = m_mi;\
		p += n;\
		*p = z [BRIDGE_MAX_PRECOUNT_SUIT_CARDS+1];\
	}\
	else{\
		p--;\
	}\
}\
else {\
	q = m_moves[l] + (k * 4 + w) * MAX_MOVES;\
	p=q+*q;\
	q++;\
}


#else


#define MOVES_INIT_BRIDGE(suit,w,p,q,mm) 	k = m_code[suit];\
l = k & 15;\
k >>= 4;\
if ( l > BRIDGE_MAX_PRECOUNT_SUIT_CARDS) {\
	j = l - BRIDGE_MAX_PRECOUNT_SUIT_CARDS;\
	m = k & ((1 << (2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS)) - 1);\
	z = m_moves[BRIDGE_MAX_PRECOUNT_SUIT_CARDS] + (m * 4 + w) * MAX_MOVES;\
	n = *z;\
	z++;\
	p = q = z;\
	p += n;\
	k >>= 2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS;\
	m = (k & 3) == w && ((m >> (2 * BRIDGE_MAX_PRECOUNT_SUIT_CARDS - 2)) & 3) == w;\
	z = m_moves[j] + (k * 4 + w) * MAX_MOVES;\
	l = *z;\
	if( m < l ){\
		if (m) {\
			z++;\
		}\
		memcpy(mm, q, n);\
		p = q = mm;\
		p += n;\
		for (z++; m < l; m++, z++) {\
			*p++ = *z + BRIDGE_MAX_PRECOUNT_SUIT_CARDS;\
		}\
	}\
	p--;\
}\
else {\
	q = m_moves[l] + (k * 4 + w) * MAX_MOVES;\
	p=q+*q;\
	q++;\
}

#endif

void Bridge::staticInit(){
	int i;

	BridgePreferansBase::staticInit();

	i=0;
	for(int& a:m_w){
		a=i%4;
		i++;
	}

	const int players=4;
	int8_t*pm;
	int j,k,c,w,v;
	int8_t* t;
	int8_t*p[players];

//	clock_t begin=clock();
	m_moves=new int8_t*[BRIDGE_MAX_PRECOUNT_SUIT_CARDS+1];

	pm=m_moves[0]=new int8_t[players*MAX_MOVES];

	for(i=0;i<players;i++,pm+=MAX_MOVES){
		*pm=0;
	}

	for(k=1;k<=BRIDGE_MAX_PRECOUNT_SUIT_CARDS;k++){
		t=new int8_t[players*MAX_MOVES];
		int n[players];

		const int size=1<<(2*k);
		pm=m_moves[k]=new int8_t[size*players*MAX_MOVES];
		//faster without permutations
		for (c = 0; c < size; c++,pm+=players * MAX_MOVES) {
			p[0]=pm+1;
			for (i = 1; i < players; i++) {
				p[i]=p[i-1]+MAX_MOVES;
			}

			for (i = 0; i < players; i++) {
				n[i]=0;
			}

			w = -1;
			for (j = c, i = 0; i < k; i++, j >>= 2) {
				v = j & 3;
				if (v != w) {
					w = v;
					*p[w]++=i;
					n[w]++;
				}
			}
			for (w = 0; w < players; w++) {
				pm[w*MAX_MOVES]=n[w];
			}
		}
		delete[] t;
	}


#ifdef BRIDGE_ENDGAME
	//clock_t begin=clock();

	const bool bridge=true;
	int a[3];
	Permutations pe[3];

	i=0;
	for (auto&p:endgameLength) {
		VVInt v = suitLengthVector(bridge, i ? EndgameType::TRUMP : EndgameType::NT);
		VInt const& max=*std::max_element(v.begin(), v.end(), [](auto &a, auto &b) {
			return a[2] < b[2];
		});

		const int size=(max[2]+1)*169;
		p=new int32_t[size];
#ifndef NDEBUG
		for(j=0;j<size;j++){
			p[j]=-1;
		}
#endif
		k=0;
		for (auto a : v) {
			j = a[0] + 13 * (a[1] + 13 * a[2]);
			assert(j < size);
			p[j] = k++;
		}
		i++;
	}

	const int n = endgameGetN(bridge);
	const int ntotal = endgameGetN(bridge ,true);

	for (i = 0; i < 3; i++) {
		pe[i].init(n, ntotal - n * i, COMBINATION);
	}

	c=ntotal*2-2;
	const int max=1<<c;
	for (auto&p:endgameIndex) {
		p=new int32_t[max];
#ifndef NDEBUG
		for(j=0;j<max;j++){
			p[j]=-1;
		}
#endif
	}

	j=0;
	for (auto &p0 : pe[0]) {
		for (auto &p1 : pe[1]) {
			for (auto &p2 : pe[2]) {
				k=bitCode(bridge,p0,p1,p2) & (max-1);
				assert(k<max);
				endgameIndex[0][k]=j;
				endgameRotate(m_w,k,c,a);
				for(i=0;i<3;i++){
					assert(a[i]<max);
					endgameIndex[i+1][a[i]]=j;
				}
				j++;
			}
		}
	}

	i=0;
	for(auto& p:endgameEstimate){
		//TODO path
		std::string path("C:/slovesno/b"+std::string(i==0?"nt":"trump")+".bin");
		j=getFileSize(path);
#ifndef NDEBUG
		endgameEstimateLength[i]=j;
#endif
		p=new int8_t[j];
		FILE*f=fopen(path.c_str(),"rb");
		fread(p,j,1,f);
		fclose(f);
		i++;
	}

//	printl(timeElapse(begin));
#endif


#ifndef CONSOLE
	g_mutex_init(&mutex);
#endif

}

void Bridge::staticDeinit(){
	int i;
	for(i=0;i<=BRIDGE_MAX_PRECOUNT_SUIT_CARDS;i++){
		delete[]m_moves[i];
	}
	delete[]m_moves;

#ifndef CONSOLE
	g_mutex_clear(&mutex);
#endif

#ifdef BRIDGE_ENDGAME
	for (auto&p:endgameLength) {
		delete[]p;
	}
	for (auto&p:endgameIndex) {
		delete[]p;
	}
	for (auto&p:endgameEstimate) {
		delete[]p;
	}
#endif

}

Bridge::Bridge() {
	//println("create bridge object %llx",uint64_t(this))
	if(m_oc++==0){
		staticInit();
	}
	m_hashTable = new Hash[HASH_SIZE];
	assert(m_hashTable);
}

Bridge::~Bridge() {
//	println("%llx",uint64_t(this));
	if(--m_oc==0){
		staticDeinit();
	}
	delete[]m_hashTable;
}

int Bridge::removeCard(int suit, int pos) {
	pos += 2;
	pos <<= 1;
	int c = m_code[suit];
	//-1 decrease suit length
	m_code[suit] = (((c >> (pos + 2)) << pos) | (c & ((1 << pos) - 1)))-1;
	return c;
}

void Bridge::solveFull(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		bool trumpChanged, int lowTricks, int highTricks) {
	if(trump==NT){
		solvebNT(c, trump, first,trumpChanged, lowTricks, highTricks);
	}
	else{
		solveb(c, trump, first,trumpChanged, lowTricks, highTricks);
	}
}

void Bridge::solveEstimateOnly(const CARD_INDEX c[52], int trump, CARD_INDEX first,
		bool trumpChanged, int lowTricks, int highTricks) {
	if(trump==NT){
		solveNT(c, trump, first,trumpChanged, lowTricks, highTricks);
	}
	else{
		solve(c, trump, first,trumpChanged, lowTricks, highTricks);
	}
}

//BEGIN AUTOMATICALLY GENERATED TEXT
int Bridge::e(const int* w, int a) {
#include "bi.h"
}

int Bridge::eb(const int* w, int a) {
#define STOREBEST
#include "bi.h"
#undef STOREBEST
}

int Bridge::eNT(const int* w, int a) {
#define NO_TRUMP
#include "bi.h"
#undef NO_TRUMP
}

int Bridge::ebNT(const int* w, int a) {
#define STOREBEST
#define NO_TRUMP
#include "bi.h"
#undef NO_TRUMP
#undef STOREBEST
}

void Bridge::solve(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	bool trumpChanged, int lowTricks, int highTricks) {
	assert(trump!=NT);
#include "bsolve.h"
}

void Bridge::solveb(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	bool trumpChanged, int lowTricks, int highTricks) {
	assert(trump!=NT);
#define STOREBEST
#include "bsolve.h"
#undef STOREBEST
}

void Bridge::solveNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	bool trumpChanged, int lowTricks, int highTricks) {
	assert(trump==NT);
#define NO_TRUMP
#include "bsolve.h"
#undef NO_TRUMP
}

void Bridge::solvebNT(const CARD_INDEX c[52], int trump, CARD_INDEX first,
	bool trumpChanged, int lowTricks, int highTricks) {
	assert(trump==NT);
#define STOREBEST
#define NO_TRUMP
#include "bsolve.h"
#undef NO_TRUMP
#undef STOREBEST
}
//END AUTOMATICALLY GENERATED TEXT

#ifndef CONSOLE

void Bridge::solve(const Problem& p, bool trumpChanged) {
	CARD_INDEX cid[52];
	p.getClearCid(cid);
	CARD_INDEX first = p.getFirst();
	gBase=0;
	if(p.m_trump==NT){
		solvebNT(cid, p.m_trump, first, trumpChanged);
	}
	else{
		solveb(cid, p.m_trump, first, trumpChanged);
	}
}

void Bridge::estimateAll(const Problem& p, ESTIMATE estimateType,
		SET_ESTIMATION_FUNCTION estimationFunction,bool beforeBest,bool trumpChanged) {
	/* make wrapper function, to avoid restore variables leaks
	 * store m_e,m_cards for bestLine function
	 */
#define M(a) const auto a=m_##a;
	M(e)
	M(cards)
#undef M

	estimateAllInner( p, estimateType,
			 estimationFunction,beforeBest,trumpChanged);

#define M(a) m_##a=a;
	M(e)
	M(cards)
#undef M
}

/* fast - do search with tight alpha beta interval,
 * which is based on best move estimation which is already found
 * otherwise search with full alpha beta window
 * leave parameter to compare speed of fast and slow options
 * function works correct when table is full
 */
void Bridge::estimateAllInner(const Problem& p, ESTIMATE estimateType,
		SET_ESTIMATION_FUNCTION estimationFunction,bool beforeBest,bool trumpChanged) {

	const bool fast=true;
	const int toIndex = beforeBest?-1:m_best;

	int i, j, e,low,high;
	bool im;
	CARD_INDEX l;
	VSC v;

	Problem z = p;
	State& st = z.getState();
	if (p.isTableFull()) {
		st.clearInner();
	}
	State so = st; //save cleared state
	CARD_INDEX* cid = st.m_cid;

	CARD_INDEX next = p.getNextMove();
	CARD_INDEX first = p.getFirst();
	const bool ns= northOrSouth(beforeBest? next: cid[toIndex]);
	const int bestE = ns ? m_ns :m_ew;


	i=st.findInner(first);

	if (i == -1) {//no cards on table
		//any card
		for (i = 0; i < 4; i++) {
			addSuitableGroups(i, cid, next, v, toIndex);
		}
	}
	else {
		j=i/13;
		addSuitableGroups(j, cid, next, v, toIndex);
		if (v.empty()) {
			for (i = 0; i < 4; i++) {
				if (i != j) {
					addSuitableGroups(i, cid, next, v, toIndex);
				}
			}
		}
	}

	//Note if bestBefore==true x.o=false always

	//set best
	for(SC const& x: v){
		if(x.o){
			for(i=0;i<x.length;i++){
				estimationFunction(x[i].toIndex(), bestE);
			}
			break;
		}
	}

	if (estimateType != ESTIMATE_ALL_LOCAL && estimateType != ESTIMATE_ALL_TOTAL){
		return;
	}

	//set question for all card groups, except bestmove
	for (SC const& x : v) {
		if (x.o) {
			continue;
		}
		for (i = 0; i < x.length; i++) {
			estimationFunction(x[i].toIndex(), UNKNOWN_ESTIMATE);
		}
	}

	//now estimate
	SolveParameters sp;
	sp.trump = p.m_trump;
	sp.ns=ns;
	solveParameters.clear();//CLEAR PREVIOUS

	for (SC const& x : v) {
		if (x.o) {
			continue;
		}
		st = so;
		j = x[0].toIndex();
		CARD_INDEX c = cid[j];
		cid[j] = getInner(c);		//make move
		e = 0;
		if (st.countInnerCards() == p.maxTableCards()) {
			l = z.getNextMove();
			st.clearInner();		//clear table
			if (l == c || l == getBridgePartner(c)) {
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
		/* fast algorithm with estimation
		 * from our side estimation=a should be <=bestE (a<=bestE),
		 * but out 1st move is fixed so estimate another side in solve function, estimation=b
		 * a+b=m_cards, a=m_cards-b
		 * m_cards-b<=bestE so the condition is b>=m_cards-bestE in case of switch move to opponents
		 */
		im = 0;
		std::copy(cid, cid + 52, sp.c);
		sp.e=e;
		sp.first=l;
		if (fast && !beforeBest) {
			if (e == 1) {
				high = bestE - 1;
				if (high <= 1) {
					if (high == 0) {
						im = 1;
						//TODO remove, cann't model such situation
						println("high == 0 GOT IT")
					}
					else {
						/* Here high=1, do search in [0,1] window in solve
						 * function this interval became [0,2] because of
						 * if(highTricks!=DEFAULT_TRICKS){
						 * 	 high=highTricks+1
						 * }
						 * so cycle while (low + 1 < high)
						 * has one iteration
						 *
						 * this case happens on GeorgeCoffin problem1.
						 */
						low = 0;
					}
				}
				else {
					low = 0;
				}
			}
			else {
				/* in case when low=m_cards, cann't use high as default value -1, which means high=m_cards+1
				 * because cycle while (low + 1 < high) in solve function returns immediately,
				 * so in this case just use estimate=m_cards. This case happens in #1.bts file without very 1st turn
				 */
				low = m_cards - bestE;
				if (low == m_cards) {
					im = 1;
				}
				else {
					high = DEFAULT_TRICKS;
				}
			}
		}
		else {
			low = high = DEFAULT_TRICKS;
		}

		if (im && !beforeBest) {
			e += ns ? m_ns : m_ew;
			for (i = 0; i < x.length; i++) {
				estimationFunction(x[i].toIndex(), e);
			}
		}
		else {
			sp.lowTricks = low;
			sp.highTricks = high;
			sp.sc.copy(x);
			solveParameters.push_back(sp);
		}
	}
#ifndef FINAL_RELEASE
	clock_t begin=clock();
#endif
	/* number of threads should be <= gconfig->m_maxThreads
	 * also number of threads should be <= solveParameters.size()
	 * so <=std::min(gconfig->m_maxThreads,solveParameters.size())
	 */
	j=std::min(gconfig->m_maxThreads,int(solveParameters.size()));

	/* if n=0 always solve without threads
	 * if n=1 run j-1 threads and solve in this thread (fastest option)
	 *
	 * in case if solve() time < 1 second or gconfig->m_maxThreads=1 or
	 * need to estimate only one move solveParameters.size()=1 then don't
	 * need use threads
	 */
	const int n=1;

	//j==0 only one best move solveParameters.size()=0, nothing to estimate, so use j<=1

	if (n == 0 || (!beforeBest && m_time < 1) || j <= 1) {
		for (auto&v : solveParameters) {
			solveEstimateOnly(v.c, v.trump, v.first, trumpChanged, v.lowTricks, v.highTricks);
			e = v.e + (v.ns ? m_ns : m_ew);
			for (i = 0; i < v.sc.length; i++) {
				estimationFunction(v.sc[i].toIndex(), e);
			}
		}
#ifndef FINAL_RELEASE
			printl("end(nothreads)",double(clock()-begin)/CLOCKS_PER_SEC)
#endif
		return;
	}

	solveParametersIndex=0;
	gEstimationFunction=estimationFunction;
	gBase=this;
	gEstimateThreadV.clear();

	j--;
	gb=new Bridge[j];

	//copy hashtable
	if( (beforeBest && !trumpChanged) || !beforeBest){
		auto begin=clock();
		for (i = 0; i < j; i++) {
			memcpy ( gb[i].m_hashTable, m_hashTable, HASH_SIZE*sizeof(Hash) );
		}
		println("hashcopy %.3lf",double(clock()-begin)/CLOCKS_PER_SEC);
	}

	for (i = 0; i < j; i++) {
		Bridge*p= gb+i;
		p->m_estimateTrumpChanged=trumpChanged;
#ifndef FINAL_RELEASE
		p->m_threadIndex=i+1;
#endif
		gEstimateThreadV.push_back( g_thread_new("", estimate_all_thread, gpointer(p)));
	}

	m_estimateTrumpChanged=trumpChanged;
#ifndef FINAL_RELEASE
	m_threadIndex=0;
#endif
	estimateAllThread();

	finishEstimateAll();
//	println("end estimate %.3lf",double(clock()-begin)/CLOCKS_PER_SEC);

}

void Bridge::finishEstimateAll(){
//	println("%lld",gEstimateThreadV.size())
	for(GThread* a:gEstimateThreadV){
//		println("%llx",int64_t(a))
		g_thread_join(a);
	}
	delete[]gb;
}

void Bridge::estimateAllThread(){
	int i,j,e;
#ifndef FINAL_RELEASE
	auto begint=clock();
	auto t=format("t%d",m_threadIndex);
#endif
	while (1) {
		g_mutex_lock(&mutex);
		i = solveParametersIndex++;
		g_mutex_unlock(&mutex);

		if (i >= int(solveParameters.size())) {
#ifndef FINAL_RELEASE
			printl("end",double(clock()-begint)/CLOCKS_PER_SEC,t)
#endif
			return;
		}

		auto& v=solveParameters[i];

#ifndef FINAL_RELEASE
		auto begin=clock();
#endif

		solveEstimateOnly(v.c, v.trump, v.first, m_estimateTrumpChanged, v.lowTricks,
				v.highTricks);

#ifndef FINAL_RELEASE
		printl(m_estimateTrumpChanged ? "trump changed" : "trump not changed",
		    double(clock()-begin)/CLOCKS_PER_SEC, v.lowTricks, v.highTricks,t )
#endif

		e = v.e + (v.ns ? m_ns : m_ew);
		m_estimateTrumpChanged=false;
		for (i = 0; i < v.sc.length; i++) {
			j=v.sc[i].toIndex();
			gEstimationFunction(j, e);
		}

	}
}
#endif

void Bridge::bestLine(const CARD_INDEX c[52], CARD_INDEX first){
	/* fast - is fast zero window search, otherwise search with full alpha beta window
	 * leave parameter to compare speed of fast and slow options
	 * function works correct when table is full
	 */
	const bool fast=true;
	int i, j, k = 0, t, l, m = 0, fi;
	CARD_INDEX o[52];
	const CARD_INDEX*p;
	USC sc0, sc1, sc2, sc3;
	int low = DEFAULT_TRICKS, high = DEFAULT_TRICKS, mc = m_cards, es = m_e;

	/* store variable for estimateAll function, which use
	 * in Bridge class m_ns, m_ew, m_best
	 * not need now because estimateAll calls before bestLine
	 */

	m_bestLine.clear();

	USC* ps[]={&sc0,&sc1,&sc2,&sc3};
	for(USC*p:ps){
		p->set(-1,-1);
	}

	fi=first-1;

	int y[4];
	for(i=0;i<4;i++){
		y[i]=(fi+i) % 4 + 1;
	}

	for (i = 0; i < 4; i++) {
		p = c + i * 13;
		for (j = 0; j < 13; j++) {
			l = *p++;
			if(l>0 && l<5){
				m++;
			}
			else if(l>=5){
				for (t = 0; t < 4; t++) {
					if (l == y[t] + 4) {
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

	if (k == 4) {		//full table
		/* in bridge use fast search option,
		 * in this case search always with zero window
		 * This option is not necessary for preferans game
		 * because problems is solved too fast.
		 */
		k = 0;
		t=getTaker(ps, SIZE(ps));
		if (t%2) {
			es--;
		}
		mc--;

		fi+=t;
		fi%=4;
		
		//clear inner cards
		for (i = 0; i < 52; i++) {
			if (c[i] >= 5) {
				o[i] = CARD_INDEX_ABSENT;
			}
		}
	}

	//nes=1 means m_e,es is estimation from north/south
	const bool nes = (k + fi) % 2;

	for (j = 0; j < (m + k) / 4; j++) {
		for (i = (j==0 ? k: 0); i < 4; i++) {
			if(fast){
				high =(i+ (j==0?k:fi+nes))%2 ? es : mc - es;
/* here analog of upper condition
				if(j==0){
					high =  (i-k) % 2  ? es : mc - es;
				}
				else{

					if((fi+i)%2==0){//ns
						high =nes ? es : mc - es;
					}
					else{
						high =!nes ? es : mc - es;

					}
				}
*/
				low = high - 2;
			}

			if (m_trumpOriginal == NT) {
				solvebNT(o, NT, CARD_INDEX(fi + 1), 0, low, high);
			} else {
				solveb(o, m_trumpOriginal, CARD_INDEX(fi + 1), 0, low, high);
			}

			m_bestLine.push_back(m_best);
			o[m_best] = CARD_INDEX(o[m_best] + 4);
			ps[i]->fromIndex(m_best);

		}
		if(j==0){
			for(l=0;l<k;l++){
				o[ps[l]->toIndex()]= CARD_INDEX_ABSENT;
			}
		}
		for (i = 0; i < 4-(j==0 ? k: 0); i++) {
			o[m_bestLine[m_bestLine.size() - 1 - i]] = CARD_INDEX_ABSENT;
		}

		t=getTaker(ps, SIZE(ps));

		if( (fi+t+nes)%2 ){// (nes && (fi+t)%2==0) || (!nes &&  (fi+t)%2==1)
			es--;
		}
		mc--;

		fi+=t;
		fi%=4;
	}

}

void Bridge::printCode(int suit) {
	int c = m_code[suit];
	int l = c & 15;
	std::string s;
	s += SUITS_CHAR[suit];
	s += " " + (l == 0 ? "" : binaryCodeString(c >> 4, l * 2) + '.')
			+ binaryCodeString(l, 4, '\0')
			+ format(" l=%d 0x%x", l, m_code[suit]);
	printl(s)
}

//TODO using include
int Bridge::ep(const int* w, int a){
	if (a >= m_depth){
		return a;
	}
	if (a + 2 <= -m_depth){
		return a + 2;
	}

	int i,j;

	Hash& he = m_hashTable[HASH_KEY];
	for(j=0;j<HASH_ITEMS;j++){
		HashItem& h = he.i[j];
		if (h.f != HASH_INVALID && (m_code[3]>>16) == h.code3 ) {
			for (i = 0; i < 3 && h.code[i] == m_code[i]; i++)
				;
			if (i == 3) {
				if (h.f == HASH_ALPHA && h.v <= a) {
					return a;
				}
				if (h.f == HASH_BETA && h.v >= a+2) {
					return a+2;
				}
			}
		}

	}


	return 100;
}

#ifndef NEW_MOVES_ORDER

#define PUSH_I(i,card)if(f##i){\
	f##i=0;\
	if(c##i.length==c##i.o){\
		c##i.push(card, suit);\
		c##i.o++;\
	}\
	else{\
		sc=c##i.a[c##i.o];\
		c##i.a[c##i.o++].set(card,suit);\
		c##i.push(sc);\
	}\
}\
else{\
	c##i.push(card, suit);\
}

#define PUSH_F(a,f) c##a.o=0;\
if(m_trump==NT){\
	for(i=0;i<4;i++){\
		if(i!=suit){\
			f(i, w[a], c##a);\
		}\
	}\
}\
else{\
	f(m_trump, w[a], c##a);\
	for(i=0;i<4;i++){\
		if(i!=suit && i!=m_trump){\
			f(i, w[a], c##a);\
		}\
	}\
}

void Bridge::suitableCardsOneSuit(int suit, int w, SC& a){
	int i;
	int c = m_code[suit];
	int l = c & 15;
	const int8_t*p;
	const int players = 4;
	c >>= 4;
	if ( l > BRIDGE_MAX_PRECOUNT_SUIT_CARDS) {
		int j=l-6;
		int c1=c&0xfff;
		p = m_moves[6] + ( c1 * players + w) * MAX_MOVES;
		for(l=*p,i=0,p++;i<l;i++,p++){
			a.push(*p, suit);
		}
		c >>= 12;

		bool b = (c&3)==w && ((c1>>10)&3)==w;
		p = m_moves[j] + (c * players + w) * MAX_MOVES;
		for(l=*p,i=0,p++;i<l;i++,p++){
			if(!b || i!=0){
				a.push(*p+6, suit);
			}
		}
		return;
	}

	p = m_moves[l] + (c * players + w) * MAX_MOVES;
	for(l=*p,i=0,p++;i<l;i++,p++){
		a.push(*p, suit);
	}

}

void Bridge::suitableCardsInverseOneSuit(int suit, int w, SC& c1){
	int c = m_code[suit];
	int l = c & 15;
	int8_t*p,*q;
	const int players = 4;
	c >>= 4;

	bool f1=1;
	USC sc;

	if ( l > BRIDGE_MAX_PRECOUNT_SUIT_CARDS) {

		//highest bits at first
		int ch=c>>12;
		//6-7 cards
		q=p = m_moves[l-6] + ( ch * players + w) * MAX_MOVES;
		p+=*q;
		while(p!=q){
			PUSH_I(1,*p+6)
			p--;
		}

		//6 cards
		c&=0xfff;
		bool b = ( (c>>10 ) &3 )==w && (ch&3)==w;
		q=p = m_moves[6] + (c * players + w) * MAX_MOVES;
		p+=*q;
		while(p!=q){
			if(b ){
				b=0;
			}
			else{
				PUSH_I(1,*p)
			}
			p--;
		}
		return;
	}

	q=p = m_moves[l] + (c * players + w) * MAX_MOVES;
	p+=*q;
	while(p!=q){
		PUSH_I(1,*p)
		p--;
	}

}
#endif

void Bridge::suitableCards3(int suit, const int* w, SC& c1, SC& c2, SC& c3){
#ifdef NEW_MOVES_ORDER

	suitableCards(suit, w[1], c1);
	suitableCards(suit, w[2], c2);
	suitableCards(suit, w[3], c3);

#else

	int i;

	suitableCardsOneSuit(suit,w[1],c1);
	if(c1.length==0){PUSH_F(1,suitableCardsInverseOneSuit)}

	suitableCardsOneSuit(suit,w[2],c2);
	if(c2.length==0){PUSH_F(2,suitableCardsInverseOneSuit)}

	suitableCardsOneSuit(suit,w[3],c3);
	if(c3.length==0){PUSH_F(3,suitableCardsInverseOneSuit)}


/*
	const int m=8;
	if(m_trump!=0){
		if(c1.length>m){
			c1.print();
			printl(m_trump)
			exit(0);
		}

		if(c2.length>m){
			c2.print();
			printl(m_trump)
			exit(0);
		}

		if(c3.length>m){
			c3.print();
			printl(m_trump)
			exit(0);
		}
	}
*/

#endif

}

void Bridge::suitableCards3NT(int suit, const int* w, SC& c1, SC& c2, SC& c3){
#ifdef NEW_MOVES_ORDER
	suitableCardsNT(suit, w[1], c1);
	suitableCardsNT(suit, w[2], c2);
	suitableCardsNT(suit, w[3], c3);

#else

	int i;

	suitableCardsOneSuit(suit,w[1],c1);
	if(c1.length==0){PUSH_F(1,suitableCardsInverseOneSuit)}

	suitableCardsOneSuit(suit,w[2],c2);
	if(c2.length==0){PUSH_F(2,suitableCardsInverseOneSuit)}

	suitableCardsOneSuit(suit,w[3],c3);
	if(c3.length==0){PUSH_F(3,suitableCardsInverseOneSuit)}

/*
	const int m=8;
	if(c1.length>m){
		c1.print();
		printl(m_trump)
		exit(0);
	}

	if(c2.length>m){
		c2.print();
		printl(m_trump)
		exit(0);
	}

	if(c3.length>m){
		c3.print();
		printl(m_trump)
		exit(0);
	}
*/
#endif

}

void Bridge::suitableCards(int suit, int w, SC& c){
#define ORDER BRIDGE_ORDER_OTHER_MOVES
#include "moves.h"
#undef ORDER
}

void Bridge::suitableCardsNT(int suit, int w, SC& c){
#define ORDER BRIDGE_ORDER_OTHER_MOVES_NT
#include "moves.h"
#undef ORDER
}
