/*
 * FrameItemArea.cpp
 *
 *       Created on: 09.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "FrameItemArea.h"
#include "../ProblemSelector.h"

static void on_draw_event(GtkWidget *widget, cairo_t *cr, FrameItemArea* area) {
	area->copySurface(cr);
}

FrameItemArea::FrameItemArea(GtkWidget*widget) :
		FrameItem(widget == NULL ? gtk_drawing_area_new() : widget) {
	m_cr = NULL;
	m_surface = NULL;

	g_signal_connect(getWidget(), "draw", G_CALLBACK(on_draw_event), this);
}

FrameItemArea::~FrameItemArea() {
	destroy(m_cr);
	destroy(m_surface);
}

void FrameItemArea::drawLine(int x1, int y1, int x2, int y2) {
	const GdkRGBA rgba = getTextColor();
	gdk_cairo_set_source_rgba(m_cr, &rgba);
	cairo_set_line_width(m_cr, 1.);

	//+0.5 to make line width equals 1
	cairo_move_to(m_cr, x1 + 0.5, y1 + 0.5);
	cairo_line_to(m_cr, x2 + 0.5, y2 + 0.5);
	cairo_stroke(m_cr);
}

void FrameItemArea::init() {
	createNew(m_cr, m_surface, getSize()); //uses for LastTrick, ProblemSelector, DrawingArea
}

void FrameItemArea::copyFromBackground(int destx, int desty, int width,
		int height, int sourcex, int sourcey) {
	copy(getBackgroundFullSurface(), m_cr, destx, desty, width, height, sourcex,
			sourcey);
}

void FrameItemArea::changeShowOption() {
	updateAfterCreation();
}

CSize FrameItemArea::getTextExtents(TextWithAttributes text) {
	CSize sz;
	PangoLayout *layout = createPangoLayout(text,m_cr);
	pango_layout_get_pixel_size(layout, &sz.cx, &sz.cy);
	g_object_unref(layout);
	return sz;
}

cairo_surface_t* FrameItemArea::getDeckSurface() const {
	return getProblemSelector().m_deckSurface;
}

CRect FrameItemArea::getInsideRect(const CRect& r, CARD_INDEX index) {
	int x = r.centerPoint().x;
	int y = r.centerPoint().y;

	const CSize cs = getCardSize();
	const int d = -cs.cx * 15 / 71;
	const CSize sz = isBridge() ? getInnerCardMargin() : CSize(d, d);

	if (northOrSouth(index)) {
		x -= cs.cx / 2;
		if (isPreferans() && index == CARD_INDEX_SOUTH) {
			if (getAbsent() == CARD_INDEX_EAST) {
				x -= d;
			}
			else if (getAbsent() == CARD_INDEX_WEST) {
				x += d;
			}
		}
	}
	else if (index == CARD_INDEX_WEST) {
		x -= cs.cx + sz.cx;
	}
	else if (index == CARD_INDEX_EAST) {
		x += sz.cx;
	}

	if (isPreferans() && northOrSouth(getAbsent())) {
		if (index == CARD_INDEX_EAST) {
			y -= cs.cy / 2;
		}
		else if (index == CARD_INDEX_NORTH) {
			y -= cs.cy + sz.cx;
		}
		else if (index == CARD_INDEX_SOUTH) {
			y += sz.cx;
		}
		else if (index == CARD_INDEX_WEST) {
			if (getAbsent() == CARD_INDEX_NORTH) {
				y -= cs.cy + sz.cx;
			}
			else {
				y += sz.cx;
			}
		}
	}
	else {
		if (eastOrWest(index)) {
			y -= cs.cy / 2;
		}
		else if (index == CARD_INDEX_NORTH) {
			y -= cs.cy + sz.cx;
		}
		else if (index == CARD_INDEX_SOUTH) {
			y += sz.cx;
		}
	}

	return CRect(CPoint(x, y), getCardSize());
}
