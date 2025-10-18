/*
 * MessageDialog.h
 *
 *       Created on: 04.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef DIALOGS_MESSAGEDIALOG_H_
#define DIALOGS_MESSAGEDIALOG_H_

#include "ButtonsDialog.h"
#include "../problem/ParseException.h"

class MessageDialog: public ButtonsDialog {
	VString m_content;

public:
	MessageDialog(const VParseException &e, BUTTONS_DIALOG_TYPE type =
			BUTTONS_DIALOG_NONE, Widget *parent = NULL);
	MessageDialog(MESSAGE_ICON_TYPE iconType, const char *s,
			BUTTONS_DIALOG_TYPE type = BUTTONS_DIALOG_NONE, Widget *parent =
					NULL);

	//click already defined in ButtonsDialog
	void clickButton(int i);

	virtual ~MessageDialog();
};

#endif /* DIALOGS_MESSAGEDIALOG_H_ */
