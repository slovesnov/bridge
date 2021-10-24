/*
 * PreferansScore.h
 *
 *  Created on: 24.10.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef PREFERANSSCORE_H_
#define PREFERANSSCORE_H_

typedef std::vector<double> VDouble;

class PreferansScore {
	int m_players,m_contract,m_tricks;
	bool m_halfwhist;
	int m_pg[4];
	double m_score[4];
	int m_whist[16];
	//player 0-players-1
	void check(int player);
	//from,to 0-players-1
	int& whist(int from,int to);
	void setGame(int players, int contract, int tricks,bool halfwhist);
public:
	static const int player=0;
	static const int whister=1;
	static const int pass=2;

	/* players 3,4
	 * contract 6-10,0-misere
	 * tricks 0-10
	 *
	 * assume player is always first player, active whister(only one) second player
	 */
	void setGame(int players, int contract, int tricks){
		setGame(players, contract, tricks,false);
	}
	void setHalfWhistGame(int players, int contract){
		setGame(players, contract, contract,true);
	}

	PreferansScore() {
	}

	PreferansScore(int players, int contract, int tricks) {
		setGame(players, contract, tricks);
	}

	PreferansScore(int players, int contract) {
		setHalfWhistGame(players, contract);
	}

	VDouble score();

	double playerScore(){
		return m_score[player];
	}

	//active whister
	double whisterScore(){
		return m_score[whister];
	}

	double passScore(){
		return m_score[pass];
	}

	void print();
};

#endif /* PREFERANSSCORE_H_ */
