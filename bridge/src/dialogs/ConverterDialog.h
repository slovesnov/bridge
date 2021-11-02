/*
 * ConverterDialog.h
 *
 *       Created on: 05.12.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef CONVERTERDIALOG_H_
#define CONVERTERDIALOG_H_

#include "../base/DigitEntry.h"

class ConverterDialog: public ButtonsDialog {
	GtkWidget* m_view;
	GtkWidget* m_help;
	GtkWidget* m_checkSplitEveryFile;
	GtkWidget* m_checkSaveWithPictures;
	GtkWidget* m_checkPreview;
	DigitEntry m_firstSplitNumber;
	VString m_files; //utf8 names

	void enableButtons()override;

public:
	ConverterDialog();
	virtual ~ConverterDialog();
	void update();
	void openUris(char**uris)override;

	bool save(bool many);

	void saveParams();

	bool click(int index)override;

};

#endif /* CONVERTERDIALOG_H_ */
