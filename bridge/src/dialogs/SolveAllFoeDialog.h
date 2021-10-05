/*
 * SolveAllFoeDialog.h
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_SOLVEALLFOEDIALOG_H_
#define DIALOGS_SOLVEALLFOEDIALOG_H_

#include "ButtonsDialogWithProblem.h"

class SolveAllFoeDialog: public ButtonsDialogWithProblem {
public:
	static const int MAX_RESULT_SIZE = MAX_BRIDGE_HAND_CARDS + 1;
	int m_positions, m_total;
	double m_fraction;
	clock_t m_begin, m_end;
	GtkWidget* m_label[MAX_RESULT_SIZE][2];
	GtkWidget* m_labelTotal;
	GtkWidget* m_labelTotalTime;
	GtkWidget* m_progressBar;
	GtkWidget** m_labelThread;
	int m_result[MAX_RESULT_SIZE];
	GtkWidget *m_loading;
	GtkWidget* m_copyButton;
	GtkWidget *m_combo;
	gint64 m_id;//read help in SolveAllFoeDialog.cpp

	SolveAllFoeDialog(int positons);
	~SolveAllFoeDialog();
	int resultSize()const;
	void clickButton(GtkWidget *w);
	void comboChanged();
	void updateLabels();
	void reset();
	void setPositions(int positions);
	void setResults();
	std::string getTotalTimeLabelString();
	std::string getProgressBarString(bool b=true);
};

#endif /* DIALOGS_SOLVEALLFOEDIALOG_H_ */
