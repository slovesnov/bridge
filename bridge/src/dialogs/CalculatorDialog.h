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
#include "../helper/PreferansScore.h"

class CalculatorDialog: public BaseDialog {
	VGtkWidgetPtr m_combo;
	GtkWidget* m_score[4];
	GtkWidget* m_label[8];
	GtkWidget* m_area;
	void updateScore();
	void setPreferansLabels();
	int getPreferansContract();
	int getPreferansPlayers();
	bool isMisere();
	void showHideRow(int row,bool show);
	std::string preferansScoreToString(int i);
	PreferansScore m_pscore;
public:
	CalculatorDialog();
	void comboChanged(GtkWidget* w);
	void drawArea(cairo_t *cr);
};

#endif /* DIALOGS_CALCULATORDIALOG_H_ */
