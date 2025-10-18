/*
 * DeckArrowSelectionDialog.h
 *
 *       Created on: 16.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef DECKARROWSELECTIONDIALOG_H_
#define DECKARROWSELECTIONDIALOG_H_

#include <atomic>
#include "../helper/SvgParameters.h"
#include "ButtonsDialog.h"

class DeckArrowSelectionDialog: public ButtonsDialog {
	static const int N_TOTAL_DECKS = N_RASTER_DECKS + N_VECTOR_DECKS;
	static const int N_TOTAL_ARROWS = N_RASTER_ARROWS + N_VECTOR_ARROWS;

	int m_deckNumber;
	int m_arrowNumber;
	bool m_isDeck;
	GtkWidget *m_area;
	GtkWidget *m_check;
	GtkWidget *m_radio[MAX(N_TOTAL_DECKS, N_TOTAL_ARROWS)];
	GtkWidget *m_hbox;
	GtkWidget *m_loading;

	GtkWidget *m_label;
	GtkWidget *m_scale;
	GtkAdjustment *m_adjustment;
	VGThreadPtr m_vThread;
	std::atomic_int m_number, m_maxv;
	int m_maxCardHeight;
	int m_maxCardWidth;
	int m_maxArrowSize;
	CSize m_lastDrawnSize;

	VGtkWidgetPtr m_hideV, m_showV;
public:
	DeckArrowSelectionDialog(bool isDeck);
	virtual ~DeckArrowSelectionDialog();
	bool click(int index) override;
	void draw(cairo_t *cr);
	void toggle(GtkWidget *w);

	void onObjectLoaded(int i);
	void loadThread(const int n);
	void valueChanged();
	gdouble getValue();
	int getValueUpperBound();

	void keyPress(int i);

	void onObjectChanged();
	int countMaxSvgWidthForDeck();

	CSize getObjectSize();

	SvgParameters& getSvgParameters();
	SvgParameters& getSvgParameters(int i);

	int getThreadsNumber();
	int getRasterSize() const;
	int getVectorSize();
	bool isScalable(int i) const;
	bool isScalable();
	int& getNumber();
	int getNumberIndex();
	std::string getObjectFileName(int n, bool svg);
	gchar* scaleFormat(gdouble value);
	void setHideShowWidgets();
	void enableButtons() override;
};

#endif /* DECKSARROWELECTIONDIALOG_H_ */
