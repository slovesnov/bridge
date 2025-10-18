/*
 * PbnEditorDialog.h
 *
 *       Created on: 22.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef PBNEDITORDIALOG_H_
#define PBNEDITORDIALOG_H_

#include "ButtonsDialog.h"
#include "../problem/ProblemVectorModified.h"
#include "../problem/Problem.h"

enum PBN_EDITOR_COMBO {
	//first items should math with CM[] defined in PbnEditorDialog.cpp
	PBN_EDITOR_COMBO_DEALER,
	PBN_EDITOR_COMBO_RESULT,
	PBN_EDITOR_COMBO_ROOM,
	PBN_EDITOR_COMBO_VULNERABLE,

	PBN_EDITOR_COMBO_DOUBLE_REDOUBLE,
	PBN_EDITOR_COMBO_SCORE,

	PBN_EDITOR_COMBO_SIZE
};

enum PBN_EDITOR_LABEL {
	PBN_EDITOR_LABEL_DEAL, PBN_EDITOR_LABEL_SCORE, PBN_EDITOR_LABEL_AUCTION,

	PBN_EDITOR_LABEL_SIZE
};

class PbnEditorDialog: public ButtonsDialog, public ProblemVectorModified {
	GtkWidget *m_grid;
	GtkWidget *m_combo[PBN_EDITOR_COMBO_SIZE];
	GtkWidget *m_label[PBN_EDITOR_LABEL_SIZE];
	GtkWidget *m_auction;
	VGtkWidgetPtr m_entry;

	GtkWidget *m_labelProblem; //shows problem number
	GtkWidget *m_button[4]; //next, previous problem

	//need to redefine functions use own problem
	const Problem& getProblem() const {
		return ProblemVectorModified::getProblem();
	}

	Problem& getProblem() {
		return ProblemVectorModified::getProblem();
	}

	void redrawProblem();
	void updateScore();
	void updateAuctionLabel();
	void updateDealLabel();
	bool storeProblem();

	bool click(int index) override;

public:
	PbnEditorDialog();
	virtual ~PbnEditorDialog();
	void comboChanged(int i);
	void clickButton(int i);
};

#endif /* PBNEDITORDIALOG_H_ */
