/*
 * PreferansScore.cpp
 *
 *  Created on: 24.10.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include <cassert>
#include "aslov.h"
#include "PreferansScore.h"

void PreferansScore::check(int player) {
	assert(player>=0 && player<m_players);
}

int& PreferansScore::whist(int from, int to) {
	return m_whist[from*4+to];
}

void PreferansScore::setGame(int players, int contract, int tricks,
		bool halfwhist) {
	assert(players == 3 || players == 4);
	assert(contract == 0 || (contract >= 6 && contract <= 10));
	assert(tricks >= 0 && tricks <= 10);

	int i,p;
	double v;

	m_players = players;
	m_contract=contract;
	m_tricks=tricks;
	m_halfwhist=halfwhist;

	for (auto &a : m_pg) {
		a = 0;
	}
	for (auto &a : m_whist) {
		a = 0;
	}

	if (contract == 0) {
		assert(!halfwhist);
		m_pg[player] = tricks == 0 ? 10 : -10 * tricks;
	}
	else{
		const int undertricks = contract - tricks; //недобранные взятки
		const int whistertricks = 10 - tricks;
		const int MIN_WHISTER_TRICKS[]={4,2,1,1,0};
		const bool contractDone = undertricks <= 0;
		const int c = 2 * (contract - 5);
		const int minwhistertricks=MIN_WHISTER_TRICKS[contract-6];

		m_pg[player] = c * (contractDone ? 1 : -undertricks);

		//http://pref-bridge.ru/PrefRulesCommon3.htm
		if(contractDone){
			if (whistertricks < minwhistertricks) {
				m_pg[whister] = c * (whistertricks - minwhistertricks);
			}
		}
		else{
			//[en] consolation - penalty whists [ru] консоляция
			for (i = 0; i < m_players; i++) {
				if (i != player) {
					whist(i, player) = c * undertricks;
				}
			}

		}
		whist(1, player) += c * whistertricks;
		if(halfwhist){
			whist(1, player)/=2;
		}
	}

	//count m_score
	for (p = 0; p < m_players; p++) {
		v = m_players * m_pg[p];
		for (i = 0; i < m_players; i++) {
			v -= m_pg[i];
		}
		v *= 10. / m_players;
		for (i = 0; i < m_players; i++) {
			if (i != p) {
				v += whist(p, i) - whist(i, p);
			}
		}
		m_score[p]=v;
	}
}

VDouble PreferansScore::score() {
	VDouble r(m_score,m_score+m_players);
	return r;
}

void PreferansScore::print() {
	int i, j, k;
	printzn("players = ", m_players, ", contract = ", m_contract, ", tricks = ",
			m_tricks, ", halfwhist = ", m_halfwhist);
	for (i = 0; i < m_players; i++) {
		if (m_pg[i]) {
			printzn("pg", i + 1, " = ", m_pg[i]);
		}
	}
	for (i = 0; i < m_players; i++) {
		for (j = 0; j < m_players; j++) {
			if ((k = whist(i, j)) != 0) {
				printzn("whist", i + 1, j + 1, " = ", k);
			}
		}
	}
}
