/*
 * DrawingArea.cpp
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "DrawingArea.h"
#include "Frame.h"
#include "problem/ProblemVector.h"
#include "dialogs/SolveForAllDeclarersDialog.h"
#include "dialogs/HtmlOptionsDialog.h"
#include "dialogs/PbnEditorDialog.h"
#include "dialogs/SolveAllFoeDialog.h"
#include "solver/Bridge.h"
#include "solver/Permutations.h"
#include "solver/Preferans.h"

//not use member of DrawingArea to avoid long compilation time
Bridge bridge;
Preferans preferans;

/* BRIDGE_BEST_MULTITHREAD working only if ESTIMATE_ALL_LOCAL || ESTIMATE_ALL_TOTAL
 * otherwise not need
 *
 * #ifdef BRIDGE_BEST_MULTITHREAD estimate all moves at first after find
 * optimal move from them, and not need later estimate
 *
 * #ifndef BRIDGE_BEST_MULTITHREAD at first find best move then multithreading
 * estimate other moves using bestmove estimation
 *
 */

//#define BRIDGE_BEST_MULTITHREAD

const CARD_INDEX OUTER_REGION[] = {
		CARD_INDEX_ABSENT,
		CARD_INDEX_NORTH,
		CARD_INDEX_EAST,
		CARD_INDEX_SOUTH,
		CARD_INDEX_WEST };

/* number of steps should be PREFERANS_SOLVE_ALL_FOE_STEPS%CORES=0 to get max processor loading
 * usually cores=2,4,6,8 so PREFERANS_SOLVE_ALL_FOE_STEPS%24=0 is good solution
 * PREFERANS_SOLVE_ALL_FOE_POSITIONS/24 ~ 7700
 */
const int PREFERANS_SOLVE_ALL_FOE_STEPS = 48;

const GdkRGBA ESTIMATE_COLOR = { 1, 1, 204. / 255, 1 };

DrawingArea* gdraw;

static gpointer solve_thread(gpointer data) {
	gdraw->solveThread((Problem*) data);
	return NULL;
}

static gboolean end_solve_thread(gpointer data) {
	gdraw->endSolveThread();
	return G_SOURCE_REMOVE;
}

static gboolean timer_animation_handler(gpointer data) {
	return gdraw->animationStep(GP2INT(data));
}

static gboolean timer_handler(gpointer) {
	gdraw->timer();
	return TRUE;
}

static void setEstimateFunction(int index, int estimate) { //callback function
	gdraw->setShowEstimation(index, estimate, true);
}

static gpointer solve_all_declarers_bridge_thread(gpointer data) {
#ifdef FINAL_RELEASE
	gdraw->solveAllDeclarersBridgeThread();
#else
	gdraw->solveAllDeclarersBridgeThread(GP2INT(data));
#endif
	return NULL;
}

static gpointer solve_all_declarers_preferans_thread(gpointer) {
	gdraw->solveAllDeclarersPreferansThread();
	return NULL;
}

static gpointer solve_all_foe_thread(gpointer data) {
	gdraw->solveAllFoeThread(GP2INT(data));
	return NULL;
}

static gboolean solve_all_bridge_set_labels(gpointer data) {
	gdraw->solveAllBridgeSetLabels(GP2INT(data));
	return G_SOURCE_REMOVE;
}

static gboolean solve_all_foe_update_result(gpointer data) {
	gdraw->solveAllFoeUpdateResult(gint64(data));
	return G_SOURCE_REMOVE;
}

static gboolean show_estimation_thread(gpointer data) {
	gdraw->showEstimation(GP2INT(data));
	return G_SOURCE_REMOVE;
}

static gboolean update_inside_region_thread(gpointer) {
	gdraw->updateInsideRegion();
	return G_SOURCE_REMOVE;
}

static gboolean make_move_thread(gpointer data) {
	int i=GP2INT(data);
	gdraw->makeMove(i&0xff,i>>8);
	return G_SOURCE_REMOVE;
}

static gboolean draw_bestline_thread(gpointer data) {
	glasttrick->drawBestLine();
	return G_SOURCE_REMOVE;
}

static gboolean mouse_press_event(GtkWidget *widget, GdkEventButton *event,
		gpointer) {
	//event->button =1 left mouse button
	//event->button =2 middle mouse button
	//event->button =3 right mouse button
	if (event->button == 1) {
		gdraw->mouseLeftButtonDown(event);
	}
	else if (event->button == 3) {
		//only one click
		if (event->type != GDK_DOUBLE_BUTTON_PRESS && !gdraw->isEmptyDeal()) {
			gdraw->menuClick(MENU_FIND_BEST_MOVE);//do all calls using menuClick, because of additional functions, for example hide ProblemSelector
		}
	}
	return TRUE;
}

static gboolean mouse_release_event(GtkWidget *widget, GdkEventButton *event,
		gpointer) {
	if (event->button == 1) {
		gdraw->mouseLeftButtonUp(event);
	}
	return TRUE;
}

static gboolean mouse_move_event(GtkWidget *widget, GdkEventButton *event,
		gpointer) {
	gdraw->mouseMove(event);
	return TRUE;
}

//Mouse out signal. Note second parameter GdkEventCrossing not GdkEventButton
static gboolean mouse_leave_event(GtkWidget *widget, GdkEventCrossing *event,
		gpointer) {
	gdraw->mouseLeave(event);
	return TRUE;
}

DrawingArea::DrawingArea() :
		FrameItemArea() {
	int i;

	gdraw = this;
	m_solveThread=0;
	m_pThread = new GThread*[getMaxRunThreads()];
	m_solveAllPtr = new SolveAll[getMaxRunThreads()];

	for (i = 0; i < SIZEI(m_totalTricksImage); i++) {
		m_totalTricksImage[i] = NULL;
		createNew(m_totalTricksImage[i], pixbuf(format("back%c.gif", i == 0 ? 'h' : 'v')));
	}
	m_totalTricksImageSize0 = CSize(gdk_pixbuf_get_width(m_totalTricksImage[0]),
			gdk_pixbuf_get_height(m_totalTricksImage[0]));

	m_crEnd = NULL;
	m_surfaceEnd = NULL;

	m_selectedCard = -1;
	m_currentId = -1;
	m_tableRect.top = 0;

	m_lastRegion = CARD_INDEX_NORTH;//click with control moves cards to NORTH by default

	m_timer = 0;
	m_animationTimer=0;
	m_startTime = 0;
	m_estimateFontHeight = 1;

	m_animationStep = 0;

	m_pstate=0;
	m_pstateSize=0;

	g_mutex_init(&m_mutex);
	g_cond_init(&m_condition);

	g_mutex_init(&m_solveAllMutex);

	//enable mouse down & up & motion
	gtk_widget_add_events(getWidget(),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK
					| GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect(getWidget(), "button_press_event",
			G_CALLBACK(mouse_press_event), NULL);
	g_signal_connect(getWidget(), "button-release-event",
			G_CALLBACK(mouse_release_event), NULL);
	g_signal_connect(getWidget(), "motion-notify-event",
			G_CALLBACK(mouse_move_event), NULL);
	g_signal_connect(getWidget(), "leave-notify-event",
			G_CALLBACK(mouse_leave_event), NULL);

}

DrawingArea::~DrawingArea() {
	int i;

	for (i = 0; i < SIZEI(m_totalTricksImage); i++) {
		destroy(m_totalTricksImage[i]);
	}

	destroy(m_crEnd);
	destroy(m_surfaceEnd);

	g_mutex_clear(&m_mutex);
	g_cond_clear(&m_condition);

	g_mutex_clear(&m_solveAllMutex);

	delete[] m_pThread;
	delete[] m_solveAllPtr;
	freePState();
}

void DrawingArea::draw() {
	int x, y;
	CSize sz = getSize();
	copyFromBackground(0, 0, sz.cx, sz.cy);
	const int radius = m_totalTricksImageSize0.cx;

	//draw lines
	if (m_tableRect.top == 0) {
		drawHorizontalLine(m_tableRect.left + radius, 0,
				m_tableRect.right - radius);
	}
	else {
		drawHorizontalLine(0, m_tableRect.top, m_windowSize.cx);
	}

	for (x = 0, y = m_tableRect.bottom; x < 2; x++, y += m_tableTop + 1) {
		drawHorizontalLine(0, y, m_windowSize.cx);
	}

	drawVerticalLine(m_windowSize.cx - 1, 0, m_windowSize.cy);

	x = m_tableRect.left;
	y = m_tableRect.top;
	cairo_new_sub_path(m_cr);
	cairo_arc(m_cr, x + m_tableRect.width() + 0.5 - radius, y + radius + 0.5,
			radius, -G_PI / 2, 0);	//top right
	cairo_arc(m_cr, x + m_tableRect.width() + 0.5 - radius,
			y + m_tableRect.height() + 1 - radius - 0.5, radius, 0, G_PI / 2);//bottom right
	cairo_arc(m_cr, x + radius + 0.5, y + m_tableRect.height() + 1 - radius - 0.5,
			radius, G_PI / 2, G_PI);	//bottom left
	cairo_arc(m_cr, x + radius + 0.5, y + radius + 0.5, radius, G_PI,
			3 * G_PI / 2);	//top left
	cairo_stroke(m_cr);

	updateAllRegions();

}

void DrawingArea::updateLanguage() {
	/* 29apr2021 for smallest decks size of east/west depends on language
	 * sometimes "East tricks 13" (in russian language) is wider then eight cards in preferans
	 * game type for 150dpi so recount size every time
	 * */
	countSize();
	initResizeRedraw();
}

void DrawingArea::updateRegion(CARD_INDEX index, bool paint) {
	if ((index == CARD_INDEX_ABSENT && !isEditEnable())
			|| (isPreferans() && index == getAbsent())) {
		return;
	}
	assert(!isInner(index));

	int i, j, k, m;
	CSize sz;
	CRect update = getRegionRect(index);
	copyFromBackground(update);

	/*
	 DO NOT REMOVE. Note. To view update regions uncomment next lines and comment body of "showCard" function
	 also check with "east west cards margin = 0"
	 cairo_set_source_rgb(m_cr, 1, 0, 0);
	 cairo_set_line_width(m_cr, 1);
	 cairo_rectangle(m_cr, update.left+0.5, update.top+0.5, update.width()-1, update.height()-1);
	 cairo_stroke(m_cr);
	 */

	//draw number of tricks of message for absent cards before showing cards
	if (index == CARD_INDEX_ABSENT) {
		double x, y;
		i = 0;
		for (auto s : getCaptions(index)) {
			getCaptionPoint(index, i, x, y);
			drawText(TextWithAttributes(s), x, y);
			i++;
		}
	}
	else if (index >= CARD_INDEX_NORTH && index <= CARD_INDEX_WEST) {
		updateTricks(index, paint);
	}

	//south or north can have all 52 cards they just will cover player name and number of tricks
	for (m = k = 0; k < 4; k++) {
		for (j = 0; j < 13; j++) {
			i = gconfig->getCard(k, j);
			if (getState().m_cid[i] == index) {
				if (index == 0 && m_cardrect[i].left > m) {
					m = m_cardrect[i].left;
				}
				if (i == m_currentId) {
					continue;
				}
				showCard(m_cr, i, m_cardrect[i].left, m_cardrect[i].top);
			}
		}
	}

	if (paint) {
		invalidateRect(update);
	}
}

void DrawingArea::updateAllRegions() {
	unsigned i;
	for (i = 0; i < SIZE(OUTER_REGION); ++i) {
		updateRegion(OUTER_REGION[i]);
	}
	updateInsideRegion();
}

void DrawingArea::updateTricks() {
	unsigned i;
	for (i = 0; i < SIZE(PLAYER); i++) {
		updateTricks(PLAYER[i]);
	}
}

void DrawingArea::updateTricks(CARD_INDEX index, bool paint) {
	if ((index == CARD_INDEX_ABSENT && !isEditEnable())
			|| (index == getAbsent() && isPreferans())) {
		return;
	}

	int i,j;
	bool underline = isPreferans() && getProblem().m_player == index;
	double x, y;
	CSize sz;
	std::string s;
	CRect update;

	i = 0;
	for (auto s : getCaptions(index)) {
		getCaptionPoint(index, i, x, y);
		TextWithAttributes text(s + "0", underline);
		sz = getTextExtents(text);
		update = CRect(CPoint(x, y + 1), sz);
		if (index == CARD_INDEX_EAST) {
			j=m_tableRect.right+m_tableRect.left;
			if (update.right > j) {
				update.right = j;
			}
		}
		copyFromBackground(update);
		drawText(TextWithAttributes(s, underline), x, y);

		if (paint) {
			invalidateRect(update);
		}
		if (!gconfig->m_showPlayerTricks && i == 0) {
			break;
		}
		i++;
	}

}

bool DrawingArea::pointInCaption(CARD_INDEX index, GdkEventButton* event) {
	int i = 0;
	CSize sz;
	double x, y;
	for (auto s : getCaptions(index)) {
		getCaptionPoint(index, i, x, y);
		sz = getTextExtents(TextWithAttributes::createUnderlinedText(s));
		if (CRect(CPoint(int(x), int(y)), sz).in(event)) {
			return true;
		}
		i++;
	}
	return false;
}

void DrawingArea::getCaptionPoint(CARD_INDEX index, int piece, double&x,
		double&y) {
	auto i = getRealRegion(index);
	auto v = getCaptions(index);
	assert(piece < int(v.size()));
	CSize sz = getTextExtents(TextWithAttributes::createUnderlinedText(v[piece]));

	if (eastOrWest(i)) {
		if (index == CARD_INDEX_ABSENT) {
			x = (m_tableRect.left - sz.cx) / 2;
		}
		else {
			assert((!piece) < int(v.size()));
			int wo = getTextExtents(
					TextWithAttributes::createUnderlinedText(v[!piece])).cx;
			x = (m_tableRect.left - sz.cx - wo) / 3;	//x=space

			if (piece == 1) {
				x = 2 * x + wo;
			}

		}
		if (i == CARD_INDEX_EAST) {
			x += m_tableRect.right;
		}

		y = m_tableRect.top;
		return;
	}
	if (index == CARD_INDEX_ABSENT) {
		x = m_tableRect.centerPoint().x - sz.cx / 2;
		y = (m_tableRect.top - sz.cy) / 2;
		if (isBridge()) {
			y += m_tableRect.bottom + m_tableRect.top;
		}
		else if (i == CARD_INDEX_SOUTH) {
			y += m_tableRect.bottom;
		}
		return;
	}

	getCaptionPoint(CARD_INDEX_WEST, 0, x, y);	//x is the same with WEST
	y = index == CARD_INDEX_NORTH ? 0 : m_tableRect.bottom;
	if (gconfig->m_showPlayerTricks) {
		y += (piece + 1) * (m_tableTop - 2 * sz.cy) / 3;
		if (piece == 1) {
			y += sz.cy;
		}
	}
	else {
		y += (m_tableTop - sz.cy) / 2;
	}
}

VString DrawingArea::getCaptions(CARD_INDEX index) {
	VString v;
	std::string s;
	if (index == CARD_INDEX_ABSENT) {
		if (isPreferans() && eastOrWest(getAbsent())) {
			s = getString(STRING_ROTATE_BY_90_DEGREES);
			v.push_back(s);
		}
		else {
			s = getString(STRING_DRAG_ABSENT_CARDS_HERE);
			if (isPreferans()) {
				s += "\n";
				s += getString(STRING_CLICK_TO_ROTATE_BY_90_DEGREES);
			}
			v.push_back(s);
		}
	}
	else {
		/*return always 2 strings even not show every player tricks
		 2nd string uses to count coordinates of first string in vector
		 */
		v.push_back(::getPlayerString(index));
		/* in case of absent is west, to define North/South x coordinates is called
		 * but getTricks(index) could be any number but needs get width of
		 * "tricks 0" not "tricks 1234567"
		 */
		v.push_back(
				getString(STRING_TRICKS)
						+ format(" %d", isPreferans() && getAbsent() == index ? 0 : getTricks(index)));
	}
	return v;
}

//return CARD_INDEX_ABSENT as absent region for bridge
CARD_INDEX DrawingArea::getRealRegion(CARD_INDEX index) {
	auto i = index;
	if (isPreferans() && index == CARD_INDEX_ABSENT) {
		i = getAbsent();
	}
	return i;
}

void DrawingArea::updateInsideRegion() {
	int i, j, k;
	int a[4];
	CARD_INDEX f;

	//inner cards
	getProblem().fillInner4(a);
	for (i = 0, f = getState().m_firstmove; i < 4;
			i++, f = getNextBridgePlayer(f)) {
		if (a[i] == -1) {
			copyFromBackground(getInsideRect(f));
		}
	}
	for (i = 0; i < 4; i++) {
		k = a[i];
		if (k != -1) {
			showCard(m_cr, k, m_cardrect[k].left, m_cardrect[k].top);
		}
	}

	//arrow
	for (j = 0; j < 4; j++) {	//clear arrow rectangles
		copyFromBackground(getArrowRect(PLAYER[j]));
	}
	showArrow(false);

	//show total tricks
	if (gconfig->m_showCommonTricks) {
		for (i = 0; i < SIZEI(m_totalTricksRect); i++) {
			copyFromPixbuf(m_totalTricksImage[i], m_cr, m_totalTricksRect[i]);
			if (isBridge()) {
				k = i == 0 ? 1 : 0;
				j = getState().m_tricks[k] + getState().m_tricks[k + 2];
			}
			else {
				for (j = k = 0; k < PREFERANS_PLAYER_SIZE; k++) {
					if ((i == 0 && getPreferansPlayer(k) == getProblem().m_player)
							|| (i != 0 && getPreferansPlayer(k) != getProblem().m_player)) {
						j += getTricks(getPreferansPlayer(k));
					}
				}
			}
			//Note background has the same color for all skins, so for tricks text always is black
			drawText(
					TextWithAttributes::createAllTricksText(j, i == 0 && isPreferans()),
					m_totalTricksRect[i], true, true);
		}
	}

//	drawHorizontalLine(m_tableRect.left,m_tableRect.centerPoint().y,m_tableRect.right);
//	drawVerticalLine(m_tableRect.centerPoint().x,m_tableRect.top,m_tableRect.bottom);

	invalidateRect(m_tableRect);

}

CRect DrawingArea::getRegionRect(CARD_INDEX index) {
	/*Notes
	 * region rectangle is not includes lines at all
	 * region rectangles is EXACTLY adjusted by showing them on screen see updateRegion function
	 */
	int y;
	auto i = getRealRegion(index);
	switch (i) {
	case CARD_INDEX_EAST:
	case CARD_INDEX_WEST:
		return CRect(
				CPoint(
						(i == CARD_INDEX_WEST ? 0 : m_tableRect.right + 1)
								+ gconfig->m_eastWestCardsMargin, m_tableRect.top + 1),
				CSize(m_tableRect.left - 2 * gconfig->m_eastWestCardsMargin,
						m_tableRect.height() - 1));

	default:	//ABSENT_INDEX || NORTH_INDEX || SOUTH_INDEX
		if (i == CARD_INDEX_ABSENT) {
			y = m_tableRect.bottom + 1 + (m_tableRect.top + 1);
			assert(isBridge());
		}
		else if (i == CARD_INDEX_NORTH) {
			y = 0;
		}
		else {	//SOUTH_INDEX
			y = m_tableRect.bottom + 1;
		}
		//m_windowSize.cx-1 - because of separation line between area & last trick/description
		return CRect(CPoint(0, y), CSize(m_windowSize.cx - 1, m_tableTop));
	}
}

void DrawingArea::mouseLeftButtonDown(GdkEventButton* event) {
	if (think()) {
		return;
	}
	int i;
	CARD_INDEX ci;
	CLICABLE_REGION region = getClickableRegion(event, ci);

	if ((event->state & GDK_CONTROL_MASK) != 0) {
		event->state &= ~GDK_CONTROL_MASK;
		mouseLeftButtonDown(event);
		event->state |= GDK_CONTROL_MASK;
		mouseLeftButtonUp(event);
		return;
	}

	//click on an arrow
	if (region == CLICABLE_REGION_ARROW) {
		hideToolTip();
		hideArrow(true);
		if (ci == getState().m_firstmove) {
			ci = getNextPlayer(ci);
		}
		getState().m_firstmove = ci;
		showArrow(true);
		updateModified();
		return;
	}

	if (region == CLICABLE_REGION_ABSENT) {
		hideToolTip();
		rotate(true, true);
		return;
	}

	//click on player in preferans to switch it
	if (region == CLICABLE_REGION_CAPTION && isEditEnable()) {//region=getClickableRegionIndex() already take account for only preferans game
		hideToolTip();
		CARD_INDEX player = getProblem().m_player;
		if (ci == getProblem().m_player) {	//for current player move it to next
			ci = getNextPlayer(getProblem().m_player);
		}

		setPlayer(ci);
		//after set player update old region and new region
		updateRegion(player);
		updateRegion(getProblem().m_player);
		updateModified();
		return;
	}

	if (!isEditEnable() && m_selectedCard != -1) {
		//do the turn right now
		m_currentId = m_selectedCard;
		recalcRects();
		updateRegion(getState().m_cid[m_selectedCard]);
		m_selectedCard = -1;
		mouseLeftButtonUp(event);
		return;
	}

	//click on card //pull cards & setup new problem
	i = m_selectedCard;

	if (m_selectedCard != -1 && isOuterOrAbsent(i) && m_cardrect[i].in(event) && (//if turns are already done then turn can do only where arrow shows
			isEditEnable() || getState().m_cid[i] > CARD_INDEX_WEST
					|| (!isEditEnable() && getState().m_cid[i] == getNextMove()))) {
		if (m_selectedCard != -1) {
			m_selectedCard = -1;
		}

		m_currentId = i;

		if (isPreferans() && i % 13 > 7) {	//for cards from 2 to 6
			m_currentId = -1;
		}

		CPoint point(event->x, event->y);
		m_addit.x = m_cardrect[i].left - point.x;
		m_addit.y = m_cardrect[i].top - point.y - getActiveCardShift();
		m_currentPoint.x = point.x + m_addit.x;
		m_currentPoint.y = point.y + m_addit.y;
		recalcRects();

		if (getState().m_cid[i] > CARD_INDEX_WEST) {
			updateInsideRegion();
		}
		else {
			updateRegion(getState().m_cid[i]);
			mouseMove(event);
		}
	}

	//click on one of inner cards -> makes some undos
	i=countUndos(event);
	if(i>0){
		undoRedo(true, false, i);
		return;
	}

}

void DrawingArea::mouseMove(GdkEventButton* event) {
	if (think()) {
		return;
	}

	if (m_currentId != -1) {
		CPoint point(event);
		point += m_addit;
		copy(m_surface, m_crEnd);
		CRect r(point, getCardSize());	//update rectangle
		CRect cr(m_currentPoint, getCardSize());
		r.join(cr);

#define M(a,b,c,d) cr.init(a,b,c,d);r.join(cr);copy(m_surface,m_crEnd,cr);
		if (point.x >= m_currentPoint.x) {
			M(m_currentPoint.x, m_currentPoint.y, point.x - m_currentPoint.x,
					getCardSize().cy);
			if (point.y >= m_currentPoint.y) {
				M(point.x, m_currentPoint.y,
						getCardSize().cx - point.x + m_currentPoint.x,
						point.y - m_currentPoint.y);
			}
			else {
				M(point.x, point.y + getCardSize().cy,
						getCardSize().cx - point.x + m_currentPoint.x,
						getCardSize().cy - point.y + m_currentPoint.y);
			}
		}
		else {
			M(point.x + getCardSize().cx, m_currentPoint.y,
					m_currentPoint.x - point.x, getCardSize().cy);
			if (point.y >= m_currentPoint.y) {
				M(m_currentPoint.x, m_currentPoint.y,
						getCardSize().cx - m_currentPoint.x + point.x,
						point.y - m_currentPoint.y);
			}
			else {
				M(m_currentPoint.x, point.y + getCardSize().cy,
						getCardSize().cx - m_currentPoint.x + point.x,
						m_currentPoint.y - point.y);
			}
		}
#undef M
		showCard(m_crEnd, m_currentId, point.x, point.y);
		invalidateRect(r);
		m_currentPoint = point;

		return;
	}

	CARD_INDEX regionIndex;
	CLICABLE_REGION region = getClickableRegion(event, regionIndex);
	//cards
	int i, j, k;

	bool found = false;
	for (k = 0; k < 4; k++) {
		for (j = 0; j < 13; j++) {
			i = gconfig->getCard(3 - k, 12 - j);

			if (m_cardrect[i].in(event)
					&& (isOuter(i)
							|| (isEditEnable() && getState().m_cid[i] == CARD_INDEX_ABSENT))) {
				found = true;
				if ((isEditEnable() || getNextMove() == getState().m_cid[i])
						&& isValidTurn(i)) {
					found = false;
					if (i != m_selectedCard) {
						m_selectedCard = i;
						updateRegion(getState().m_cid[i]);
					}
				}	//if
				goto out;
			}	//if
		}	//for(j)
	}	//for(k)
	out: if (((k == 4 && j == 13) || found) && m_selectedCard != -1) {
		CARD_INDEX ind = getState().m_cid[m_selectedCard];
		m_selectedCard = -1;
		updateRegion(ind);
	}

	//for PREFERANS "Click to rotate by 90 degrees."
	if (region == CLICABLE_REGION_ABSENT) {
		showToolTip(STRING_CLICK_TO_ROTATE_BY_90_DEGREES);
		return;
	}
	if (region == CLICABLE_REGION_CAPTION) {
		const STRING_ID csid[] = {
				STRING_DECLARER,
				STRING_MISERE_PLAYER,
				STRING_CLICK_TO_SWITCH_PLAYER,
				STRING_CLICK_TO_SWITCH_MISERE_PLAYER,
				STRING_WHISTER,
				STRING_CATCHER };
		k = isEditEnable() ? 2 : (regionIndex == getProblem().m_player ? 0 : 4);
		showToolTip(csid[k + isMisere()]);
		return;
	}

	//common tricks
	STRING_ID sid;
	if (region == CLICABLE_REGION_CAPTION_HORIZONTAL_TRICKS) {
		sid =
				isBridge() ?
						STRING_TRICKS_OF_WEST_AND_EAST :
						(isMisere() ? STRING_TRICKS_OF_MISERE_PLAYER : STRING_TRICKS_OF_PLAYER);
		showToolTip(sid);
		return;
	}
	else if (region == CLICABLE_REGION_CAPTION_VERTICAL_TRICKS) {
		sid =
				isBridge() ?
						STRING_TRICKS_OF_NORTH_AND_SOUTH :
						(isMisere() ?
								STRING_TRICKS_OF_CATCHERS : STRING_TRICKS_OF_WHISTERS);
		showToolTip(sid);
		return;
	}

	//arrows
	if (region == CLICABLE_REGION_ARROW) {
		showToolTip(STRING_CLICK_TO_SWITCH_MOVE);
		return;
	}

	if(region==CLICABLE_REGION_UNDO_MOVE){
		showToolTip(STRING_CLICK_TO_UNDO_MOVE);
		return;
	}

	if(region==CLICABLE_REGION_UNDO_MOVES){
		showToolTip(STRING_CLICK_TO_UNDO_MOVES);
		return;
	}

	hideToolTip();

}

void DrawingArea::mouseLeave(GdkEventCrossing* event) {
	if (think()) {
		return;
	}
	if (m_currentId != -1) {
		//Note Coordinates of event is out of window,so just use last mouseMove coordinates which is valid
		GdkEventButton e;
		e.x = m_currentPoint.x;
		e.y = m_currentPoint.y;
		mouseLeftButtonUp(&e);
		//Note mouseLeave left "bad card traces" so call paint
		paint();
	}
	if (m_selectedCard != -1) {	//mouse was over absent card in preferans and now mouse is out
		int select = m_selectedCard;
		m_selectedCard = -1;
		recalcRects();
		updateRegion(getState().m_cid[select]);
	}
}

void DrawingArea::mouseLeftButtonUp(GdkEventButton* event) {
	if (think()) {
		return;
	}

	//no pulling card
	if (m_currentId == -1) {
		return;
	}

	int i, j;
	CARD_INDEX ci;
	int storecurid = m_currentId;
	m_currentId = -1;
	CARD_INDEX from = getState().m_cid[storecurid];

	CPoint point(event);
	point += m_addit;
	invalidateRect(point.x, point.y, getCardSize().cx, getCardSize().cy);
	point -= m_addit;
	if (isOuterOrAbsent(storecurid) && isEditEnable()) {
		for (i = 0; i < SIZEI(OUTER_REGION); i++) {
			ci = OUTER_REGION[i];
			if (getRegionRect(ci).in(event)) {
				getState().m_cid[storecurid] = ci;
				updateModified();
				break;
			}
		}

		if (i < SIZEI(OUTER_REGION)) {//pull not in outer square and not out from inner square
			CARD_INDEX to = getState().m_cid[storecurid];

			if (from == to) {	//card belongs the same player
				if ((event->state & GDK_CONTROL_MASK) != 0) {
					getState().m_cid[storecurid] = m_lastRegion;
					updateModified();
				}
				else {
					auto base = getBasePlayer();

					if (getFindBestState() == BUTTON_STATE_ENABLED
							&& getNextMove() == getState().m_cid[storecurid]) {
						ci = getInner(storecurid);//click on card in valid position and in arrow do a move to inside rectangle
					}
					else if (from == base) {
						auto pci = isBridge() ? PLAYER[0] : getNextPlayer(base);

						//j - counting to understand how many cards need to move to player
						j = countCards(CARD_INDEX_ABSENT) / maxTableCards();

						for (i = 0; i < maxTableCards(); i++, pci = getNextPlayer(pci)) {
							if (countCards(pci) < getMaxHandCards() - j) {
								ci = pci;
								break;
							}
						}
						if (isPreferans() && i == maxTableCards()) {
							ci = CARD_INDEX_ABSENT;
						}
					}
					else {
						ci = base;
					}

					if (isInner(ci)) {
						getProblem().m_currentState = 0;
						saveState();
						updateUndoRedoAll();
					}

					getState().m_cid[storecurid] = ci;
					updateModified();
				}
			}
			else {
				m_lastRegion = getState().m_cid[storecurid];
			}
			recalcRects();
			updateRegion(from);
			if (isInner(storecurid)) {
				enableEdit(false);
				updateInsideRegion();
			}
			else {
				updateRegion(getState().m_cid[storecurid]);
			}

			//potentially select after click.
			//if user release left mouse button and under mouse another card placed then need to select it
			mouseMove(event);
			updateFindBestStateAll();
			return;
		}			//if(i<SIZEI(OUTER_REGION))
	}			//if isOuterOrAbsent(storecurid) && isEditEnable()

	//check for nextmove
	if (getNextMove() != getState().m_cid[storecurid]) {//drag to table not from valid player
		recalcRects();
		updateRegion(from);
		return;
	}

	//if pulling to inner square
	if (isValidTurn(storecurid)) {
		//check that position is correct at first
		if (!isValidDeal()) {			//drag to table but position is incorrect
			recalcRects();
			updateRegion(from);
			return;
		}
		enableEdit(false);

		makeMove(storecurid);
		updateModified();
		updateUndoRedoAll();
		updateFindBestStateAll();
		return;
	}
}

void DrawingArea::showArrow(bool paint) {
	CARD_INDEX next = getNextMove();
	CRect rect = getArrowRect(next);
	copyFromPixbuf(getProblemSelector().m_arrow[next - 1], m_cr, rect);
	if (paint) {
		invalidateRect(rect);
	}
}

CLICABLE_REGION DrawingArea::getClickableRegion(GdkEventButton* event,
		CARD_INDEX& region) {

	int i;

	if (isPreferans()) {
		if (isEditEnable() && pointInCaption(CARD_INDEX_ABSENT, event)) {
			return CLICABLE_REGION_ABSENT;
		}
		for (auto p : PLAYER) {
			if (p == getAbsent()) {
				continue;
			}
			if (pointInCaption(p, event)) {
				//cards can be over North/South text so it's not region caption
				for (i = 0; i < 52; i++) {
					if (getState().m_cid[i] == CARD_INDEX_INVALID) {
						continue;
					}
					if (m_cardrect[i].in(event)) {
						break;
					}
				}
				if (i == 52) {
					region = p;
					return CLICABLE_REGION_CAPTION;
				}
			}
		}
	}

	if (isEditEnable()) {
		for (auto p : PLAYER) {
			if (isPreferans() && p == getAbsent()) {
				continue;
			}
			if (getArrowRect(p).in(event)) {
				region = p;			//for leftButtonDown
				return CLICABLE_REGION_ARROW;
			}
		}
	}

	if (isEditEnable() && gconfig->m_showCommonTricks) {
		for (i = 0; i < SIZEI(m_totalTricksRect); i++) {
			if (m_totalTricksRect[i].in(event)) {
				return
						i == 0 ?
								CLICABLE_REGION_CAPTION_HORIZONTAL_TRICKS :
								CLICABLE_REGION_CAPTION_VERTICAL_TRICKS;
			}
		}
	}

	if(!think()){
		i=countUndos(event);
		if(i>0){
			return i==1 ? CLICABLE_REGION_UNDO_MOVE : CLICABLE_REGION_UNDO_MOVES;
		}
	}
	return CLICABLE_REGION_NOT_FOUND;
}

void DrawingArea::redrawState() {
	recalcRects();
	updateAllRegions();
	updateLastTrick();

	updateUndoRedoAll();
	//if was empty position no cards left, so find best was disabled, after undo find best is enabled
	updateFindBestStateAll();
	updateModified();
}

void DrawingArea::undoRedo(bool undo, bool full, int count) {
	//use ignore think versions because this function calls when computer find best move which is next
	assert(
			(undo && isUndoEnableIgnoreThink())
					|| (!undo && isRedoEnableIgnoreThink()));

	if (full) {
		getProblem().m_currentState = undo ? 0 : getProblem().m_maxState;
	}
	else {
		getProblem().m_currentState += undo ? -count : count;
	}
	redrawState();
}

void DrawingArea::showCard(cairo_t * ct, int index, int x, int y) {
	if (index == m_selectedCard) {
		y -= getActiveCardShift();
	}

	if (x + getCardSize().cx >= m_windowSize.cx - 1) {
		if (x < m_windowSize.cx - 1) {
			//prevent right vertical line between DrawingArea and LastTrick/Description from damage
			//it occurs when user do rotate of empty problem
			copyFromDeck(ct, x, y, m_windowSize.cx - 1 - x, getCardSize().cy, index,
					0, 0);
		}
	}
	else {
		copyFromDeck(ct, x, y, getCardSize().cx, getCardSize().cy, index, 0, 0);
		showEstimation(ct, index, x, y);
	}
}

void DrawingArea::init() {
	int i;
	//countSize();call from ProblemSelector().init()

	createNew(m_cr, m_surface, m_windowSize);
	createNew(m_crEnd, m_surfaceEnd, m_windowSize);

	//TODO not set any time only if font changed
	setFont(m_cr);
	cairo_set_font_size(m_cr, getFontHeight());

	const int dx = getIndentInsideSuit() - 1;			//-1 because of card border
	for (i = 2;
			i < 2 * dx
					&& getTextExtents(TextWithAttributes::createEstimateText("13", i)).cx
							<= dx; i++)
		;
	m_estimateFontHeight = i - 1;

	recalcRects();
}

void DrawingArea::countSize(int y) {
	int i,j,w;
	std::string s;
	//println("%d",y)
	int height=getMaxCardSize().cy;
	int as=getArrowSize();
	const int tableSize =countTableSize(height,as,y);
	/* in case of north cards in preferans is visible m_tableTop=m_tableRect.top
	 * otherwise m_tableTop = height of north/south regions
	 */
	m_tableTop = countTableTop(height);

	//140dpi for smallest decks
	if(!m_cr){//for getTextExtents
		init();
	}

	w = (maxCardsInSuit() - 1) * getIndentInsideSuit() + getMaxCardSize().cx;

	for(i=0;i<2;i++){
		s=format("%s %s 13",getString(i==0?STRING_EAST:STRING_WEST),getString(STRING_TRICKS));
		j=getTextExtents(s).cx;
		w=std::max(w,j);
	}


	m_tableRect = CRect(
			CPoint(2 * gconfig->m_eastWestCardsMargin + w,
					northInvisible() ? 0 : m_tableTop),
			CSize(tableSize, tableSize));//includes top/left lines and not includes bottom/right ones
	m_windowSize = CSize(2 * (m_tableRect.left + 1) + tableSize,
			countAreaHeight(height,as, y));

	//Note [for m_windowSize] -1 for sizey because we don't need include last right/lower line

	i = m_totalTricksImageSize0.cx + m_totalTricksImageSize0.cy;
	m_totalTricksRect[0] = CRect(
			CPoint(m_tableRect.right - i,
					m_tableRect.bottom - m_totalTricksImageSize0.cy),
			m_totalTricksImageSize0);
	m_totalTricksRect[1] = CRect(
			CPoint(m_tableRect.right - m_totalTricksImageSize0.cy,
					m_tableRect.bottom - i),
			CSize(m_totalTricksImageSize0.cy, m_totalTricksImageSize0.cx));

	//getSize() uses isEditEnable() && isBridge,
	i = getSize().cy - getAreaMaxHeight();
	//we skip a condition part of code below if(i%2==1){i++;} so condition is i>1 not i>0 prevents infinite cycle
	if (i > 1) {
//		println("(%d) %d %d",getInnerCardMargin().cy - i / 2,getInnerCardMargin().cy, i)
		/* for countSize( getInnerCardMargin().cy ) got getSize().cy
		 * for countSize( getInnerCardMargin().cy -k) got getSize().cy-2*k
		 * getSize().cy-2*( getInnerCardMargin().cy -k)<=m_maxHeight
		 * 2*(getInnerCardMargin().cy-k) >= (getSize().cy-m_maxHeight)=i
		 */

		/*next condition is true, but more nice view make window height a little bit more than working area height
		 if(i%2==1){
		 i++;
		 }*/

		/* 2*( getInnerCardMargin().cy -k) >= i
		 * ( getInnerCardMargin().cy -k)>=i/2
		 * k<=getInnerCardMargin().cy-i/2
		 */
		countSize(getInnerCardMargin().cy - i / 2);
	}

}

void DrawingArea::recalcRects() {
	int i = 0, j, k;
	CARD_INDEX ci, r, rid;
	const int AY24 = (m_tableRect.height()
			- getTextExtents(TextWithAttributes::createUnderlinedText("Qy")).cy
			- getActiveCardShift() - getCardSize().cy) / 3;	//add y for east&west

	CPoint begin;
	for (auto id : OUTER_REGION) {
		if (isPreferans() && id == getAbsent()) {
			continue;
		}
		ci = isPreferans() && id == CARD_INDEX_ABSENT ? getAbsent() : id;

		//center cards of east&west horizontally
		const int STARTX24 = (ci == CARD_INDEX_EAST ? m_tableRect.right + 1 : 0)
				+ (m_tableRect.left - ((maxCardsInSuit() - 1)) * getIndentInsideSuit()
						- getCardSize().cx) / 2;

		r = isBridge() ? ci : getRealRegion(ci);

		if (eastOrWest(r)) {
			begin = CPoint(STARTX24,
					m_tableRect.bottom - getCardSize().cy - 3 * AY24);
		}
		else {
			begin.x = 0;

			//center (card&selected card) vertically cards for north,south,absent regions
			begin.y = (m_tableTop - getCardSize().cy + getActiveCardShift()) / 2;
			if (ci == CARD_INDEX_ABSENT || ci == CARD_INDEX_SOUTH) {
				begin.y += m_tableRect.bottom + 1;
				if (ci == CARD_INDEX_ABSENT && isBridge()) {
					begin.y += m_tableRect.top + 1;
				}
			}

		}

		bool needAdd = true;
		rid = isBridge() ? id : getRealRegion(id);

		for (k = 0; k < 4; ++k) {
			for (j = 0; j < 13; j++) {
				i = gconfig->getCard(k, j);
				if (getState().m_cid[i] == id) {
					m_cardrect[i] = CRect(begin, getCardSize());

					if (i != m_currentId) {
						needAdd = false;
						begin.x += getIndentInsideSuit();
					}
				}
			}			//for(j)

			if (eastOrWest(rid)) {
				begin.y += AY24;
				begin.x = STARTX24;
			}
			else if ((isBridge() && northOrSouth(rid))
					|| (isPreferans() && id != CARD_INDEX_ABSENT)) {
				if (!needAdd && begin.x != 0) {
					needAdd = true;
					begin.x += (gconfig->m_indentBetweenSuits - getIndentInsideSuit())
							+ getCardSize().cx;
				}
			}
		}			//for(k)

	}			//for(id)

	//center north&south player cards relatively m_tableRect
	for (j = 1; j <= 3; j += 2) {
		k = 0;
		for (i = 0; i < 52; ++i) {
			if (getState().m_cid[i] == j || getState().m_cid[i] == j + 10) {
				if (m_cardrect[i].left > k) {
					k = m_cardrect[i].left;
				}
			}
		}
		k += getCardSize().cx;

		k = m_tableRect.centerPoint().x - k / 2;
		for (i = 0; i < 52; ++i)
			if (getState().m_cid[i] == j || getState().m_cid[i] == j + 10) {
				m_cardrect[i].left += k;
				m_cardrect[i].right += k;
			}
	}

	for (i = 0; i < 52; ++i) {
		if (isInner(getState().m_cid[i])) {
			m_cardrect[i] = getInsideRect(getOuter(i));
		}
	}
}

CRect DrawingArea::getArrowRect(CARD_INDEX index) {
	//Note this function changed in version 4.0 works fine
	int a = getArrowSize();
	CPoint p = m_tableRect.centerPoint();

#define M(A,B,C) p.A=(m_tableRect.C+getInsideRect(index).C-a)/2;p.B-=a/2;
//+1 because arrow starts from p.A, so don't need to intersect line (tested on biggest deck & small arrow
#define ML(A,B,C) M(A,B,C);if(p.A<m_tableRect.C+1){p.A=m_tableRect.C+getInnerCardMargin().c##A;}
#define MH(A,B,C) M(A,B,C);if(p.A+a>m_tableRect.C){p.A=m_tableRect.C-a-getInnerCardMargin().c##A;}

	/* if(p.A<m_tableRect.C){p.A=m_tableRect.C+getInnerCardMargin().c##A;} &&
	 * if(p.A+a>m_tableRect.C){p.A=m_tableRect.C-a-getInnerCardMargin().c##A;}
	 * conditions prevents arrow out for small monitors & big cards because tableRect can be very small
	 */

	switch (index) {
	case CARD_INDEX_NORTH:
		ML(y, x, top)
		break;

	case CARD_INDEX_EAST:
		MH(x, y, right)
		break;

	case CARD_INDEX_SOUTH:
		MH(y, x, bottom)
		break;

	default:
		ML(x, y, left)
	}
	return CRect(p, CSize(a, a));

#undef M
#undef ML
#undef MH

}

void DrawingArea::setDeal(bool random) {
	if (isMisere()) {
		getToolbar().setMisere();
	}
	else {
		getToolbar().setTrump(getProblem().m_trump);
		getToolbar().setContract(getProblem().m_contract);
	}

	m_currentId = -1;

	updateFindBestStateAll();			//make enable
	enableEdit(!random);
	updateUndoRedoAll();	//need update undo/redo state
	updateModified();
	resize();
	recalcRects();
	redraw();
}

CSize DrawingArea::getSize() const {
	CSize size(m_windowSize);
	int c;
	if (isBridge()) {
		c = !isEditEnable();
	}
	else {
		if (northOrSouth(getAbsent())) {
			c = 1 + !isEditEnable();
		}
		else {
			c = 1;
		}
	}
	size.cy -= c * m_tableTop;
	if (c > 0) {
		size.cy--;	//sub 1 only one time to show lower line of table
	}
	return size;
}

void DrawingArea::invalidateRect(gint x, gint y, gint width, gint height) {
	gtk_widget_queue_draw_area(getWidget(), x, y, width, height);
}

CSize DrawingArea::getMaxCardSize() {
	return
			gconfig->m_resizeOnDeckChanged ? getCardSize() : MAX_CARD_SIZE;
}

void DrawingArea::hideArrow(bool paint) {
	CRect r = getArrowRect(getNextMove());
	copyFromBackground(r);
	if (paint) {
		invalidateRect(r);
	}
}

void DrawingArea::findBest(const Problem* problem) {
	int i;
	hideToolTip();
	if (problem == NULL) {
		if (think()) {
			stopCountThread();
			endSolveThread();
			for(i=0;i<52;i++){
				if (getState().m_estimate[i] == UNKNOWN_ESTIMATE) {
					setShowEstimation(i, ESTIMATE_CLEAR, false);
				}
			}
			return;
		}

		//deselect card at first
		if (m_currentId != -1) {
			i = m_currentId;
			m_currentId = -1;
			recalcRects();
			updateRegion(getState().m_cid[i]);
		}
		if (m_selectedCard != -1) {
			i = m_selectedCard;
			m_selectedCard = -1;
			recalcRects();
			updateRegion(getState().m_cid[i]);
		}

		m_timer = g_timeout_add_seconds(1, timer_handler, (gpointer)0);
		m_startTime = clock();
		enableEdit(false);
		updateThinkAll();
	}

	m_solveThread=g_thread_new("", solve_thread, gpointer(problem));
}

void DrawingArea::edit() {
	int i;
	//sometimes we cann't do undo because problem was loaded and has no history
	for (i = 0; i < 52; ++i) {
		if (isInner(i)) {
			getState().m_cid[i] = getOuter(getState().m_cid[i]);
		}
	}

	getState().clearEstimates();//anyway do redraw() so don't need use setShowEstimation
	getState().clearTricks();
	getState().m_bestLine.clear();

	i = getProblem().m_currentState;
	getProblem().m_maxState = getProblem().m_currentState = 0;
	getLastTrick().newGame();			//after m_currentState=0 no last trick
	getState() = getProblem().m_states[i];

	recalcRects();
	enableEdit(true);
	updateUndoRedoAll();
	getMenu().setItemAttributes(MENU_SOLVE_ALL_FOE);
	updateModified();

}

void DrawingArea::saveHtml(std::string filepath, bool images,
		const ProblemVector& p) {
	std::string s;
	int i;
	FILE*f = open(filepath, "wb+");
	if (f == NULL) {
		message(MESSAGE_ICON_ERROR, STRING_ERROR_COULD_NOT_OPEN_FILE);
		return;
	}
	if (gconfig->m_htmlShowDialog) {
		HtmlOptionsDialog dialog;
	}

	bool solveHtml = gconfig->m_htmlStoreBestMove
			|| gconfig->m_htmlStoreNumberOfTricks;

	fprintf(f, PROBLEM_HTML_BEGIN);
	i=1;
	for (auto& pr : p.m_problems) {
		if (solveHtml && getFindBestState() == BUTTON_STATE_ENABLED) {//if getFindBestState()!=BUTTON_STATE_ENABLED then we will wait condition infinitely
			m_functionWait = true;
			findBest(&pr);
			//wait for finish
			waitForFunction();
		}
		s = pr.getHTMLContent(i, m_bestHtml, m_northSouthTricksHtml,
				m_eastWestTricksHtml, false,p.m_problems.size());
		s = Problem::postproceedHTML(s, images);
		fprintf(f, "%s", s.c_str());
		i++;
	}
	fprintf(f, "%s", PROBLEM_HTML_END);
	fclose(f);

	if (gconfig->m_htmlPreview) {
		openURL(filepath);		//don't call openURL("file:///"+filepath) - it's error
	}

}


void DrawingArea::setShowEstimation(int index, int estimation, bool thread) {
	assert(index >= 0 && index < 52);
	if (getState().m_estimate[index] == estimation) {
		return;
	}

	getState().m_estimate[index] = estimation;
	if (index != m_currentId) {			//for animation fly
		if (thread) {
			gdk_threads_add_idle(show_estimation_thread, GP(index));
		}
		else {
			showEstimation(index);
		}
	}
}

void DrawingArea::showEstimation(cairo_t* ct, int index, int x, int y) {
	char s[4];
	auto e = getEstimateType();
	int estimation =
			e == ESTIMATE_NONE ? ESTIMATE_CLEAR : getState().m_estimate[index];

	if (estimation == UNKNOWN_ESTIMATE) {
		sprintf(s, "?");
	}
	else if (estimation != ESTIMATE_CLEAR) {
		if ((e == ESTIMATE_ALL_LOCAL || e == ESTIMATE_ALL_TOTAL)
				&& getProblem().m_currentState > 0) {
			const CARD_INDEX player =
					isOuter(getState().m_cid[index]) ?
							getState().m_cid[index] : getOuter(getState().m_cid[index]);
			estimation += getPreviousStateTricks(player);
			if (isBridge() || (isPreferans() && player != getProblem().m_player)) {
				estimation += getPreviousStateTricks(getPartner(player));
			}
		}
		sprintf(s, "%d", estimation);

	}

	const int width = getIndentInsideSuit() - 1;			//-1 because of card border
	//need only text height,even for estimation==ESTIMATE_CLEAR when 's' isn't set, so always use "13" as string
	const int height = getTextExtents(
			TextWithAttributes::createEstimateText("13", m_estimateFontHeight)).cy;

	const int dx = 1;	//+1 because of card border
	const int dy = gconfig->getEstimationIndent();
	x += dx;
	y += dy;

	if (estimation == ESTIMATE_CLEAR) {
		copyFromDeck(ct, x, y, width, height, index, dx, dy);	//copy from cards picture
	}
	else {
		gdk_cairo_set_source_rgba(ct, &ESTIMATE_COLOR);
		cairo_set_line_width(ct, 0);
		cairo_rectangle(ct, x, y, width, height);
		cairo_stroke_preserve(ct);
		cairo_fill(ct);

		drawTextToCairo(ct,
				TextWithAttributes::createEstimateText(s, m_estimateFontHeight),
				CRect(CPoint(x, y), CSize(width, height)), true, true);
	}

	invalidateRect(x, y, width, height);	//have to do invalidate rectangle
}

void DrawingArea::updateEstimationType() {
	int i;
	CARD_INDEX cid;
	for (i = 0; i < 52; i++) {
		cid = getState().m_cid[i];
		//skip absent & 2-7 cards for preferans
		if (cid != CARD_INDEX_INVALID && cid != CARD_INDEX_ABSENT) {
			showEstimation(i);
		}
	}
}

//Note cann't call updateModified(); from makeMove because this function is used when loading some problem and do sequence of turns
void DrawingArea::makeMove(int index,bool estimateBeforeBest) {
	int i;
	CARD_INDEX ci;

	const int omax = getProblem().m_maxState;

	/* can call from thread so use isRedoEnableIgnoreThink() instead of isRedoEnable()
	 * if make move from next state, store m_maxState and restore it after saveState()
	 * so it'll be like redo
	 */
	const bool redo = isRedoEnableIgnoreThink()
			&& index == getNextState().getDifference(getState());

	saveState();
	/* best line copied, after saveState() adjust best line
	 * need to call adjustBestLinethis at least when user click on grid in LastTrick
	 * so do not remove adjusting
	 */
	getState().adjustBestLine(index);
	getLastTrick().drawBestLine();

	if (redo) {
		getProblem().m_maxState = omax;
	}

	if (isTableFull()) {
		getState().m_firstmove = getNextMove();
		getState().clearInner();
	}

	/* in case !estimateBeforeBest clear in solveThread() function
	 */
	if(!estimateBeforeBest){
		for (i = 0; i < 52; i++) {	//clear all estimations
			setShowEstimation(i, ESTIMATE_CLEAR, false);
		}
	}

	ci = getState().m_cid[index];
	getState().m_cid[index] = getInner(index);

	if (isTableFull()) {	//update number of tricks immediately after move
		CARD_INDEX in = getNextMove();
		getState().incrementTricks(in);
		updateTricks(in);

		//redraw last trick
		updateLastTrick();
	}
	recalcRects();
	if (!think()) {
		updateInsideRegion();
	}
	updateRegion(ci);
	getMenu().setItemAttributes(MENU_SOLVE_ALL_FOE);
	finishWaitFunction();
}

void DrawingArea::startWaitFunction(GSourceFunc function, gpointer data) {
	m_functionWait = true;
	gdk_threads_add_idle(function, data);
}

void DrawingArea::finishWaitFunction() {
	g_mutex_lock(&m_mutex);
	g_cond_signal(&m_condition);
	m_functionWait = false;
	g_mutex_unlock(&m_mutex);
}

void DrawingArea::waitForFunction() {
	g_mutex_lock(&m_mutex);
	while (m_functionWait) {
		g_cond_wait(&m_condition, &m_mutex);
	}
	g_mutex_unlock(&m_mutex);
}

CARD_INDEX DrawingArea::getPartner(CARD_INDEX index) {
	int i;
	assert(isOuter(index));
	if (isBridge()) {
		return getBridgePartner(index);
	}
	else {
		assert(index != getProblem().m_player);
		for (i = 0; i < PREFERANS_PLAYER_SIZE; i++) {
			if (getPreferansPlayer()[i] != getProblem().m_player
					&& getPreferansPlayer()[i] != index) {
				break;
			}
		}
		assert(i < PREFERANS_PLAYER_SIZE);
		return getPreferansPlayer()[i];
	}
}

bool DrawingArea::needPlayNextTurn() const {
	//check that next move is sequence
	if (isEmptyDeal()) {
		return false;
	}

	int leading = getLeadingSuit();
	int suit;
	if (leading == NT) {	//first turn
		return nextPlayerHasOnlyOneSuit(suit) && nextPlayerIsSequenceSuit(suit);
	}

	if (nextPlayerHasLeadingSuit()) {	//turn2-4 & hasLeadingSuit
		return nextPlayerIsSequenceSuit(leading);
	}

	//turn2-4 & player has no leading suit
	if (nextPlayerHasOnlyOneSuit(suit)) {
		return nextPlayerIsSequenceSuit(suit);
	}
	else {
		if (isBridge()) {
			return false;
		}

		//preferans only
		if (isMisere() || getTrump() == NT) {
			return false;
		}

		return nextPlayerHasTrumpSuit() && nextPlayerIsSequenceSuit(getTrump());
	}

}

bool DrawingArea::nextPlayerHasOnlyOneSuit(int& suit) const {
	suit = -1;
	for (int i = 0; i < NT; i++) {
		if (getState().hasSuit(getNextMove(), i)) {
			if (suit == -1) {
				suit = i;
			}
			else {
				return false;
			}
		}
	}
	assert(suit != -1);
	return true;
}

bool DrawingArea::nextPlayerIsSequenceSuit(int suit) const {
	//return cards which are already on table for correct getting of sequences
	int i;

	CARD_INDEX cid[52];
	for (i = 0; i < 52; i++) {
		cid[i] = isInner(i) ? getOuter(i) : getState().m_cid[i];
	}

	CARD_INDEX taker = getProblem().getTaker(countInnerCards(),
			getProblem().m_currentState);

	for (i = suit * 13; i < (suit + 1) * 13; i++) {
		if (cid[i] == getNextMove()) {
			break;
		}
	}
	assert(i < (suit + 1) * 13);

	bool otherFound = false;
	for (; i < (suit + 1) * 13; i++) {
		if (cid[i] == CARD_INDEX_INVALID || cid[i] == CARD_INDEX_ABSENT) {
			continue;
		}

		if (cid[i] == getNextMove()) {
			if (otherFound) {
				return false;
			}
		}
		else {
			//check inner card which is not taker
			if (isInner(i)
					&& (countInnerCards() == maxTableCards() || taker != getOuter(i))) {
				continue;
			}
			otherFound = true;
		}
	}	//for(i)
	return true;
}

void DrawingArea::updateProblem() {
	getLastTrick().newGame();

	if (isMisere()) {
		getToolbar().setMisere();
	}
	else {
		getToolbar().setTrump(getProblem().m_trump);
		getToolbar().setContract(getProblem().m_contract);
	}

	DEAL_STATE st = getDealState(false);
	enableEdit(st == DEAL_STATE_NEW || st == DEAL_STATE_ERROR);

	redrawState();
}

void DrawingArea::solveThread(Problem* problem) {
	int i,j,k,se[52];
	Problem& p = problem == NULL ? getProblem() : *problem;
	Problem old(p);
	auto e = getEstimateType();

#ifdef BRIDGE_BEST_MULTITHREAD
	const bool estimateBeforeBest= p.isBridge() && (getEstimateType() == ESTIMATE_ALL_LOCAL || getEstimateType() == ESTIMATE_ALL_TOTAL);
#else
	const bool estimateBeforeBest=false;
#endif

#ifndef FINAL_RELEASE
	const bool showTimes=1;
	clock_t begin=clock();
	double solveTime=0;
#endif

	if (p.isBridge()) {

//		std::string s;
//		char b[25];
//		for(i=0;i<52;i++){
//			sprintf(b,"%d",int(cid[i]));
//			s+=b;
//				if((i+1)%13==0){
//					s+=" ";
//				}
//		}
//		println("%s",s.c_str() )
//		println("%d %d",p.m_trump, isTrumpChanged() )

		if(estimateBeforeBest){
			//setEstimateFunction should write estimates to next state which is not created
			for (i = 0; i < 52; i++) {	//clear all estimations
				se[i]=getState().m_estimate[i];
				setShowEstimation(i, ESTIMATE_CLEAR, true);
			}

			bridge.estimateAll(old,e, setEstimateFunction,true, isTrumpChanged());

			//initialize i to prevent warning
			i=k=-1;
			j=0;
			for(int q:getState().m_estimate){
				if(q!=ESTIMATE_CLEAR && q>k){
					i=j;
					k=q;
				}
				j++;
			}
			//i - is the best turn
			//bridge.m_best=i;
		}
		else{
			bridge.solve(p, isTrumpChanged());
			i = bridge.m_best;
		}
	}
	else {
		preferans.solve(p, isTrumpChanged());
		i = preferans.m_best;
	}
#ifndef FINAL_RELEASE
	if(showTimes && p.isBridge()){
		if(estimateBeforeBest){
			println("solve+estimateAll %.3lf",double(clock()-begin)/CLOCKS_PER_SEC);
		}
		else{
			solveTime=bridge.m_time;
			println("solve %.3lf",bridge.m_time);
		}
	}
#endif

	setTrumpChanged(problem != NULL);

	if (problem != NULL) {
		m_bestHtml = i;
		m_northSouthTricksHtml = bridge.m_ns;
		m_eastWestTricksHtml = bridge.m_ew;
		//make signal that count is finished
		finishWaitFunction();
		return;
	}

	m_currentPoint = m_cardrect[i].topLeft();
	startWaitFunction(make_move_thread, GP(i | (estimateBeforeBest<<8)));
	waitForFunction();
	if(needStopThread()){
		return;
	}

	if (gconfig->m_animation) {	//animation fly
		startWaitFunction(timer_animation_handler, GP(i));	//do step 0 immediately. First step also runs animation timer
	}
	else {
		gdk_threads_add_idle(update_inside_region_thread, NULL);//gdk call use main thread
	}

	//if search was for storing in html file then don't need to estimate all turns
	if (e != ESTIMATE_NONE) {
#ifndef FINAL_RELEASE
		begin=clock();
#endif
		if (p.isBridge()) {
			if(!estimateBeforeBest){
				bridge.estimateAll(old, e, setEstimateFunction,false,false);
			}
		}
		else {
			preferans.estimateAll(old, e, setEstimateFunction);
		}

#ifndef FINAL_RELEASE
		if(showTimes && !estimateBeforeBest){
			double v=double(clock()-begin)/CLOCKS_PER_SEC;
			println("estimateAll %.3lf, total %.3lf",v,v+solveTime);
		}
#endif

		if (p.isBridge()) {
			if (estimateBeforeBest) {
				for (i = 0; i < 52; i++) {
					if (getPreviousState().m_estimate[i] != ESTIMATE_CLEAR) {
						getState().m_estimate[i] = getPreviousState().m_estimate[i];
					}
					getPreviousState().m_estimate[i] = se[i];
				}
			}
		}
	}

	/* function bestLine works very fast so don't need more optimizations
	 * if move calling bestLine before estimate all, need to store some
	 * variables in class
	 */
//#ifndef FINAL_RELEASE
//		begin=clock();
//#endif
	if (isBridge()) {
		bridge.bestLine(getState().m_cid, old.getFirst());
	}
	else {
		preferans.bestLine(getState().m_cid, old.getFirst(), p.m_player, p.m_misere,
				p.m_preferansPlayer);
	}

//#ifndef FINAL_RELEASE
//	if(showTimes){
//		println("bestline %.3lf", double(clock()-begin)/CLOCKS_PER_SEC);
//	}
//#endif
	getState().setBestLine(isBridge() ? bridge.m_bestLine : preferans.m_bestLine);
	gdk_threads_add_idle(draw_bestline_thread, 0);

	if (gconfig->m_animation) {
		waitForFunction();
	}

	if(needStopThread()){
		return;
	}

	if (gconfig->m_autoPlaySequence && needPlayNextTurn()) {
		solveThread(NULL);
	}
	else {
		gdk_threads_add_idle(end_solve_thread, NULL);
	}

}

void DrawingArea::endSolveThread() {
	stopTimer(m_timer);

	updateModified();

	timer();	//clear clock after think()=false
	updateThinkAll();	//after think()=false
}

void DrawingArea::timer() {
	//m_timer==0 -> remove clock, otherwise draw close

	int seconds = int(double(clock() - m_startTime) / CLOCKS_PER_SEC);

	//draw only if time>=1 second
	if (think() && seconds < 1) {
		return;
	}

	const double radius = gdk_pixbuf_get_width(m_totalTricksImage[1]);

	CSize sz = getTextExtents(TextWithAttributes("000:00"));
	CRect r(CPoint(m_tableRect.left + radius, m_totalTricksRect[0].top),
			CSize(sz.cx, m_totalTricksRect[0].height()-1));
	copyFromBackground(r);

	if (think()) {
		drawText(
				TextWithAttributes(format("%02d:%02d", seconds / 60, seconds % 60)), r,
				false, true);
	}

	invalidateRect(r);
}

gboolean DrawingArea::animationStep(int index) {
	if (index != -1) {	//means first step
		m_animationStep = 0;
		m_currentId = index;
		m_animationTimer=g_timeout_add(ANIMATION_STEP_TIME, timer_animation_handler, gpointer(-1));
	}

	animationDraw(false);

	if (m_animationStep++ == ANIMATION_STEPS) {
		m_animationTimer=0;
		endAnimation(false);
		return G_SOURCE_REMOVE;
	}
	else if (index != -1) {
		return G_SOURCE_REMOVE;
	}
	else {
		return G_SOURCE_CONTINUE;
	}
}

void DrawingArea::animationDraw(bool stop){
	CRect r(CPoint(0, 0), getSize());	//some of estimations could change so invalidate full rectangle
	copy(m_surface, m_crEnd);

	if (!stop) {
		CPoint p = m_cardrect[m_currentId].topLeft();
		showCard(m_crEnd, m_currentId,
				m_currentPoint.x
						+ (p.x - m_currentPoint.x) * m_animationStep / ANIMATION_STEPS,
				m_currentPoint.y
						+ (p.y - m_currentPoint.y) * m_animationStep / ANIMATION_STEPS);
	}
	invalidateRect(r);
}

void DrawingArea::endAnimation(bool stop){
	if(stop){
		animationDraw(true);
	}
	CARD_INDEX id = getOuter(m_currentId);
	copy(m_surfaceEnd, m_cr);	//currentId changed
	m_currentId = -1;
	recalcRects();	//m_currentId changed
	updateInsideRegion();
	updateRegion(id);

	finishWaitFunction();
}

int DrawingArea::getTricks(CARD_INDEX player) const {
	int index = INDEX_OF(PLAYER, player);	//always PLAYER not PREFERANS_PLAYER
	assert(index != -1);
	if (index != -1) {
		return getState().m_tricks[index];
	}
	else {
		return 0;
	}
}

const gchar* DrawingArea::getPlayerString() const {
	return ::getPlayerString(getProblem().m_player);
}

int DrawingArea::countInnerCards() const {
	return getState().countInnerCards(m_currentId);
}

void DrawingArea::updateLastTrick() {
	getLastTrick().redraw();
}

void DrawingArea::updateDeckSelection() {
	initResizeRedraw();
}

void DrawingArea::rotate(bool clockwise, bool likeBridge) {
	if (think()) {
		return;
	}
	getProblem().rotate(clockwise, likeBridge);
	gconfig->m_absent = getProblem().m_absent;	//update only by manual rotation
	bool b = isPreferans() && likeBridge && !isEditEnable();
	if (b) {
		countSize();
	}
	recalcRects();
	updateAllRegions();
	updateLastTrick();
	updateModified();
	if (b) {
		//window size is changed
		gframe->updateEdit();
	}
}

void DrawingArea::solveAllDeclarersPreferansThread() {
	int i, v, first;
	Preferans p;
	int trump;
	bool misere;
	CARD_INDEX c[52], player;
	CARD_INDEX ci;

	g_mutex_lock(&m_solveAllMutex);
	for (i = 0; i < 52; i++) {
		ci = getState().m_cid[i];
		if (isInner(ci)) {
			ci = getOuter(ci);
		}
		c[i] = ci;
	}
	const int MAXV = getMaxSolveAllThreads();
	g_mutex_unlock(&m_solveAllMutex);

	while (1) {
		g_mutex_lock(&m_solveAllMutex);
		v = m_solveAllNumber++;
		g_mutex_unlock(&m_solveAllMutex);

		if (v >= MAXV) {
			return;
		}

		parsePreferansSolveAllDeclarersParameters(v, trump, misere, player);
		for (first = 0; first < 3; first++) {
			p.solveEstimateOnly(c, trump, getPreferansPlayer(first), player, misere,
					getPreferansPlayer(), first == 0);
			m_solveAllDeclarersPreferansResult[v * 3 + first] = p.m_playerTricks;
		}

	}
}

#ifdef FINAL_RELEASE
	void DrawingArea::solveAllDeclarersBridgeThread() {
#else
	void DrawingArea::solveAllDeclarersBridgeThread(int thread) {
#endif

	int trump;
	int i,v;
	CARD_INDEX c[52],ci;
	Bridge b;

	const int MAXV = getMaxSolveAllThreads();
	for (i = 0; i < 52; i++) {
		ci = getState().m_cid[i];	//use current state
		assert(ci != CARD_INDEX_INVALID);
		if (isInner(ci)) {
			ci = getOuter(ci);
		}
		c[i]=ci;
	}

	while (1) {
		g_mutex_lock(&m_solveAllMutex);
		v = m_solveAllNumber++;
		g_mutex_unlock(&m_solveAllMutex);

		if (v >= MAXV) {
			return;
		}

		trump = v == 0 ? NT : v - 1;
#ifndef FINAL_RELEASE
		auto begin = clock();
#endif

		for (i = 0; i < SIZEI(PLAYER); i++) {
			b.solveEstimateOnly(c, trump, PLAYER[i], i == 0);
			m_solveAllDeclarersBridgeResult[trump][i] = i%2 ? b.m_ns : b.m_ew;
		}
		gdk_threads_add_idle(solve_all_bridge_set_labels, GP(trump));

#ifndef FINAL_RELEASE
		println("t%d trump%d %.3lf", thread, trump,
				double(clock()-begin)/CLOCKS_PER_SEC);
#endif

	}
}

int DrawingArea::getFoeSteps(Permutations const& p){
	/* later using i%(p.number()/steps)
	 * so steps>=1 & p.number()/steps>=1 ~ (p.number()>=steps)
	 *
	 */
	int steps;
	if(isBridge()){
		//is case of bridge speed very differs from problem to problem
		const int deals=40;
		steps=p.number()/deals;
	}
	else{
		steps=PREFERANS_SOLVE_ALL_FOE_STEPS;
	}

	if(steps==0 || p.number()/steps==0){
		steps=p.number();
	}

	return steps;
}

void DrawingArea::solveAllFoeThread(int index) {
	int i, j,k;
	const int sz=getMaxHandCards()+1;
	int* result=new int[sz];
	bool trumpChanged = true;
	SolveAll& sa = m_solveAllPtr[index];
	CARD_INDEX*ptr = sa.cid;
	int*o = sa.o;
	clock_t t,lastUpdate=clock();
	Permutations p(sa.k, sa.n, COMBINATION);
	const int steps=getFoeSteps(p);
	bool ur;
	const bool b=isBridge();

	g_mutex_lock(&m_solveAllMutex);
	sa.begin=clock();
	g_mutex_unlock(&m_solveAllMutex);

	/* Bridge object allocate memory for hash table so create
	 * bridge object only if solve bridge deals. The same true
	 * for preferans
	 */
	Bridge*pb=0;
	Preferans*pp=0;

	if(b){
		pb=new Bridge();
	}
	else{
		pp=new Preferans(false);
	}

	while (1) {
		g_mutex_lock(&m_solveAllMutex);
		k = m_solveAllNumber++;
		g_mutex_unlock(&m_solveAllMutex);
		if (k >= steps) {
			if(b){
				gdk_threads_add_idle(solve_all_foe_update_result, gpointer(m_solveAllFoeDialog->m_id));
			}
			//println("allfoe exit %d",index)
			//use break to clear memory
			break;
		}

		for (i = 0; i < sz; i++) {
			result[i] = 0;
		}

		auto&state=m_pstate[k];
		p.loadState(state);

		for (j = 0; j < state.parameter; j++, p.next()) {
			for (i=0; i < sa.n; i++) {
				ptr[o[i]] = sa.p[1];
			}

			for(auto& v:p.getIndexes()){
				ptr[o[v]] = sa.p[0];
			}

			if(b){
				pb->solveEstimateOnly(ptr,sa.trump,sa.first,trumpChanged);
				i=sa.ns ? pb->m_ns:pb->m_ew;
			}
			else{
				pp->solveEstimateOnly(ptr, sa.trump, sa.first, sa.player, sa.misere,
						sa.preferansPlayer, trumpChanged);
				i = pp->m_playerTricks;
			}

			trumpChanged = false;

			assert(i >= 0 && i < sz);
			result[i]++;
			//check only preferans, bridge in solve() function file bi.h
			if(!b && j % 50 == 0 ){
				if (needStopThread()) {
					//println("allfoe exit (user break) %d",index)
					//use goto (because need to break two cycles) to clear memory
					goto l2169;
				}
			}
		}

		g_mutex_lock(&m_solveAllMutex);
		for (i = 0; i < sz; i++) {
			sa.positions+=result[i];
			m_solveAllFoeDialog->m_result[i] += result[i];
		}
		sa.end=clock();
		g_mutex_unlock(&m_solveAllMutex);

		ur=true;
		if(b){
			t=clock();
			if( double(t-lastUpdate)/CLOCKS_PER_SEC < 1.5 ){
				ur=false;
			}
			else{
				lastUpdate=t;
			}
		}
		if(ur){
			gdk_threads_add_idle(solve_all_foe_update_result, gpointer(m_solveAllFoeDialog->m_id));
		}
	}

	l2169:
	if(b){
		delete pb;
	}
	else{
		delete pp;
	}
	delete[]result;

}

void DrawingArea::solveAllFoe(bool createDialog) {
	int i, j, k, n, *pi;
	State& state = getState();
	Permutations p;
	CARD_INDEX c[2],ci;
	auto&pr=getProblem();
	const CARD_INDEX player = pr.m_player;

	//if table full
	CARD_INDEX cid[52];
	pr.getClearCid(cid);
	CARD_INDEX first = pr.getFirst();

	m_solveAllNumber = 0;

	if(isBridge()){
		c[0]=isBridgeFoeAbsentNS() ? CARD_INDEX_NORTH:CARD_INDEX_EAST;
		c[1]=getBridgePartner(c[0]);
	}
	else{
		c[0]=pr.getNextPlayer(player);
		c[1]=getNextPlayer(c[0]);
	}

	k = state.countCards(c[0]);
	if(k==0){
		/* one of players has no cards, so another one has just one
		 * it happens in very last trick
		 */
		k=n=1;
		ci=c[0];
		c[0]=c[1];
		c[1]=ci;
	}
	else{
		n = state.countCards(c[1])+k;
	}
	p.init(k, n, COMBINATION);
	const int steps=getFoeSteps(p);

	//println("%d %d %d",k,n,steps)

	/* m_solveAllPtr[i].positions uses in m_solveAllFoeDialog
	 * updataLabels()
	 */
	for (i = 0; i < getMaxSolveAllFoeThreads(); i++) {
		m_solveAllPtr[i].positions = 0;
	}
	if(createDialog){
		m_solveAllFoeDialog = new SolveAllFoeDialog(p.number());
	}
	else{
		m_solveAllFoeDialog->setPositions(p.number());
	}

	SolveAll& s = m_solveAllPtr[0];
	s.init(k,n,first,getTrump(),c,cid,m_solveAllFoeDialog->m_id);

	if(isBridge()){
		s.ns=!northOrSouth(pr.getVeryFirstMove());
	}
	else {
		s.player = player;
		s.misere = isMisere();
		for (i = 0; i < 3; i++) {
			s.preferansPlayer[i] = getPreferansPlayer(i);
		}
	}

	//begin save permutations states
	if(steps>m_pstateSize){
		freePState();
		m_pstate=new Permutations::State[steps];
	}

	//shuffle states
	pi=new int[steps];
	for(i=0;i<steps;i++){
		pi[i]=i;
	}

	if (isBridge()) {
		for (i = 0; i < steps - 1; i++) {
			j = rand() % (steps - i);
			if (j != 0) {
				k = pi[0];
				pi[0] = pi[j];
				pi[j] = k;
			}
		}
	}

	i=j=0;
	do{
		if(i%(p.number()/steps)==0 && j<steps){
			p.saveState(m_pstate[pi[j]]);
			if(j!=0){
				m_pstate[pi[j-1]].parameter=i;
			}
			j++;
			i=0;
		}
		i++;
	}while(p.next());
	//j=steps;
	m_pstate[pi[j-1]].parameter=i;
	delete[]pi;
	//end save permutations states

	for (i = 0; i < getMaxSolveAllFoeThreads(); i++) {
		if (i > 0) {
			m_solveAllPtr[i] = m_solveAllPtr[0];
		}
		m_pThread[i] = g_thread_new("", solve_all_foe_thread, GP(i));
	}
}

void DrawingArea::solveAllDeclarers() {
	int i;

	if (isPreferans()) {
		/* #ifdef SOLVEALLDECLARERSPREFERANS_USE_THREADS use many threads gives
		 * some acceleration for two cores processor may be will be faster for
		 * more cores
		 */
#define SOLVEALLDECLARERSPREFERANS_USE_THREADS

#ifdef SOLVEALLDECLARERSPREFERANS_USE_THREADS
		m_solveAllNumber = 0;
		for (i = 0; i < getMaxRunThreads(); i++) {
			m_pThread[i] = g_thread_new("", solve_all_declarers_preferans_thread,
					gpointer(0));
		}

		for (i = 0; i < getMaxRunThreads(); i++) {
			g_thread_join(m_pThread[i]);
		}

#else
		int v, first;
		Preferans p;
		int trump;
		bool misere;
		CARD_INDEX c[52], player;

		for (i = 0; i < 52; i++) {
			ci = getState().m_cid[i];
			if (isInner(ci)) {
				ci = getOuter(ci);
			}
			c[i] = ci;
		}
		const int MAXV = getMaxSolveAllThreads();
		for (v = 0; v < MAXV; v++) {
			parsePreferansSolveAllDeclarersParameters(v, trump, misere, player);
			for (first = 0; first < 3; first++) {
				p.solveEstimateOnly(c, trump, getPreferansPlayer(first), player, misere,
						getPreferansPlayer(), first == 0);
				m_solveAllDeclarersPreferansResult[v * 3 + first] = p.m_playerTricks;
			}
		}
#endif

#undef SOLVEALLDECLARERSPREFERANS_USE_THREADS

		m_solveAllDeclarersDialog = new SolveForAllDeclarersDialog(
				m_solveAllDeclarersPreferansResult);

	}
	else {
		m_solveAllDeclarersDialog = new SolveForAllDeclarersDialog();
		m_solveAllNumber = 0;

		for (i = 0; i < getBridgeSolveAllDeclarersThreads(); i++) {
			m_pThread[i] = g_thread_new("", solve_all_declarers_bridge_thread, GP(i));
		}
	}

}

int DrawingArea::getBridgeSolveAllDeclarersThreads(){
	return std::min(getMaxRunThreads(),getMaxSolveAllThreads());
}

void DrawingArea::solveAllBridgeSetLabels(int trump) {
	//Dialog could be closed, before this function, so need to check
	if(m_solveAllDeclarersDialog){
		m_solveAllDeclarersDialog->setBridgeLabel(trump);
	}
}

void DrawingArea::solveAllFoeUpdateResult(gint64 id) {
	//Dialog could be closed, before this function, so need to check
	if (m_solveAllFoeDialog) {
		/* check that signal goes from same foe dialog,
		 * it's not a problem do not this check, because
		 * just call updateLabels(), but it give more clarify code
		 * so leave this checking. Also later some parameters can appear
		 */
		if(m_solveAllFoeDialog->m_id==id){
			m_solveAllFoeDialog->updateLabels();
		}
		else{
			//println("wrong id skip")
		}
	}
}

void DrawingArea::stopSolveAllFoeThreads() {
	stopSolveAllThreads(getMaxSolveAllFoeThreads());
	/* some solveAllFoeUpdateResult() can be in loop, so make
	 * m_solveAllFoeDialog=0 to indicate that dialog is closed
	 */
	m_solveAllFoeDialog=0;
}

void DrawingArea::stopSolveAllDeclarersBridgeThreads() {
	stopSolveAllThreads(getBridgeSolveAllDeclarersThreads());
	/* some solveAllBridgeSetLabels() can be in loop, so make
	 * m_solveAllDeclarersDialog=0 to indicate that dialog is closed
	 */
	m_solveAllDeclarersDialog=0;
}

void DrawingArea::stopSolveAllThreads(int threads){
	g_atomic_int_set(&BridgePreferansBase::m_stop, 1);

//	clock_t begin=clock();
	for (int i = 0; i < threads; i++) {
		g_thread_join(m_pThread[i]);
	}
//	println("%.3lf",double(clock()-begin)/CLOCKS_PER_SEC);

	g_atomic_int_set(&BridgePreferansBase::m_stop, 0);
}


int DrawingArea::countUndos(GdkEventButton* event){
	int i = countInnerCards();
	int j,k;
	CARD_INDEX ci;
	if (i > 0) {
		ci = getInner(getPlayer(getState().m_firstmove, true, i - 1));
		for (j = 0; j < i; j++) {
			for (k = 0; k < 52; k++) {
				if (m_cardrect[k].in(event) && getState().m_cid[k] == ci) {
					//number of undos j+1
					if (getProblem().m_currentState > j) {//check can make needed number of undos
						return j+1;
						//undoRedo(true, false, j + 1);
					}
					return 0;
				}
			}
			ci = getPreviousPlayer(ci);
		}
	}

	return 0;
}

void DrawingArea::stopCountThread(){
	if(m_solveThread){
//		START_TIMER
		g_atomic_int_set(&BridgePreferansBase::m_stop, 1);
		finishWaitFunction();//add 23feb2021
		stopTimer(m_timer);
		if (m_animationTimer) {
			stopTimer(m_animationTimer);
			endAnimation(true);
		}
		g_thread_join(m_solveThread);
		/* in case when user stop thread and after click exit this function
		 * call two times so set m_solveThread=0 to prevents two times calls of
		 * g_thread_join(m_solveThread)
		 */
		m_solveThread=0;
		g_atomic_int_set(&BridgePreferansBase::m_stop, 0);
//		OUT_TIMER
	}
}

void DrawingArea::stopTimer(guint& t){
	if(t){
		g_source_remove(t);
		t=0;
	}
}

bool DrawingArea::needStopThread(){
	return g_atomic_int_get(&BridgePreferansBase::m_stop);
}
