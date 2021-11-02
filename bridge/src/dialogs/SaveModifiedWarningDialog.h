/*
 * SaveModifiedWarningDialog.h
 *
 *       Created on: 06.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef SAVEMODIFIEDWARNINGDIALOG_H_
#define SAVEMODIFIEDWARNINGDIALOG_H_

#include "ButtonsDialog.h"

class SaveModifiedWarningDialog: public ButtonsDialog {
	GtkWidget* m_checkPreview;
public:
	SaveModifiedWarningDialog();
	virtual ~SaveModifiedWarningDialog();

	bool click(int index)override;
};

#endif /* SAVEMODIFIEDWARNINGDIALOG_H_ */
