/*
 * EditListDialog.h
 *
 *       Created on: 04.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_EDITLISTDIALOG_H_
#define DIALOGS_EDITLISTDIALOG_H_

#include "ButtonsDialogWithProblem.h"
#include "../problem/ProblemVectorModified.h"

enum EDIT_LIST_BUTTON {
	EDIT_LIST_MOVE_LEFT, EDIT_LIST_MOVE_RIGHT, EDIT_LIST_INSERT, EDIT_LIST_DELETE,

	EDIT_LIST_MOVE_0, EDIT_LIST_MOVE_1, EDIT_LIST_MOVE_2, EDIT_LIST_MOVE_3,

	EDIT_LIST_DIALOG_BUTTON_SIZE
};

class EditListDialog: public ButtonsDialogWithProblem, public ProblemVectorModified {

	GtkWidget* m_button[EDIT_LIST_DIALOG_BUTTON_SIZE];
	GtkWidget* m_grid;
	CARD_INDEX m_lastHide;
	void redrawProblem();
	//need to redefine functions use own problem
	const Problem& getProblem() const {
		return ProblemVectorModified::getProblem();
	}

	Problem& getProblem() {
		return ProblemVectorModified::getProblem();
	}

public:
	EditListDialog();
	virtual ~EditListDialog() {
	}

	void click(EDIT_LIST_BUTTON e);
};

#endif /* DIALOGS_EDITLISTDIALOG_H_ */
