/*
 * FrameItemArea.cpp
 *
 *       Created on: 09.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "FrameItemArea.h"
#include "../ProblemSelector.h"

static void on_draw_event(GtkWidget *widget, cairo_t *cr, FrameItemArea *area) {
	area->copySurface(cr);
}

FrameItemArea::FrameItemArea(GtkWidget *widget) :
		FrameItem(widget == NULL ? gtk_drawing_area_new() : widget) {

	g_signal_connect(getWidget(), "draw", G_CALLBACK(on_draw_event), this);
}

FrameItemArea::~FrameItemArea() {
}

void FrameItemArea::drawLine(int x1, int y1, int x2, int y2) {
	const GdkRGBA rgba = getTextColor();
	cairo_t *cr = m_cs;
	gdk_cairo_set_source_rgba(cr, &rgba);
	cairo_set_line_width(cr, 1.);

	//+0.5 to make line width equals 1
	cairo_move_to(cr, x1 + 0.5, y1 + 0.5);
	cairo_line_to(cr, x2 + 0.5, y2 + 0.5);
	cairo_stroke(cr);
}

void FrameItemArea::init() {
	m_cs.create(getSize()); //uses for LastTrick, ProblemSelector, DrawingArea
}

void FrameItemArea::copyFromBackground(int destx, int desty, int width,
		int height, int sourcex, int sourcey) {
	copy(getBackgroundFullSurface(), m_cs, destx, desty, width, height, sourcex,
			sourcey);
}

void FrameItemArea::changeShowOption() {
	updateAfterCreation();
}

CPoint FrameItemArea::getTextExtents(TextWithAttributes text) {
	return getTextExtents(text, m_cs);
}

CPoint FrameItemArea::getTextExtents(TextWithAttributes text, cairo_t *cr) {
	CPoint sz;
	PangoLayout *layout = createPangoLayout(cr, text);
	pango_layout_get_pixel_size(layout, &sz.x, &sz.y);
	g_object_unref(layout);
	return sz;
}

cairo_surface_t* FrameItemArea::getDeckSurface() const {
	return getProblemSelector().m_deck;
}

void FrameItemArea::copyFromDeck(cairo_t *cr, int destx, int desty, int width,
		int height, int index, int addx, int addy) {
	copy(getDeckSurface(), cr, destx, desty, width, height,
			(12 - index % 13) * getCardSize().x + addx,
			index / 13 * getCardSize().y + addy);
}

CRect FrameItemArea::getInsideRect(const CRect &r, CARD_INDEX index) {
	int x = r.centerPoint().x;
	int y = r.centerPoint().y;

	const CPoint cs = getCardSize();
	const int d = -cs.x * 15 / 71;
	const CPoint sz = isBridge() ? INNER_CARD_MARGIN : CPoint(d, d);

	if (northOrSouth(index)) {
		x -= cs.x / 2;
		if (isPreferans() && index == CARD_INDEX_SOUTH) {
			if (getAbsent() == CARD_INDEX_EAST) {
				x -= d;
			} else if (getAbsent() == CARD_INDEX_WEST) {
				x += d;
			}
		}
	} else if (index == CARD_INDEX_WEST) {
		x -= cs.x + sz.x;
	} else if (index == CARD_INDEX_EAST) {
		x += sz.x;
	}

	if (isPreferans() && northOrSouth(getAbsent())) {
		if (index == CARD_INDEX_EAST) {
			y -= cs.y / 2;
		} else if (index == CARD_INDEX_NORTH) {
			y -= cs.y + sz.x;
		} else if (index == CARD_INDEX_SOUTH) {
			y += sz.x;
		} else if (index == CARD_INDEX_WEST) {
			if (getAbsent() == CARD_INDEX_NORTH) {
				y -= cs.y + sz.x;
			} else {
				y += sz.x;
			}
		}
	} else {
		if (eastOrWest(index)) {
			y -= cs.y / 2;
		} else if (index == CARD_INDEX_NORTH) {
			y -= cs.y + sz.x;
		} else if (index == CARD_INDEX_SOUTH) {
			y += sz.x;
		}
	}

	return CRect(CPoint(x, y), getCardSize());
}

void FrameItemArea::initResizeRedraw() {
	init();
	resize();
	redraw();
}

void FrameItemArea::updateEdit() {
	initResizeRedraw();
}
