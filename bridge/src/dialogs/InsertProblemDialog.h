/*
 * InsertProblemDialog.h
 *
 *       Created on: 16.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_INSERTPROBLEMDIALOG_H_
#define DIALOGS_INSERTPROBLEMDIALOG_H_

#include "ButtonsDialog.h"
#include "../problem/ProblemVectorModified.h"

class EditListDialog;

enum {
	INSERT_PROBLEM_DIALOG_COMBO_BEFORE_AFTER,
	INSERT_PROBLEM_DIALOG_COMBO_GAMETYPE,
	INSERT_PROBLEM_DIALOG_COMBO_QUANTITY,

	INSERT_PROBLEM_DIALOG_COMBO_SIZE
};

const STRING_ID INSERT_PROBLEM_DIALOG_RID[] = {
		STRING_NEW_DEAL,
		STRING_RANDOM_DEAL,
		STRING_FROM_FILE };

class InsertProblemDialog: public ButtonsDialog {
	GtkWidget* m_combo[INSERT_PROBLEM_DIALOG_COMBO_SIZE];
	GtkWidget *m_radio[SIZE(INSERT_PROBLEM_DIALOG_RID)];
	GtkWidget *m_grid;
	ProblemVectorModified* m_pvm;
public:
	InsertProblemDialog(EditListDialog* eld);
	virtual ~InsertProblemDialog();

	bool click(int index)override;
	void toggle(GtkWidget *w);
};

#endif /* DIALOGS_INSERTPROBLEMDIALOG_H_ */
