/*
 * SolveForAllDeclarersDialog.h
 *
 *       Created on: 30.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef SOLVEFORALLDECLARERSDIALOG_H_
#define SOLVEFORALLDECLARERSDIALOG_H_

#include "BaseDialog.h"

class SolveForAllDeclarersDialog: public BaseDialog {
	GtkWidget *m_label[SIZE(PLAYER)][NT + 2][3][2];
	GtkWidget *m_loading[2];
	int m_searches;
public:
	SolveForAllDeclarersDialog(const int *r = NULL);
	void setBridgeLabel(int trump);
};

#endif /* SOLVEFORALLDECLARERSDIALOG_H_ */
