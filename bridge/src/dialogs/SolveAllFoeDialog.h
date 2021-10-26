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
	VGtkWidgetPtr m_labelThread;
	int m_result[MAX_RESULT_SIZE];
	GtkWidget *m_loading[2];
	GtkWidget* m_button[2];
	GtkWidget *m_combo[3];
	GtkWidget *m_grid;
	GtkWidget *m_labelPercentTab2;
	GtkWidget *m_notebook;
	gint64 m_id;//read help in SolveAllFoeDialog.cpp

	SolveAllFoeDialog(int positons);
	~SolveAllFoeDialog();
	int resultSize()const;
	void clickButton(GtkWidget *w);
	void comboChanged(GtkWidget *w);
	void updateLabels();
	void reset();
	void setPositions(int positions);
	void setResults();
	std::string getTotalTimeLabelString();
	std::string getProgressBarString(bool b=true);
	GtkWidget* createTab2();
	void updateTab2();
	void addGridRow(GtkWidget *w,int row);
	void setGridLabels(int contract,const VDouble& v);
	int getPreferansPlayers();
	void updateNumberOfPlayersTab2();
	std::string getPercentString();
};

#endif /* DIALOGS_SOLVEALLFOEDIALOG_H_ */
