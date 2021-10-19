/*
 * State.h
 *
 *       Created on: 01.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef STATE_H_
#define STATE_H_

#include "../base/Base.h"
class Problem;

class State {
public:
	CARD_INDEX m_firstmove;
	CARD_INDEX m_cid[52];
	int m_tricks[4];
	int m_estimate[52];
	VInt m_bestLine;

	void operator=(const State& s);

	int getDifference(const State& s) const;
	void clearTricks();
	void clearEstimates();

	int countInnerCards(int currentId = -1) const;
	bool hasSuit(CARD_INDEX player, int suit) const;

	void rotate(const Problem& problem, bool clockwise, bool likeBridge);

	void setInvalidCardsForPreferans();

	void newDeal(const Problem& problem);
	int randomDeal(const Problem& problem);

	int findInner(CARD_INDEX player) const;
	void incrementTricks(CARD_INDEX ci);

	void copyTricks(State const& s);

	void clearInner();

	int countCards(CARD_INDEX ci) const;

	void setBestLine(VInt const& v);
	void adjustBestLine(const int index);

	void getOuterState(CARD_INDEX cid[52])const;
#ifndef FINAL_RELEASE
	void printcids() const;
#endif
};

#endif /* STATE_H_ */
