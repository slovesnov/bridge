/*
 * LastTrick.h
 *
 *       Created on: 09.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef LASTTRICK_H_
#define LASTTRICK_H_

#include "base/FrameItemArea.h"

/* Last trick and best line grid
 */

class LastTrick: public FrameItemArea {
	GtkWidget* m_grid;
	GtkWidget* m_scrolled;
	GdkPixbuf* m_suitPixbuf[4];
	GtkWidget* m_suit[52];
	GtkWidget* m_labelCard[52];
	int m_rows;
	int m_columns;
	VString m_vLastTrick,m_vGameAnalysis;
public:
	GtkWidget* m_full;

	void setLastTrickGameAnalysisStrings();

	LastTrick();
	virtual ~LastTrick();

	void newGame() override;

	bool isEmpty();

	void draw() override;
	CSize getSize() const override;

	void updateLanguage() override;
	void updateAfterCreation() override;

	void updateEdit() override;
	void updateDeckSelection() override;
	void updateFontSelection() override;
	void setDeal(bool random) override;

	void drawGridBackground(cairo_t *cr);

	void cellClick(int n);
	void drawBestLine();

	void gridMouseEnter(int i);
	CSize getVisibleSize() const;
	CSize getFullVisibleSize() const;

	void setSuitPixbufs();
	void freeSuitPixbufs();

	void updateSkin() override;
};

extern LastTrick* glasttrick;

#endif /* LASTTRICK_H_ */
