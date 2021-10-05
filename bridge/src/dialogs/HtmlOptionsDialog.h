/*
 * HtmlOptionsDialog.h
 *
 *       Created on: 03.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef HTMLOPTIONSDIALOG_H_
#define HTMLOPTIONSDIALOG_H_

#include "ButtonsDialog.h"

class HtmlOptionsDialog: public ButtonsDialog {
	GtkWidget* m_check[4];
public:
	HtmlOptionsDialog();
	virtual ~HtmlOptionsDialog();

	virtual bool click(int index);

};

#endif /* HTMLOPTIONSDIALOG_H_ */
