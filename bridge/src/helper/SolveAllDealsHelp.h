/*
 * SolveAllDealsHelp.h
 *
 *  Created on: 24.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef HELPER_SOLVEALLDEALSHELP_H_
#define HELPER_SOLVEALLDEALSHELP_H_

#include <gtk/gtk.h>
#include "../solver/BridgeCommon.h"

class SolveAllDealsHelp {
public:
	GtkWidget *check;
	int index;
	int playerIndex;
};

#endif /* HELPER_SOLVEALLDEALSHELP_H_ */
