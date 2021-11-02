/*
 * FrameItem.h
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef FRAMEITEM_H_
#define FRAMEITEM_H_

#include "Widget.h"

class FrameItem: public Widget {

public:
	FrameItem(GtkWidget*widget);
	virtual ~FrameItem();
	virtual void updateAfterCreation()=0; //update item when all objects are created
	virtual void updateEdit() {
	}

	virtual void updateGameType() {
	}

	virtual void updateFindBestState() {
	}

	virtual void updateEstimationType() {
	}

	virtual void newGame() {
	}

	virtual void setDeal(bool random) {
	}

	virtual void updateLanguage() {
	}

	virtual void updateSkin() {
	}

	virtual void updateUndoRedo() {
	}

	virtual void updateDeckSelection() {
	}

	virtual void updateArrowSelection() {
	}

	virtual void updateThink() {
	}

	virtual void updateFontSelection() {
	}

	virtual void updateResetSettings()final;
};

typedef void (FrameItem::*FrameItemFunction)();
typedef std::vector<FrameItem*> VFrameItemPointer;

#endif /* FRAMEITEM_H_ */
