/*
 * Toolbar.h
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef TOOLBAR_H_
#define TOOLBAR_H_

#include "base/FrameItem.h"

class Toolbar: public FrameItem {
	GtkToolItem * m_contractToolItem;
	GtkWidget *m_contract;
	GtkToolItem * m_trumpToolItem;
	GtkWidget *m_trump;
	GtkToolItem *m_button[TOOLBAR_BUTTON_SIZE];
public:
	GtkWidget* m_tooltip;
private:
	bool m_trumpChanged;
	bool m_needModify;
	bool m_lock;
	//cann't move to FrameItem or Widget need only one copy of pixbufs
	GdkPixbuf *m_pixbuf[TOOLBAR_BUTTON_SIZE][2][2]; //[...][small][enable]
	GdkPixbuf *m_stopPixbuf[2]; //[small] enable always true

	void updateButton(TOOLBAR_BUTTON id);

	gint getComboTrumpPosition() const;
	void setComboTrumpPosition(gint p) const;
	gint getComboContractPosition() const;
	void setComboContractPosition(gint p) const;

public:
	bool isEditEnable() {
		return gtk_widget_get_sensitive(GTK_WIDGET(m_trump));
	}

	void comboboxChanged(GtkWidget* combo);

	Toolbar();
	virtual ~Toolbar();

	virtual void updateAfterCreation() {
		updateGameType();
	}

	virtual void updateGameType();
	virtual void updateLanguage();
	virtual void newGame();

	//don't call function getGameType() already defined in Widget
	GAME_TYPE getCurrentGameType() const;
	void setGameType();

	inline void setTrumpChanged(bool changed) {
		m_trumpChanged = changed;
	}

	inline bool isTrumpChanged() {
		return m_trumpChanged;
	}

	void showToolTip(const gchar *s);

	void hideToolTip() {
		showToolTip("");
	}

	void click(GtkToolItem *toolbutton);

	void setMisere();

	void setTrump(gint trump);
	void setContract(gint contract);

	virtual void updateEdit();
	virtual void updateUndoRedo();
	virtual void updateFindBestState();
	void changeEnableEdit(bool enable);

	virtual void updateThink();

	GdkPixbuf * getPixbuf(TOOLBAR_BUTTON id, bool small, BUTTON_STATE state);
	void drawTooltipBackground(cairo_t *cr);
	void changeShowOption();
};

#endif /* TOOLBAR_H_ */
