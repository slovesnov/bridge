/*
 * Accelerator.h
 *
 *       Created on: 03.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef ACCELERATOR_H_
#define ACCELERATOR_H_

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "Enums.h"

const GdkModifierType GDK_NONE_MASK = static_cast<GdkModifierType>(0);

class Accelerator {
public:
	MENU_ID id;
	guint key;
	GdkModifierType mask;

	Accelerator(MENU_ID _id, int _key, GdkModifierType _mask) {
		id = _id;
		key = _key;
		mask = _mask;
	}

	//for search by event. Note 'id' is not used
	Accelerator(GdkEventKey *event) {
		id = MENU_INVALID;
		key = event->keyval;
		mask = static_cast<GdkModifierType>(event->state);
	}

	//for search by event. Note 'id' is not used
	inline bool operator==(Accelerator const& a) const {
		return key == a.key && mask == a.mask;
	}
};

#endif /* ACCELERATOR_H_ */
