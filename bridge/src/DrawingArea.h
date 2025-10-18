/*
 * DrawingArea.h
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef DRAWINGAREA_H_
#define DRAWINGAREA_H_

#include <atomic>
#include "base/Config.h"
#include "base/FrameItemArea.h"
#include "base/Widget.h"
#include "CRect.h"
#include "helper/Enums.h"
#include "helper/SolveAll.h"
#include "problem/ProblemVector.h"
#include "Permutations.h"

class SolveForAllDeclarersDialog;
class SolveAllDealsDialog;
class Bridge;
class Preferans;

class DrawingArea: public FrameItemArea {
public:
	static const int ANIMATION_STEP_TIME = 70; //milliseconds//70
	static const int ANIMATION_STEPS = 10;

	//the same order using in constructor and destructor
	CairoSurface m_csEnd;

	CRect m_tableRect; //includes left/top lines, and not includes right/bottom lines
	int m_tableTop;
	/*Note
	 * m_tableRect.left+1 = horizontal region size with line ( east & west )
	 * m_tableRect.top+1 = vertical region size with line( north & south )
	 * draw horizontal lines at m_tableRect.top,m_tableRect.bottom,m_tableRect.bottom+(m_tableRect.top+1)
	 * draw vertical lines at m_tableRect.left,m_tableRect.right
	 */
	CSize m_windowSize; //include lines
	CRect m_totalTricksRect[2]; //0-horizontal, 1-vertical
	CRect m_cardrect[52];
	int m_currentId;
	CPoint m_currentPoint, m_addit;
	int m_selectedCard;
	CARD_INDEX m_lastRegion;

	guint m_timer, m_animationTimer;
	clock_t m_startTime;
	int m_estimateFontHeight;

	GThread *m_solveThread;
	int m_animationStep;
	GMutex m_mutex; //
	GCond m_condition; //
	bool m_functionWait;

	//solve for all declarers
	SolveForAllDeclarersDialog *m_solveAllDeclarersDialog;
	int m_solveAllDeclarersBridgeResult[MAX_THREADS_BRIDGE][4];
	int m_solveAllDeclarersPreferansResult[MAX_THREADS_PREFERANS * 3];

	//solve all deals
	SolveAllDealsDialog *m_solveAllDealsDialog;
	VGThreadPtr m_vThread;
	Permutations::State *m_pstate;
	int m_pstateSize;

	//solve for all declarers & solve all deals
	GMutex m_solveAllMutex;
	std::atomic_int m_solveAllNumber, m_maxv;
private:
	VSolveAll m_vSolveAll;
public:
	VSolveAll& getSolveAll() {
		return m_vSolveAll;
	}
	void startWaitFunction(GSourceFunc function, gpointer data);
	void finishWaitFunction();
	void waitForFunction();

	//save to html variables
	int m_bestHtml; //best turn index
	int m_tricksHtml[2];

	void hideArrow(bool paint);
	void showArrow(bool paint);

	void setPlayer(CARD_INDEX player);

	State& getPreviousState() {
		return getProblem().getPreviousState();
	}

	State& getNextState() {
		return getProblem().getNextState();
	}

	int getPreviousStateTricks(CARD_INDEX player) {
		return getPreviousState().m_tricks[static_cast<int>(player) - 1];
	}

	CARD_INDEX getBasePlayer() const {
		return getProblem().getBasePlayer();
	}

	void updateSkin() override {
		redraw();
	}

	void updateAfterCreation() override {
		init();
		resize();
		draw();
	}

	CSize getMaxCardSize();

	bool pointInCaption(CARD_INDEX index, GdkEventButton *event);
	VString getCaptions(CARD_INDEX index);
	void getCaptionPoint(CARD_INDEX index, int piece, double &x, double &y);
	CARD_INDEX getRealRegion(CARD_INDEX index);

	static const int INNER_INDEX_DELTA = CARD_INDEX_NORTH_INNER
			- CARD_INDEX_NORTH;

	/*using cid*/
	inline CARD_INDEX getInner(int index) const {
		assert(index >= 0 && index < 52);
		return getInner(getState().m_cid[index]);
	}

	inline CARD_INDEX getOuter(int index) const {
		assert(index >= 0 && index < 52);
		return getOuter(getState().m_cid[index]);
	}

	inline bool isInner(int index) const {
		assert(index >= 0 && index < 52);
		return isInner(getState().m_cid[index]);
	}

	inline bool isOuter(int index) const {
		assert(index >= 0 && index < 52);
		return isOuter(getState().m_cid[index]);
	}

	inline bool isOuterOrAbsent(int index) const {
		assert(index >= 0 && index < 52);
		return isOuterOrAbsent(getState().m_cid[index]);
	}

	//DO NOT MOVE THIS FUNCTIONS TO Base.h getInner(CARD_INDEX) & getInner(int) call from class DrawArea cann't define which function to call
	static inline CARD_INDEX getInner(CARD_INDEX index) {
		assert(isOuter(index));
		return static_cast<CARD_INDEX>(index + INNER_INDEX_DELTA);
	}

	static inline bool isOuter(CARD_INDEX index) {
		return index >= CARD_INDEX_NORTH && index <= CARD_INDEX_WEST;
	}

	//NORTH_INNER_INDEX -> NORTH_INDEX
	static inline CARD_INDEX getOuter(CARD_INDEX index) {
		assert(isInner(index));
		return static_cast<CARD_INDEX>(index - INNER_INDEX_DELTA);
	}

	static inline bool isInner(CARD_INDEX index) {
		return index >= CARD_INDEX_NORTH_INNER && index <= CARD_INDEX_WEST_INNER;
	}

	static inline bool isOuterOrAbsent(CARD_INDEX index) {
		return index == CARD_INDEX_ABSENT || isOuter(index);
	}

	CLICABLE_REGION getClickableRegion(GdkEventButton *event,
			CARD_INDEX &region);

	int countCards(CARD_INDEX index) {
		return getState().countCards(index);
	}

	inline bool isTableFull() const {
		return countInnerCards() == maxTableCards();
	}

	inline bool isTableFullOrEmpty() const {
		const int i = countInnerCards();
		return i == maxTableCards() || i == 0;
	}

	int countInnerCards() const;

	bool think() const {
		return m_timer != 0;
	}

	CRect getInsideRect(CARD_INDEX index) {
		return FrameItemArea::getInsideRect(m_tableRect, index);
	}

	CRect getArrowRect(CARD_INDEX index);

	int maxCardsInSuit() const {
		return isBridge() ? 13 : 8;
	}

	const gchar* getPlayerString() const;

	bool needPlayNextTurn() const;
	bool nextPlayerHasOnlyOneSuit(int &suit) const;
	bool nextPlayerIsSequenceSuit(int suit) const;
	inline bool nextPlayerHasTrumpSuit() const {
		return getState().hasSuit(getNextMove(), getProblem().m_trump);
	}

	inline bool nextPlayerHasLeadingSuit() const {
		return getProblem().hasLeadingSuit(getNextMove(),
				getProblem().m_currentState);
	}

	int getLeadingSuit() const {
		return getProblem().getLeadingSuit(getProblem().m_currentState);
	}

	CARD_INDEX getNextMove() const {
		return getProblem().getNextMove();
	}

	bool isValidTurn(int index) {
		return getProblem().isValidTurn(index, getProblem().m_currentState,
				true);
	}

	void redrawState();
	void saveState() {
		getProblem().saveState();
	}

public:
	DrawingArea();
	virtual ~DrawingArea();

	void updateEstimationType() override;
	void updateLanguage() override;
	void updateDeckSelection() override;
	void updateFontSelection() override;
	void updateArrowSelection() override;
	void updateGameType() override;
	void newGame() override;

	void solveAllDeclarersBridgeThread(int thread);
	void solveAllDeclarersPreferansThread();

	void createAllDealsDialog();
	void solveAllDeals();
	void solveAllDealsThread(int index);
	void solveAllDealsThreadInner(int index, const bool bridge, const int sz,
			int *result, Bridge *pb, Preferans *pp);
	void solveAllBridgeSetLabels(int trump);
	void solveThread(Problem *problem); //problem=NULL solve current problem, otherwise solve for html
	void makeMove(int index, bool estimateBeforeBest = false);
	void endSolveThread();
	void timer();
	gboolean animationStep(int index);
	void updateLastTrick();

	CSize getSize() const override;

	void invalidateRect(CRect const &r) {
		invalidateRect(r.left, r.top, r.width(), r.height());
	}

	void invalidateRect(gint x, gint y, gint width, gint height);

	void countSize() {
		countSize(INNER_CARD_MARGIN.cy);
	}

	void countSize(int y);

	void init() override;
	void recalcRects();
	void mouseLeftButtonDown(GdkEventButton *event);
	void mouseLeftButtonUp(GdkEventButton *event);
	void mouseMove(GdkEventButton *event);
	void mouseLeave(GdkEventCrossing *event);
	void undoRedo(bool undo, bool full, int count = 1);

	bool isUndoEnable() const {
		return isUndoEnableIgnoreThink() && !think();
	}

	bool isRedoEnable() const {
		return isRedoEnableIgnoreThink() && !think();
	}

	bool isUndoEnableIgnoreThink() const {
		return getProblem().m_currentState > 0;
	}

	bool isRedoEnableIgnoreThink() const {
		return getProblem().m_currentState < getProblem().m_maxState;
	}

	void findBest(const Problem *problem); //problem=NULL solve current problem, otherwise solve for html

	void setDeal(bool random) override;

	void copySurface(cairo_t *cr) override;

	void draw() override;		//draw in memory

	void updateAllRegions();
	void updateRegion(CARD_INDEX index, bool paint = true);
	void updateTricks();
	void updateTricks(CARD_INDEX index, bool paint = true);
	void updateInsideRegion();
	CRect getRegionRect(CARD_INDEX index);
	void showCard(cairo_t *cr, int index, int x, int y);

	CARD_INDEX getPartner(CARD_INDEX index);

	void setShowEstimation(int index, int estimation, bool thread);

	void showEstimation(cairo_t *cr, int index, int x, int y);
	inline void showEstimation(int index) {
		if (index != m_currentId) {
			showEstimation(m_cs, index, m_cardrect[index].left,
					m_cardrect[index].top);
		}
	}

	void edit();

	void saveHtml(std::string filepath, bool images) {
		saveHtml(filepath, images, getProblemVector());
	}
	void saveHtml(std::string filepath, bool images, const ProblemVector &p);

	void updateProblem();
	//likeBridge used only in preferans when {true then rotate by 90 degrees}
	void rotate(bool clockwise, bool likeBridge = false);

	void solveAllDeclarers();
	void solveAllDealsUpdateResult(gint64 id);
	void stopSolveAllDealsThreads();

	bool northInvisible() const {
		return isPreferans() && !isEditEnable()
				&& getAbsent() == CARD_INDEX_NORTH;
	}

	void stopSolveAllDeclarersBridgeThreads();
	/* count how many undos need to do if click on inner card
	 * in case if event coordinates not over inner cards, function returns 0
	 */
	int countUndos(GdkEventButton *event);

	int getSolveAllDeclarersThreads();
	void stopSolveAllThreads();
	int getSolveAllDealsSteps(Permutations const &p);

	void freePState() {
		if (m_pstate) {
			delete[] m_pstate;
		}
	}

	void stopCountThread();
	void stopTimer(guint &t);
	void endAnimation(bool stop);
	void animationDraw(bool stop);
	bool needStopThread();

	void drawCardback(int i);
};
extern DrawingArea *gdraw;

#endif /* DRAWINGAREA_H_ */
