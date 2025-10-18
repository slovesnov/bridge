/*
 * SolveAllDealsEmptyLabels.h
 *
 *  Created on: 25.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef HELPER_SOLVEALLDEALSEMPTYLABELS_H_
#define HELPER_SOLVEALLDEALSEMPTYLABELS_H_

#include <gtk/gtk.h>
#include <vector>

class SolveAllDealsEmptyLabels {
public:
	GtkWidget *label;
	int suit;
	int anotherPlayerIndex;
};

using VSolveAllDealsEmptyLabels = std::vector<SolveAllDealsEmptyLabels>;
using VSolveAllDealsEmptyLabelsCIT = VSolveAllDealsEmptyLabels::const_iterator;

#endif /* HELPER_SOLVEALLDEALSEMPTYLABELS_H_ */
