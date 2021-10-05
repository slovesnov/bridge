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
	GtkWidget** m_label;
	int m_labels;
public:
	AboutDialog();
	virtual ~AboutDialog();
	void click(GtkWidget* label);
};

#endif /* ABOUTDIALOG_H_ */
