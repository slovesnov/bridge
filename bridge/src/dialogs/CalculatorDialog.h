/*
 * CalculatorDialog.h
 *
 *       Created on: 27.12.2016
 *           Author: alexey slovesnov
 * copyright(c/c++): 2016-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_CALCULATORDIALOG_H_
#define DIALOGS_CALCULATORDIALOG_H_

#include "BaseDialog.h"

class CalculatorDialog: public BaseDialog {
	VGtkWidgetPtr m_combo;
	GtkWidget* m_score[2];
	void updateScore();
	std::string m_player[4];
public:
	CalculatorDialog();
	void comboChanged(GtkWidget* w);

};

#endif /* DIALOGS_CALCULATORDIALOG_H_ */
