/*
 * State.cpp
 *
 *       Created on: 01.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "State.h"
#include "Problem.h"

void State::rotate(const Problem& problem, bool clockwise, bool likeBridge) {
	int i;
	for (i = 0; i < 52; ++i) {
		problem.rotate(m_cid[i], clockwise, likeBridge);
	}
	problem.rotate(m_firstmove, clockwise, likeBridge);

	const GAME_TYPE gt = likeBridge ? BRIDGE : problem.m_gameType;
	const int sz = MAX_TABLE_CARDS[gt];
	const CARD_INDEX* ARRAY = gt == BRIDGE ? PLAYER : problem.m_preferansPlayer;

	int tricks[4];
	for (i = 0; i < sz; i++) {
		auto p = getPlayerForArray(ARRAY[i], ARRAY, sz, !clockwise);
		tricks[indexOfPlayer(ARRAY[i])] = m_tricks[indexOfPlayer(p)];
	}
	for (i = 0; i < 4; i++) { //Note i<4 not i<sz
		m_tricks[i] = tricks[i];
	}
}

void State::operator=(const State& s) {
	unsigned i;
#define M(N) for(i=0;i<SIZE(N);i++){N[i]=s.N[i];}
	M(m_cid);
	M(m_tricks);
	M(m_estimate);
#undef M

	m_firstmove = s.m_firstmove;
	m_bestLine=s.m_bestLine;
}

int State::getDifference(const State& s) const {
	int i;
	for (i = 0; i < 52; ++i) {
		if (m_cid[i] != s.m_cid[i] && m_cid[i] != CARD_INDEX_ABSENT
				&& s.m_cid[i] != CARD_INDEX_ABSENT) {
			return i;
		}
	}
	assert(0);
	return -1;
}

void State::clearTricks() { //tricks for all players=0
	int i;
	for (i = 0; i < 4; i++) {
		m_tricks[i] = 0;
	}
}

int State::countInnerCards(int currentId) const {
	int i, n = 0;
	for (i = 0; i < 52; i++) {
		if (isInner(m_cid[i]) && i != currentId) {
			n++;
		}
	}
	return n;
}

bool State::hasSuit(CARD_INDEX player, int suit) const {
	assert(suit >= 0 && suit < NT);
	for (int i = suit * 13; i < (suit + 1) * 13; i++) {
		assert(i >= 0 && i < 52);
		if (m_cid[i] == player) {
			return true;
		}
	}
	return false;
}

void State::newDeal(const Problem& problem) {
	int i;
	//set always valid m_firstmove (for all absents)
	m_firstmove = problem.getBasePlayer();
	for (i = 0; i < 52; ++i) {
		m_cid[i] = m_firstmove;
		m_estimate[i] = ESTIMATE_CLEAR;
	}
	for (i = 0; i < 4; ++i) {
		m_tricks[i] = 0;
	}
	m_bestLine.clear();

	if (problem.isPreferans()) {
		setInvalidCardsForPreferans();
	}

}

void State::setInvalidCardsForPreferans() {
	int i, j;
	for (j = 0; j < 52; j += 13) {
		for (i = 8; i < 13; ++i) {
			m_cid[i + j] = CARD_INDEX_INVALID;
		}
	}
}

#ifndef FINAL_RELEASE
void State::printcids() const {
	std::string s;
	int i;
	for (i = 0; i < 52; i++) {
		s += format("%d", m_cid[i]);
		if((i+1)%13==0){
			s+=" ";
		}
	}
	println("%s", s.c_str());
}
#endif

void State::clearEstimates() {
	int i;
	for (i = 0; i < 52; i++) {
		m_estimate[i] = ESTIMATE_CLEAR;
	}
}

int State::randomDeal(const Problem& problem) {
	int i, j, k, l, m, n[52];

	if (problem.isPreferans()) {
		auto f = getNextBridgePlayer(problem.m_absent);
		for (i = 0; i < 13; i++) {
			for (j = 0; j < 4; j++) {
				n[j * 13 + i] = i > 7 ? -1 : j * 13 + i;
				m_cid[j * 13 + i] = i > 7 ? CARD_INDEX_INVALID : f;
			}
		}

		for (j = 0; j < 4; j++) {
			for (i = 0; i < 8; i++) {
				k = rand() % (32 - j * 8 - i);
				k = (k / 8) * 13 + (k % 8);
				m = 31 - (j * 8 + i);
				m = (m / 8) * 13 + (m % 8);
				l = n[k];
				n[k] = n[m];
				n[m] = l;
			}
		}

		for (l = j = 0; j < 4 && l < 20; j++) {
			for (i = 0; i < 8 && l < 20; i++, l++) {
				if (l % 10 == 0) {
					f = getNextBridgePlayer(f);
				}
				m_cid[n[i + j * 13]] = f; //PLAYER[l / 10];
			}
		}

		//[0..8], [13..13+8], [26..26+8] [39..39+8] - only this n[] !=-1
		m_cid[n[39 + 7]] = m_cid[n[39 + 6]] = CARD_INDEX_ABSENT;

		do {
			k = (rand() % 4) * 13 + rand() % 8;
		} while (m_cid[n[k]] == 0);

		k = n[k];

	}
	else {
		for (i = 0; i < 52; i++) {
			n[i] = i;
			m_cid[i] = CARD_INDEX_WEST;
		}
		for (j = 0; j < 4; j++) {
			for (i = 0; i < 13; i++) {
				k = rand() % (52 - j * 13 - i);
				m_cid[n[k]] = PLAYER[j]; //was j+1
				n[k] = n[51 - j * 13 - i];
			}
		}

		k = rand() % 52;
	} //else

	m_firstmove = m_cid[k];
	return k;
}

void State::copyTricks(const State& s) {
	int i;
	for (i = 0; i < 4; i++) {
		m_tricks[i] = s.m_tricks[i];
	}
}

void State::clearInner() {
	int i;
	for (i = 0; i < 52; i++) {
		if (isInner(m_cid[i])) {
			m_cid[i] = CARD_INDEX_ABSENT;
		}
	}
}

int State::countCards(CARD_INDEX ci) const {
	int i, j = 0;
	for (i = 0; i < 52; i++) {
		if (m_cid[i] == ci) {
			j++;
		}
	}
	return j;
}

void State::setBestLine(VInt const& v){
	m_bestLine=v;
}

void State::adjustBestLine(const int index){
	if(m_bestLine.empty()){
		return;
	}

	//if do move from best line then just remove 1st item, otherwise clear
	if(m_bestLine[0]==index){
		m_bestLine.erase(m_bestLine.begin());
		return;
	}
	int i,j;
	bool sequence=false;
	//check m_bestLine[0] & index is a sequence
	if((j=m_bestLine[0]/13)==index/13){//only if same suit
		int r1=m_bestLine[0]%13;
		int r2=index%13;
		int rmin=std::min(r1,r2)+j*13;
		int rmax=std::max(r1,r2)+j*13;
		for(i=rmin+1;i<rmax && m_cid[i]==m_cid[index];i++);
		sequence=i==rmax;
	}
	if(sequence){
		int pos = indexOf(m_bestLine, index);
		m_bestLine[pos]=m_bestLine[0];
		m_bestLine.erase(m_bestLine.begin());
	}
	else{
		m_bestLine.clear();
	}
}

int State::findInner(CARD_INDEX player) const {
	return INDEX_OF(m_cid, getInner(player));
}

void State::incrementTricks(CARD_INDEX ci) {
	assert(INDEX_OF(PLAYER,ci)!=-1);
	m_tricks[ci - CARD_INDEX_NORTH]++;
}
