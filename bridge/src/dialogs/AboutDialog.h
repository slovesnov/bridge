/*
 * AboutDialog.h
 *
 *       Created on: 20.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef ABOUTDIALOG_H_
#define ABOUTDIALOG_H_

#include "BaseDialog.h"

class AboutDialog: BaseDialog {
	VGtkWidgetPtr m_label;
	Pixbuf m_suits[4];
public:
	AboutDialog();
	void click(GtkWidget* label);
	void createSvgSuits(int size);
};

#endif /* ABOUTDIALOG_H_ */
