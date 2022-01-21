/*
 * BridgePreferansBase.cpp
 *
 *       Created on: 19.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include <numeric>
#include <set>

#include "Bridge.h"
#include "Preferans.h"
#include "Permutations.h"
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

VVInt BridgePreferansBase::suitLengthVector(int n,bool bridge,EndgameType option) {
	//l[] - number of cards in suit
	int l[4];
	int ntotal=(bridge?4:3)*n;
	const int up = std::min(bridge ? 13 : 8, ntotal);
	VVInt v;
	VInt vi;
	for (l[0] = 0; l[0] <= up; l[0]++) {
		for (l[1] = 0; l[1] <= up; l[1]++) {
			for (l[2] = 0; l[2] <= up; l[2]++) {
				l[3] = ntotal - l[0] - l[1] - l[2];
				if (l[3] >= 0 && l[3]<=up) {
					if (option == EndgameType::ALL
							|| (option == EndgameType::NT  && l[0] <= l[1] && l[1] <= l[2]
									&& l[2] <= l[3])
							|| (option == EndgameType::TRUMP && l[1] <= l[2]
									&& l[2] <= l[3])) {
						vi.assign(l, l+4);
						v.push_back(vi);
					}
				}
			}
		}
	}
	return v;
}

VVInt BridgePreferansBase::suitLengthVector(bool bridge,EndgameType option) {
	return suitLengthVector(endgameGetN(bridge),bridge,option);
}

int BridgePreferansBase::endgameGetN(bool bridge,bool total/*=false*/){
	int i=bridge?Bridge::endgameN:Preferans::endgameN;//number of cards of each player
	if(total){
		i*= (bridge?4:3);
	}
	return i;
}

int BridgePreferansBase::bitCode(bool bridge, VInt const &p0, VInt const &p1, VInt const &p2) {
	int i,j;
	int n=endgameGetN(bridge);
	int ntotal=endgameGetN(bridge,true);
	VInt freepos(ntotal);
	std::iota(freepos.begin(), freepos.end(), 0);

	VInt vb[2+bridge];
	for (auto&a:vb) {
		a.resize(n);
	}

	const VInt *pv[] = { &p0, &p1, &p2 };
	i=0;
	for(auto q:pv){
		/* use reverse order to get more understandable vectors
		 * and need to remove from freepos in reverse order
		 * p0={0,1,2} vb[0]={2,1,0}
		 * p1={0,1,2} vb[1]={5,4,3}
		 * p2={0,1,2} vb[2]={8,7,6}
		 */
		j = n - 1;
		for (auto it = q->rbegin(); it != q->rend(); it++) {
			vb[i][j--] = freepos[*it];
			freepos.erase(freepos.begin() + *it);
		}
		i++;
		if(!bridge && i==2){
			break;
		}
	}

	// bit code v[0] - 01 bits, v[1] - 10, (if bridge==true v[2] - 11)
	int c=0;
	i=1;
	for (auto&a:vb) {
		for(j=0;j<n;j++){
			c |= i<<2*a[j];
		}
		i++;
	}
	return c;
}

void BridgePreferansBase::endgameRotate(bool bridge,const int mw[],int n,int bits,int a[]){
	int i,j,r;

	assert(bits % 2 == 0);
	for (j = 0; j < ((bridge?4:3)-1); j++) {
		r = 0;
		for (i = 0; i < bits / 2; i++) {
			r |= mw[((n>>(2*i)) & 3)+j+1] << (2 * i);
		}
		a[j] = r;
	}
}

int BridgePreferansBase::endgameCm (int n,bool bridge) {//if bridge=1 C^n_4n*C^n_3n*C^n_2n, else C^n_3n*C^n_2n
	int i=1;
	Permutations p;
	const int k=bridge?4:3;
	for(int j=0;j<k-1;j++){
		p.init(n, (k -j)*n, COMBINATION);
		i*=p.number();
	}
	return i;
}

int BridgePreferansBase::endgameCm (bool bridge) {//if bridge=1 C^n_4n*C^n_3n*C^n_2n, else C^n_3n*C^n_2n
	return endgameCm(endgameGetN(bridge),bridge);
}

int BridgePreferansBase::getMinBijectionMultiplier(bool bridge) {
	int i,j,k;
	VVInt v[2];
	bool b;
	const int n=endgameGetN(bridge);
	std::set<int> set;
	for (i = 0; i < 2; i++) {
		v[i] = suitLengthVector(n, bridge,
				i == 0 ? EndgameType::NT : EndgameType::TRUMP);
	}

	for (j = 7; j < 15; j++) {
		b=true;
		for(i=0;i<2;i++){
			set.clear();
			for (auto &a : v[i]) {
				k = a[0] + j * (a[1] + j * a[2]);
				if (set.find(k) == set.end()) {
					set.insert(k);
				} else {
					b=false;
					goto l303;
				}
			}
		}
		l303:
		if(b){
			return j;
		}
	}
	assert(0);
	return -1;
}

void BridgePreferansBase::endgameInit(bool bridge,
		int32_t* endgameLength[],
		int32_t* endgameIndex[],
		int8_t* endgameEstimate[],
	#ifndef NDEBUG
		int endgameEstimateLength[],
	#endif
		const int endgameMultiplier,
		const int endgameTypes,
		const int mw[]
		){

	//clock_t begin=clock();
	int i,j,k,c;
	int a[3];
	Permutations pe[3];

	for (i=0;i<endgameTypes;i++) {
		auto&p=endgameLength[i];
		// i ? EndgameType::TRUMP : EndgameType::NT is ok for bridge and preferans
		VVInt v = suitLengthVector(bridge, i ? EndgameType::TRUMP : EndgameType::NT);
		VInt const& max=*std::max_element(v.begin(), v.end(), [](auto &a, auto &b) {
			return a[2] < b[2];
		});

		const int size=(max[2]+1)*endgameMultiplier*endgameMultiplier;
		p=new int32_t[size];
#ifndef NDEBUG
		for(j=0;j<size;j++){
			p[j]=-1;
		}
#endif
		k=0;
		for (auto a : v) {
			j = a[0] + endgameMultiplier * (a[1] + endgameMultiplier * a[2]);
			assert(j < size);
			p[j] = k++;
		}
	}

	const int n = endgameGetN(bridge);
	const int ntotal = endgameGetN(bridge ,true);

	for (i = 0; i < 3; i++) {
		pe[i].init(n, ntotal - n * i, COMBINATION);
	}

	c=ntotal*2-2;
	const int max=1<<c;
	for (i=0;i<(bridge?4:3);i++) {
		auto&p=endgameIndex[i];
		p=new int32_t[max];
#ifndef NDEBUG
		for(j=0;j<max;j++){
			p[j]=-1;
		}
#endif
	}

	//printl(max,c,pe[2].number());

	j=0;
	for (auto &p0 : pe[0]) {
		for (auto &p1 : pe[1]) {
			for (auto &p2 : pe[2]) {
				k=bitCode(bridge,p0,p1,p2) & (max-1);
				assert(k<max);
//				if(k==99){
//					printi
//				}
				endgameIndex[0][k]=j;
				endgameRotate(bridge,mw,k,c,a);

				for(i=0;i<(bridge?4:3)-1;i++){
					assert(a[i]<max);
					endgameIndex[i+1][a[i]]=j;
				}
				j++;
			}
		}
	}

	for (i=0;i<endgameTypes;i++) {
		auto& p=endgameEstimate[i];
		//TODO path
		std::string path=format("C:/slovesno/%c%d%s.bin",bridge?'b':'p',n,i==0?"nt":"trump");
		j=getFileSize(path);
#ifndef NDEBUG
		endgameEstimateLength[i]=j;
#endif
		p=new int8_t[j];
		FILE*f=fopen(path.c_str(),"rb");
		fread(p,j,1,f);
		fclose(f);
	}

//	printl(bridge,timeElapse(begin));

}
