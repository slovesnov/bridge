/*
 * ButtonsDialog.h
 *
 *       Created on: 04.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef BUTTONSDIALOG_H_
#define BUTTONSDIALOG_H_

#include "BaseDialog.h"

//dialog with buttons on bottom of Dialog
class ButtonsDialog: public BaseDialog {
	int m_type;

	void init(BUTTONS_DIALOG_TYPE type);

protected:

	VGtkWidgetPtr m_button;

	void show(GtkWidget *w = NULL, bool before = false);
	void show(GtkWidget *w, bool before, VGtkWidgetPtr const &v);

	//return true if dialog should be closed, otherwise it's error and dialog isn't close
	virtual bool click(int index) {
		return true;
	}

	ButtonsDialog(MENU_ID id, bool modal, BUTTONS_DIALOG_TYPE type =
			BUTTONS_DIALOG_OK_CANCEL, Widget *parent = NULL, bool resizable =
			false) :
			BaseDialog(id, modal, parent, resizable) {
		init(type);
	}

	ButtonsDialog(MENU_ID id, BUTTONS_DIALOG_TYPE type =
			BUTTONS_DIALOG_OK_CANCEL, Widget *parent = NULL, bool resizable =
			false) :
			BaseDialog(id, true, parent, resizable) {
		init(type);
	}

	ButtonsDialog(STRING_ID id, BUTTONS_DIALOG_TYPE type =
			BUTTONS_DIALOG_OK_CANCEL, Widget *parent = NULL, bool resizable =
			false) :
			BaseDialog(id, true, parent, resizable) {
		init(type);
	}

	virtual ~ButtonsDialog();

public:

	void clickButton(GtkWidget *w);
	virtual void enableButtons() {
	}

};

#endif /* BUTTONSDIALOG_H_ */
