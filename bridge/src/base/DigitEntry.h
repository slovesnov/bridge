/*
 * DigitEntry.h
 *
 *       Created on: 16.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef BASE_DIGITENTRY_H_
#define BASE_DIGITENTRY_H_

#include "../dialogs/ButtonsDialog.h"

class DigitEntry {
public:
	GtkWidget* m_widget;
	int m_min;
	int m_max;
	bool m_valid;
	int m_value;
	ButtonsDialog* m_dialog;

	void create(int min, int max, int startValue, ButtonsDialog*dialog);
	virtual ~DigitEntry();

	void check();
	void highlight();

};

#endif /* BASE_DIGITENTRY_H_ */
