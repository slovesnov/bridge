/*
 * ButtonsDialogWithProblem.h
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef DIALOGS_BUTTONSDIALOGWITHPROBLEM_H_
#define DIALOGS_BUTTONSDIALOGWITHPROBLEM_H_

#include "ButtonsDialog.h"

class ButtonsDialogWithProblem: public ButtonsDialog {
protected:
	ButtonsDialogWithProblem(MENU_ID id, bool modal = true,
			BUTTONS_DIALOG_TYPE type = BUTTONS_DIALOG_OK_CANCEL,
			Widget *parent = NULL, bool resizable = false);

	virtual ~ButtonsDialogWithProblem();

	GdkPixbuf *m_suitPixbuf[4];
	GtkWidget *m_playerBox[4][4]; //[player][suit]
	GtkWidget *m_playerNameBox[4]; //[player]
	GtkWidget *m_labelPlayerSuit[4][4]; //[player][suit]
	GtkWidget *m_arrow[8]; //inner image, match with REGION_INNER
	GtkWidget *m_suit[4];
	GtkWidget *m_labelCard[4]; //inner card label
	GdkPixbuf *m_arrowPixbuf[4];
	static const CPoint EDIT_LIST_REGION_POSITION[];

	GtkWidget* createPlayerBox(CARD_INDEX ci, PLAYERBOX_NAME_TYPE type =
			PLAYERBOX_NAME_TYPE_SIMPLE);
	void attachInnerTable(GtkWidget *g);
	void setInnerTable(const Problem &p);
};

#endif /* DIALOGS_BUTTONSDIALOGWITHPROBLEM_H_ */
