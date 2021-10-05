/*
 * DrawingArea.h
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef DRAWINGAREA_H_
#define DRAWINGAREA_H_

#include "base/Config.h"
#include "base/FrameItemArea.h"
#include "base/Widget.h"
#include "helper/CPoint.h"
#include "helper/CRect.h"
#include "helper/CSize.h"
#include "helper/Enums.h"
#include "helper/SolveAll.h"
#include "problem/ProblemVector.h"
#include "solver/Permutations.h"

class SolveForAllDeclarersDialog;
class SolveAllFoeDialog;

class DrawingArea: public FrameItemArea {
public:
	static const int ANIMATION_STEP_TIME = 70; //milliseconds//70
	static const int ANIMATION_STEPS = 10;

	//the same order using in constructor and destructor
	GdkPixbuf* m_totalTricksImage[2]; //0-horizontal, 1-vertical
	cairo_t* m_crEnd;
	cairo_surface_t *m_surfaceEnd;

	CRect m_tableRect; //includes left/top lines, and not includes right/bottom lines
	int m_tableTop;
	/*Note
	 * m_tableRect.left+1 = horizontal region size with line ( east & west )
	 * m_tableRect.top+1 = vertical region size with line( north & south )
	 * draw horizontal lines at m_tableRect.top,m_tableRect.bottom,m_tableRect.bottom+(m_tableRect.top+1)
	 * draw vertical lines at m_tableRect.left,m_tableRect.right
	 */
	CSize m_windowSize; //include lines
	CSize m_totalTricksImageSize0; //horizontal
	CRect m_totalTricksRect[2]; //0-horizontal, 1-vertical
	CRect m_cardrect[52];
	int m_currentId;
	CPoint m_currentPoint, m_addit;
	int m_selectedCard;
	CARD_INDEX m_lastRegion;

	guint m_timer,m_animationTimer;
	clock_t m_startTime;
	int m_estimateFontHeight;

	GThread*m_solveThread;
	int m_animationStep;
	GMutex m_mutex; //
	GCond m_condition; //
	bool m_functionWait;

	//solve for all declarers
	SolveForAllDeclarersDialog *m_solveAllDeclarersDialog;
	int m_solveAllDeclarersBridgeResult[MAX_THREADS_BRIDGE][4];
	int m_solveAllDeclarersPreferansResult[MAX_THREADS_PREFERANS * 3];

	//solve all foe
	SolveAllFoeDialog *m_solveAllFoeDialog;
	GThread**m_pThread;
	Permutations::State*m_pstate;
	int m_pstateSize;

	//solve for all declarers & solve all foe
	GMutex m_solveAllMutex;
	gint m_solveAllNumber;
	SolveAll* m_solveAllPtr;

	void startWaitFunction(GSourceFunc function, gpointer data);
	void finishWaitFunction();
	void waitForFunction();

	//save to html variables
	int m_bestHtml; //best turn index
	int m_northSouthTricksHtml;
	int m_eastWestTricksHtml;

	void hideArrow(bool paint);
	void showArrow(bool paint);

	inline void setPlayer(CARD_INDEX player) {
		getProblem().m_player = player;
	}

	inline State& getPreviousState() {
		return getProblem().getPreviousState();
	}

	inline State& getNextState() {
		return getProblem().getNextState();
	}

	inline int getPreviousStateTricks(CARD_INDEX player) {
		return getPreviousState().m_tricks[static_cast<int>(player) - 1];
	}

	CARD_INDEX getBasePlayer() const {
		return getProblem().getBasePlayer();
	}

	virtual void updateSkin() {
		redraw();
	}

	virtual void updateAfterCreation() {
		init();
		resize();
		draw();
	}

	CSize getMaxCardSize();

	bool pointInCaption(CARD_INDEX index, GdkEventButton* event);
	VString getCaptions(CARD_INDEX index);
	void getCaptionPoint(CARD_INDEX index, int piece, double&x, double&y);
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

	CLICABLE_REGION getClickableRegion(GdkEventButton* event, CARD_INDEX& region);

	int countCards(CARD_INDEX index){
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

	int getTricks(CARD_INDEX player) const;

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

	const gchar * getPlayerString() const;

	bool needPlayNextTurn() const;
	bool nextPlayerHasOnlyOneSuit(int& suit) const;
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
		return getProblem().isValidTurn(index, getProblem().m_currentState, true);
	}

	void redrawState();
	void saveState() {
		getProblem().saveState();
	}

public:
	DrawingArea();
	virtual ~DrawingArea();

	virtual void updateEstimationType();
	virtual void updateLanguage();
	virtual void updateDeckSelection();
	virtual void updateArrowSize() {
		initResizeRedraw();
	}

	virtual void updateGameType() {
		recalcRects();
		initResizeRedraw();
	}

	virtual void newGame() {
		recalcRects();
		initResizeRedraw();
	}

#ifdef FINAL_RELEASE
	void solveAllDeclarersBridgeThread();
#else
	void solveAllDeclarersBridgeThread(int thread);
#endif
	void solveAllDeclarersPreferansThread();

	void solveAllFoe(bool createDialog=true);
	void solveAllFoeThread(int index);
	void solveAllBridgeSetLabels(int trump);
	void solveThread(Problem* problem); //problem=NULL solve current problem, otherwise solve for html
	void makeMove(int index,bool estimateBeforeBest=false);
	void endSolveThread();
	void timer();
	gboolean animationStep(int index);
	void updateLastTrick();

	virtual CSize getSize() const;

	void invalidateRect(CRect const& r) {
		invalidateRect(r.left, r.top, r.width(), r.height());
	}

	void invalidateRect(gint x, gint y, gint width, gint height);

	void countSize() {
		countSize(getInnerCardMargin().cy);
	}

	void countSize(int y);

	virtual void init();
	void recalcRects();
	void mouseLeftButtonDown(GdkEventButton* event);
	void mouseLeftButtonUp(GdkEventButton* event);
	void mouseMove(GdkEventButton* event);
	void mouseLeave(GdkEventCrossing* event);
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

	void findBest(const Problem* problem); //problem=NULL solve current problem, otherwise solve for html

	virtual void setDeal(bool random);

	virtual void copySurface(cairo_t* cr) {
		cairo_set_source_surface(cr, m_currentId == -1 ? m_surface : m_surfaceEnd,
				0, 0);
		cairo_paint(cr);
	}

	virtual void draw();		//draw in memory

	void updateAllRegions();
	void updateRegion(CARD_INDEX index, bool paint = true);
	void updateTricks();
	void updateTricks(CARD_INDEX index, bool paint = true);
	void updateInsideRegion();
	CRect getRegionRect(CARD_INDEX index);
	void showCard(cairo_t * ct, int index, int x, int y);

	CARD_INDEX getPartner(CARD_INDEX index);

	void setShowEstimation(int index, int estimation, bool thread);

	void showEstimation(cairo_t * ct, int index, int x, int y);
	inline void showEstimation(int index) {
		if (index != m_currentId) {
			showEstimation(m_cr, index, m_cardrect[index].left,
					m_cardrect[index].top);
		}
	}

	void edit();

	void saveHtml(std::string filepath, bool images) {
		saveHtml(filepath, images, getProblemVector());
	}
	void saveHtml(std::string filepath, bool images, const ProblemVector& p);

	void updateProblem();
	//likeBridge used only in preferans when {true then rotate by 90 degrees}
	void rotate(bool clockwise, bool likeBridge = false);

	void solveAllDeclarers();
	void solveAllFoeUpdateResult(gint64 id);
	void stopSolveAllFoeThreads();

	bool northInvisible() const {
		return isPreferans() && !isEditEnable() && getAbsent() == CARD_INDEX_NORTH;
	}

	void stopSolveAllDeclarersBridgeThreads();
	//TODO
	#define getMaxSolveAllFoeThreads getMaxRunThreads
	/* count how many undos need to do if click on inner card
	 * in case if event coordinates not over inner cards, function returns 0
	 */
	int countUndos(GdkEventButton* event);

	int getBridgeSolveAllDeclarersThreads();
	void stopSolveAllThreads(int threads);
	int getFoeSteps(Permutations const& p);

	void freePState(){
		if(m_pstate){
			delete[]m_pstate;
		}
	}

	void stopCountThread();
	void stopTimer(guint& t);
	void endAnimation(bool stop);
	void animationDraw(bool stop);
	bool needStopThread();

};
extern DrawingArea* gdraw;

#endif /* DRAWINGAREA_H_ */
