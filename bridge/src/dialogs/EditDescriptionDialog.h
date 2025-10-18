/*
 * EditDescriptionDialog.h
 *
 *       Created on: 02.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef EDITDESCRIPTIONDIALOG_H_
#define EDITDESCRIPTIONDIALOG_H_

#include "ButtonsDialog.h"

class EditDescriptionDialog: public ButtonsDialog {
	GtkWidget *m_comment;
public:
	EditDescriptionDialog();
	virtual ~EditDescriptionDialog();
	bool click(int index) override;
};

#endif /* EDITDESCRIPTIONDIALOG_H_ */
