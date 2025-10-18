/*
 * ProblemVectorModified.h
 *
 *       Created on: 20.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef PROBLEM_PROBLEMVECTORMODIFIED_H_
#define PROBLEM_PROBLEMVECTORMODIFIED_H_

#include "ProblemVector.h"

/* cann't inherit Base on should inherit as virtual class
 * because ProblemSelector inherit FrameItemArea & ProblemVectorModified
 * this is intermediate class uses for ProblemSelector & EditListDialog
 * user can create EditListDialog make some changes and click 'cancel'
 */
class ProblemVectorModified {
	VInt m_vbridgeOnly;
	int m_currentBridgeOnly;
public:
	ProblemVector m_vproblem;
protected:
	ProblemVector m_vproblemOriginal;
	VBool m_modified;

	void prepareBridgeOnly();

	const Problem& getProblem() const {
		return m_vproblem[m_current];
	}

	Problem& getProblem() {
		return m_vproblem[m_current];
	}

	void move(bool left);
	void move(int i, bool bridgeOnly = false);

	bool isMovePossible(int i, bool bridgeOnly = false) { //left all, left, right, right all
		bool left = i < 2;
		int c = bridgeOnly ? m_currentBridgeOnly : m_current;
		return c != (left ? 0 : size(bridgeOnly) - 1);
	}

	void countModified() {
		countModified(m_current);
	}

	void countModified(int i) {
		assert(m_vproblem.size() == m_vproblemOriginal.size());
		m_modified[i] = m_vproblemOriginal[i] != m_vproblem[i];
	}
public:
	int m_current;
	ProblemVectorModified();
	virtual ~ProblemVectorModified();

	int size(bool bridgeOnly = false) const {
		return bridgeOnly ? m_vbridgeOnly.size() : m_vproblem.size();
	}

	void insert(VProblem const &vp, bool before);
	void insert(bool _new, bool before, int quantity, GAME_TYPE g);
	void insert(VString const &v, bool before);

	void setModifiedIfSameSize();

	void deleteCurrent();
	void deleteAllBeforeAfter(bool before);

};

#endif /* PROBLEM_PROBLEMVECTORMODIFIED_H_ */
