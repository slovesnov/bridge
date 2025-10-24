/*
 * FrameItemArea.h
 *
 *       Created on: 09.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef FRAMEITEMAREA_H_
#define FRAMEITEMAREA_H_

#include "FrameItem.h"
#include "CairoSurface.h"

class FrameItemArea: public FrameItem {
protected:
	CairoSurface m_cs;

public:
	FrameItemArea(GtkWidget *widget = NULL);
	virtual ~FrameItemArea();

	inline void redraw() { //redraw in memory & show on screen
		draw();
		paint();
	}

	virtual void changeShowOption();

	virtual void copySurface(cairo_t *cr) {
		cairo_set_source_surface(cr, m_cs, 0, 0);
		cairo_paint(cr);
	}

	void initResizeRedraw();

	virtual CPoint getSize() const =0;

protected:

	virtual void resize() {
		CPoint sz = getSize();
		gtk_widget_set_size_request(getWidget(), sz.x, sz.y);
	}

	virtual void updateEdit();

	virtual void updateGameType() {
		newGame();
	}

	virtual void updateDeckSelection() { //size of DrawingArea could change so need to redraw background & hence full redraw
		updateAfterCreation();
	}

	virtual void init();

	virtual void draw()=0;

	void drawLine(int x1, int y1, int x2, int y2);
	inline void drawHorizontalLine(int x1, int y, int x2) {
		drawLine(x1, y, x2, y);
	}
	inline void drawVerticalLine(int x, int y1, int y2) {
		drawLine(x, y1, x, y2);
	}

	void copyFromBackground(gint destx, gint desty, gint width, gint height,
			gint sourcex, gint sourcey);

	inline void copyFromBackground(gint x, gint y, gint width, gint height) {
		copyFromBackground(x, y, width, height, x, y);
	}

	inline void copyFromBackground(CRect const &r) {
		copyFromBackground(r.left, r.top, r.width(), r.height());
	}

	inline void paint() { //draw on screen from memory
		gtk_widget_queue_draw(getWidget());
	}

	void updateSkin() {
		redraw();
	}

	void drawText(TextWithAttributes text, CRect r, bool centerx,
			bool centery) {
		drawTextToCairo(m_cs, text, r, centerx, centery);
	}

	void drawText(TextWithAttributes text, int x, int y) {
		CRect r(CPoint(x, y), CPoint(0, 0));
		drawText(text, r, false, false);
	}

public:
	CPoint getTextExtents(TextWithAttributes text);
	CPoint getTextExtents(TextWithAttributes text, cairo_t *cr);
protected:

	cairo_surface_t* getDeckSurface() const;

	void copyFromDeck(cairo_t *cr, int destx, int desty, int index) {
		copyFromDeck(cr, destx, desty, getCardSize().x, getCardSize().y,
				index, 0, 0);
	}

	void copyFromDeck(cairo_t *cr, int destx, int desty, int width, int height,
			int index, int addx, int addy);

	CRect getInsideRect(const CRect &r, CARD_INDEX index);

};

#endif /* FRAMEITEMAREA_H_ */
