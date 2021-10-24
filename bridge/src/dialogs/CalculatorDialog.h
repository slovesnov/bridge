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
	GtkWidget* m_label[5];
	std::string m_player[4];
	void updateScore();
	void setPreferansLabel();
	int getPrerefansContract();
	bool isMisere();
	bool isHalfWhist();
	void showHideRow(int row,bool show);
public:
	CalculatorDialog();
	void comboChanged(GtkWidget* w);

};

#endif /* DIALOGS_CALCULATORDIALOG_H_ */
