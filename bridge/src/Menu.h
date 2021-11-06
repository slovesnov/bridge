/*
 * Menu.h
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef MENU_H_
#define MENU_H_

#include <map>
#include "base/FrameItem.h"
#include "helper/Accelerator.h"

typedef std::map<MENU_ID, GtkWidget*> MenuMap;

const Accelerator ACCELERATOR[] = {
		Accelerator(MENU_NEW, GDK_KEY_N, GDK_CONTROL_MASK),
		Accelerator(MENU_OPEN, GDK_KEY_O, GDK_CONTROL_MASK),
		Accelerator(MENU_SAVE, GDK_KEY_S, GDK_CONTROL_MASK),
		Accelerator(MENU_EDIT,
		GDK_KEY_E, GDK_CONTROL_MASK),
		Accelerator(MENU_EDIT_PROBLEM_LIST,
		GDK_KEY_L, GDK_CONTROL_MASK),
		Accelerator(MENU_EDIT_DESCRIPTION,
		GDK_KEY_D, GDK_CONTROL_MASK),
		Accelerator(MENU_PBN_EDITOR, GDK_KEY_P, GDK_CONTROL_MASK),
		Accelerator(MENU_EXIT, GDK_KEY_Q, GDK_CONTROL_MASK),

		Accelerator(MENU_RANDOM_DEAL, GDK_KEY_G, GDK_CONTROL_MASK),
		Accelerator(MENU_GAME_TYPE, GDK_KEY_T, GDK_CONTROL_MASK),
		Accelerator(MENU_CALCULATOR, GDK_KEY_L, GDK_CONTROL_MASK),
		Accelerator(MENU_SOLVE_ALL_DEALS, GDK_KEY_W, GDK_CONTROL_MASK),
		Accelerator(MENU_ROTATE_BY_90_DEGREES_CLOCKWISE, GDK_KEY_4,	GDK_CONTROL_MASK),
		Accelerator(MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE, GDK_KEY_5,
				GDK_CONTROL_MASK),

		Accelerator(MENU_SELECT_ARROW, GDK_KEY_2, GDK_CONTROL_MASK),
		Accelerator(MENU_SELECT_DECK, GDK_KEY_3, GDK_CONTROL_MASK),

		Accelerator(MENU_UNDOALL, GDK_KEY_Z, GDK_CONTROL_MASK),
		Accelerator(MENU_UNDO, GDK_KEY_Z, GDK_NONE_MASK),
		Accelerator(MENU_REDO, GDK_KEY_X, GDK_NONE_MASK),
		Accelerator(MENU_REDOALL, GDK_KEY_X, GDK_CONTROL_MASK),

		Accelerator(MENU_FIND_BEST_MOVE, GDK_KEY_space, GDK_NONE_MASK),
};

class Menu: public FrameItem {
	MenuMap m_map;
	bool m_signals;
	VGtkWidgetPtr m_top;

	GSList *radioGroup;
	int m_lastRecentSize;

	GtkAccelGroup* m_accelerator[SIZE(ACCELERATOR)];

	//update menu item text if it exists; return true item exists
	void updateMenu(MENU_ID id, const gchar* s) {
		gtk_label_set_text(getLabel(id), s);
	}

	void updateMenu(MenuString menuString) {
		updateMenu(menuString.first, menuString.second.c_str());
	}

	GtkLabel* getLabel(const MENU_ID id) const;

	void insertTopMenu(MenuString menuString);
	void insertSubMenu(MenuString menuString);
	MenuString recentMenuString(int index);
	void insertRecentMenu(int index) {
		insertSubMenu(recentMenuString(index));
	}

	GtkWidget* createImageMenuItem(const char* s, GdkPixbuf *p);

	void blockSignals() {
		m_signals = false;
	}

	void unblockSignals() {
		m_signals = true;
	}

	//if last=false return first subMenu
	GtkMenuShell* getSubMenu(bool last) {
		assert(m_top.size() > 0);
		return GTK_MENU_SHELL(
				gtk_menu_item_get_submenu(GTK_MENU_ITEM( last?m_top.back():m_top[0] )));
	}

	void insertSeparator() {
		gtk_menu_shell_append(getSubMenu(true), gtk_separator_menu_item_new());
	}

	void setItemAttributes(const MENU_ID id[], unsigned size);

public:
	Menu();
	virtual ~Menu();

	 void updateAfterCreation()override;
	 void updateEdit() override{
		setItemAttributes(MENU_EDIT);
	}
	 void updateEstimationType()override;
	 void updateFindBestState()override;

	 void updateGameType()override;
	 void updateLanguage()override;
	 void updateSkin()override;
	 void updateUndoRedo()override;

	void updateNewGameState() {
		setItemAttributes(MENU_NEW);
	}

	//get menu string already in UTF8, fast search because of map
	const gchar* getString(const MENU_ID id) const {
		return gtk_label_get_text(getLabel(id));
	}

	void setItemAttributes(const MENU_ID id);
	void click(const MENU_ID id);
	void updateRecent();

	void addAccelerators(bool add);

	void updateThink()override;

};

extern Menu* gmenu;

#endif /* MENU_H_ */
