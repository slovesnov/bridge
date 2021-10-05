/*
 * FrameItem.cpp
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "FrameItem.h"
#include "../Frame.h"

FrameItem::FrameItem(GtkWidget*widget) :
		Widget(widget) {
	setDragDrop(getWidget());
}

FrameItem::~FrameItem() {
}
