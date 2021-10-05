/*
 * DeleteProblemDialog.h
 *
 *       Created on: 09.03.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_DELETEPROBLEMDIALOG_H_
#define DIALOGS_DELETEPROBLEMDIALOG_H_

#include "ButtonsDialog.h"
#include "../problem/ProblemVectorModified.h"

class EditListDialog;

const STRING_ID DELETE_PROBLEM_DIALOG_RID[] = {
		STRING_CURRENT_DEAL,
		STRING_ALL_DEALS };

class DeleteProblemDialog: public ButtonsDialog {
	GtkWidget* m_combo;
	GtkWidget *m_radio[SIZE(DELETE_PROBLEM_DIALOG_RID)];

	ProblemVectorModified* m_pvm;
public:
	DeleteProblemDialog(EditListDialog* eld);
	virtual ~DeleteProblemDialog();

	virtual bool click(int index);
};

#endif /* DIALOGS_DELETEPROBLEMDIALOG_H_ */
