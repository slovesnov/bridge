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

enum WHIST_OPTION{
	WHIST_OPTION_WHIST,/*first player whist, others pass*/
	WHIST_OPTION_HALFWHIST,/*first player half whist, others pass*/
	WHIST_OPTION_ALL_PASS,/*all players pass*/
};

class PreferansScore {
	int m_players,m_contract,m_tricks;
	WHIST_OPTION m_whistOption;
	int m_pg[4];
	double m_score[4];
	int m_whist[16];
	//player 0-players-1
	void check(int player);
	//from,to 0-players-1
	int& whist(int from,int to);
	void setGame(int players, int contract, int tricks,WHIST_OPTION whistOption);
public:
	static const int player=0;
	static const int whister=1;

	/* players 3,4
	 * contract 6-10,0-misere
	 * tricks 0-10
	 *
	 * assume player is always first player, active whister(only one) second player
	 */
	void setGame(int players, int contract, int tricks){
		setGame(players, contract, tricks,WHIST_OPTION_WHIST);
	}
	void setNonPlayingGame(int players, int contract,bool halfwhist){
		setGame(players, contract, contract,halfwhist?WHIST_OPTION_HALFWHIST:WHIST_OPTION_ALL_PASS);
	}

	PreferansScore() {
	}

	PreferansScore(int players, int contract, int tricks) {
		setGame(players, contract, tricks);
	}

	PreferansScore(int players, int contract,bool halfwhist) {
		setNonPlayingGame(players, contract,halfwhist);
	}

	VDouble score();

	double playerScore(){
		return m_score[player];
	}

	//active whister
	double whisterScore(){
		return m_score[whister];
	}

	void print();
};

#endif /* PREFERANSSCORE_H_ */
