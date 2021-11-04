/*
 * SolveAllDealsDialog.h
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DIALOGS_SOLVEALLDEALSDIALOG_H_
#define DIALOGS_SOLVEALLDEALSDIALOG_H_

#include "ButtonsDialogWithProblem.h"

class SolveAllDealsDialog: public ButtonsDialogWithProblem {
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
	GMutex m_mutex;
public:
	gint64 m_id;//read help in SolveAllDealsDialog.cpp

	SolveAllDealsDialog(int positons);
	~SolveAllDealsDialog();
	int resultSize()const;
	void clickButton(GtkWidget *w);
	void comboChanged(GtkWidget *w);
	void updateResult(int *result,int size);//multithread new data
	void updateData();//multithread data changed
	void reset();
	void setPositions(int positions);
	std::string getTotalTimeLabelString();
	std::string getProgressBarString(bool b=true);
	GtkWidget* createTab2();
	void updateTab2();
	void addGridRow(GtkWidget *w,GtkWidget *w1, int row);
	void setGridLabels(int contract,const VDouble& v);
	int getPreferansPlayers();
	void updateNumberOfPreferansPlayers();
	VGtkWidgetPtr getLastWhisterWidgets();
	std::string getPercentString();
	void setPreferans2ndTitleRow();
	void setGridLabel(std::string const&s,int left,int top);
	int getTableRowsTab2();
	std::string getNSEWString(bool ns);
};

#endif /* DIALOGS_SOLVEALLDEALSDIALOG_H_ */
