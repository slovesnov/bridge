/*
 * SuitsOrderDialog.h
 *
 *       Created on: 18.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef SUITSORDERDIALOG_H_
#define SUITSORDERDIALOG_H_

#include "ButtonsDialog.h"

class SuitsOrderDialog: ButtonsDialog {
	GtkWidget *m_combo[4];
	GtkWidget *m_radio[2];
public:
	SuitsOrderDialog();
	virtual ~SuitsOrderDialog();
	void comboChanged(GtkWidget *w);
	bool click(int index) override;

};

#endif /* SUITSORDERDIALOG_H_ */
