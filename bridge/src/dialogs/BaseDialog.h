/*
 * BaseDialog.h
 *
 *       Created on: 21.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef BASEDIALOG_H_
#define BASEDIALOG_H_

#include "../base/Widget.h"

class BaseDialog: public Widget {
	void init(const char *title, bool modal, Widget *parent, bool resizable);
	bool m_modal;
protected:
	gint m_returnCode;

	//NEED modal parameter for SolveForAllDeclarers
	BaseDialog(MENU_ID id, bool modal = true, Widget *parent = NULL,
			bool resizable = false) :
			Widget(gtk_dialog_new()) {
		init(id == MENU_INVALID ? gconfig->getTitle().c_str() : getString(id),
				modal, parent, resizable);
	}

	BaseDialog(STRING_ID id, bool modal = true, Widget *parent = NULL,
			bool resizable = false) :
			Widget(gtk_dialog_new()) {
		init(getString(id), modal, parent, resizable);
	}

	inline GtkWidget* getContentArea() {
		return gtk_dialog_get_content_area(GTK_DIALOG(getWidget()));
	}

	void show();
	void showExclude(VGtkWidgetPtr const &v);

	GtkWidget* createButton(const char *img, STRING_ID id = STRING_INVALID);
	GtkWidget* createButton(const char *img, MENU_ID id);
	GtkWidget* createButton(const char *img, const char *str);

	GtkWidget* createTextButton(STRING_ID id);
	GtkWidget* createTextButton(MENU_ID id);
	GtkWidget* createTextButton(const char *str);

public:
	virtual ~BaseDialog();
	gint getReturnCode() const {
		return m_returnCode;
	}

};

#endif /* BASEDIALOG_H_ */
