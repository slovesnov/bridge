/*
 * ProblemVectorModified.cpp
 *
 *       Created on: 20.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */


#include <cmath>

#include "ProblemVectorModified.h"

ProblemVectorModified::ProblemVectorModified() {
	m_current = 0;
#ifndef NDEBUG
	m_currentBridgeOnly = 0;
#endif
}

ProblemVectorModified::~ProblemVectorModified() {
}

void ProblemVectorModified::move(int i, bool bridgeOnly) {
	int oc = m_current;
	int v;

	//avg(sqrt/opt)=1.159407682
	v = (i == 0 || i == 3 ? std::max(2.0, sqrt(size(bridgeOnly)) / 1.16) : 1);
	if (i < 2) {
		v = -v;
	}

	i = (bridgeOnly ? m_currentBridgeOnly : m_current) + v;

	if (i < 0) {
		i = 0;
	}
	else if (i >= size(bridgeOnly)) {
		i = size(bridgeOnly) - 1;
	}

	if (bridgeOnly) {
		m_currentBridgeOnly = i;
		m_current = m_vbridgeOnly[i];
	}
	else {
		m_current = i;
	}

	assert(m_current != oc);

	if (m_vproblem.size() == m_vproblemOriginal.size()) {
		assert(oc >= 0 && oc < int(m_modified.size()));
		countModified(oc);
	}
}

void ProblemVectorModified::move(bool left) {
	assert((left && m_current > 0) || (!left && m_current < size() - 1));
	int i = m_current;

	if (left) {
		i--;
	}
	else {
		i++;
	}

	m_vproblem.swap(i, m_current);

	if (m_vproblem.size() == m_vproblemOriginal.size()) {
		countModified();
		countModified(i);
	}

	m_current = i;
}

void ProblemVectorModified::insert(bool _new, bool before, int quantity,
		GAME_TYPE g) {
	int i;
	VProblem vp;
	for (i = 0; i < quantity; i++) {
		vp.push_back(Problem(g, CARD_INDEX_SOUTH, _new ? 1 : 2));
	}

	insert(vp, before);
}

void ProblemVectorModified::insert(const VString& v, bool before) {
	if (v.size() == 0) {
		return;
	}

	ProblemVector pv;
	if (!pv.set(v, false)) {
		//no new problems recognized
		return;
	}

	insert(pv.m_problems, before);
}

void ProblemVectorModified::insert(const VProblem& vp, bool before) {
	m_vproblem.insert(m_current + !before, vp);
	if (before) {
		m_current += vp.size();
	}
	setModifiedIfSameSize();
}

void ProblemVectorModified::deleteCurrent() {
	assert(size() > 1);
	bool last = m_current == size() - 1;

	m_vproblem.m_problems.erase(m_vproblem.begin() + m_current);

	if (last) {
		m_current--;
	}

	setModifiedIfSameSize();
}

void ProblemVectorModified::deleteAllBeforeAfter(bool before) {
	assert(size() > 1);

	if (before) {
		printinfo
		m_vproblem.m_problems.erase(m_vproblem.begin(),
				m_vproblem.begin() + m_current);
		m_current = 0;
	}
	else {
		printinfo
		m_vproblem.m_problems.erase(m_vproblem.begin() + m_current + 1,
				m_vproblem.end());
	}

	setModifiedIfSameSize();
}

void ProblemVectorModified::prepareBridgeOnly() {
	int i;
	VProblemCI it;

	//also set m_currentBridgeOnly for PbnEditorDialog
	for (i = 0, it = m_vproblem.begin(); it != m_vproblem.end(); it++, i++) {
		if (it->isBridge()) {
			if (i == m_current) {
				m_currentBridgeOnly = m_vbridgeOnly.size();
			}
			m_vbridgeOnly.push_back(i);
		}
	}

	//make current problem bridge
	if (getProblem().isPreferans()) {
		assert(m_vbridgeOnly.size() > 0);
		m_currentBridgeOnly = 0;
		m_current = m_vbridgeOnly[0];
	}

}

void ProblemVectorModified::setModifiedIfSameSize() {
	int i;
	if (m_vproblem.size() == m_vproblemOriginal.size()) {
		m_modified.resize(m_vproblem.size());
		for (i = 0; i < m_vproblem.size(); i++) {
			countModified(i);
		}
	}

}
