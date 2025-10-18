/*
 * SolveAllDealsDialog.h
 *
 *       Created on: 10.05.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef DIALOGS_SOLVEALLDEALSDIALOG_H_
#define DIALOGS_SOLVEALLDEALSDIALOG_H_

#include <atomic>

#include "ButtonsDialogWithProblem.h"
#include "../helper/SolveAllDealsHelp.h"
#include "../helper/SolveAllDealsEmptyLabels.h"

class SolveAllDealsDialog: public ButtonsDialogWithProblem {
	int m_positions, m_total;
	double m_fraction;
	clock_t m_begin, m_end;
	GtkWidget *m_label[MAX_RESULT_SIZE][2];
	GtkWidget *m_labelTotal;
	GtkWidget *m_labelTotalTime;
	GtkWidget *m_progressBar;
	VGtkWidgetPtr m_labelThread;
	int m_result[MAX_RESULT_SIZE];
	GtkWidget *m_loading[2];
	GtkWidget *m_button[5];
	GtkWidget *m_entry;
	GtkWidget *m_combo[3];
	GtkWidget *m_grid;
	GtkWidget *m_labelPercentTab2;
	GtkWidget *m_notebook;
	GMutex m_mutex;
	std::map<GtkWidget*, SolveAllDealsHelp> m_map;
	std::map<GtkWidget*, SolveAllDealsHelp> m_mapLC;
	VInt m_handCards[2][2];
	VSolveAllDealsEmptyLabels m_vel;
	bool m_runCountThreads;
	GtkWidget *m_exportProgressBar;
	GThread *m_exportThread;
	std::ofstream m_file;
	std::atomic_int m_exportStop;
public:
	gint64 m_id; //read help in SolveAllDealsDialog.cpp

	SolveAllDealsDialog();
	~SolveAllDealsDialog();
	int resultSize() const;
	void clickButton(GtkWidget *w);
	void comboChanged(GtkWidget *w);
	void updateResult(int *result, int size); //multithread new data
	void updateData(); //multithread data changed
	void reset();
	void setPositions(int positions);
	std::string getTotalTimeLabelString();
	std::string getProgressBarString(bool b = true);
	GtkWidget* createTab2();
	void updateTab2();
	void addGridRow(GtkWidget *w, GtkWidget *w1, int row);
	void setGridLabels(int contract, const VDouble &v);
	int getPreferansPlayers();
	void updateNumberOfPreferansPlayers();
	VGtkWidgetPtr getLastWhisterWidgets();
	std::string getPercentString();
	void setPreferans2ndTitleRow();
	void setGridLabel(std::string const &s, int left, int top);
	int getTableRowsTab2();
	std::string getNSEWString(bool ns);
	void close();
	void setPlayersCards();
	void checkChanged(GtkWidget *check, GtkWidget *label);
	void labelClick(GtkWidget *w);
	VInt fixedCards(int i);
	int handCards(int i, int suit, bool fixed);
	void setHandCards();
	void updateEmptyLabel(SolveAllDealsEmptyLabels const &a);
	void stopAndRunSolveAll();
	std::pair<GtkWidget*, GtkWidget*> addClickableLabelWithCheck(GtkWidget *w,
			std::string s);
	void addClickableNameWithCheckAddMap(int n);
	void clearHandCards();
	void exportThread();
	void setExportProbressBar(size_t value, size_t total);
	void stopExportThread();
};

#endif /* DIALOGS_SOLVEALLDEALSDIALOG_H_ */
