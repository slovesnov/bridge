/*
 * CustomSkinDialog.h
 *
 *       Created on: 16.03.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_CUSTOMSKINDIALOG_H_
#define DIALOGS_CUSTOMSKINDIALOG_H_

#include "ButtonsDialog.h"

const STRING_ID CUSTOMSKINDIALOG_BUTTONS[] = {
		STRING_SELECT_IMAGE,
		STRING_SELECT_COLOR };

class CustomSkinDialog: public ButtonsDialog {
	GtkWidget*m_button[SIZE(CUSTOMSKINDIALOG_BUTTONS)];
public:
	CustomSkinDialog();
	virtual ~CustomSkinDialog();

	bool click(int index)override;
	void clickButton(int i);
};

#endif /* DIALOGS_CUSTOMSKINDIALOG_H_ */
