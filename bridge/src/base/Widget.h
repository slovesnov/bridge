/*
 * Widget.h
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef WIDGET_H_
#define WIDGET_H_

#include "Config.h"
#include "../helper/FileChooserResult.h"
#include "../problem/ProblemVector.h"
#include "../helper/TextWithAttributes.h"

class Frame;
class DrawingArea;
class LastTrick;
class Toolbar;
class Menu;
class ProblemSelector;
class SelectorToolbar;

class Widget {
	GtkWidget *m_widget;

	//add 'widget' to names to allow descendant classes use normal names
	VString m_widgetFiles;
	void widgetAddFile(char* url);
	void widgetAddDir(char* url);

protected:
	/* use isBridge() isPreferans() isMisere() etc functions here not in Base because
	 * class Problem is inherit from Base but functions isBridge() ...
	 * is different, because this class has it's own m_gametype
	 */
	bool isBridge() const {
		return getProblem().isBridge();
	}

	bool isPreferans() const {
		return getProblem().isPreferans();
	}

	GAME_TYPE getGameType() const {
		return getProblem().m_gameType;
	}

	bool isMisere() const {
		//getGameType()==PREFERANS DON'T REMOVE
		return getGameType() == PREFERANS && getProblem().m_misere;
	}

	inline State const& getState() const {
		return getProblem().getState();
	}

	inline State& getState() {
		return getProblem().getState();
	}

	inline int minContract() const {
		return getProblem().minContract();
	}

	inline int maxContract() const {
		return getProblem().maxContract();
	}

	gint getTrump() const {
		return getProblem().m_trump;
	}
	/*
	 gint getContract()const{
	 return getProblem().m_contract;
	 }
	 */

	const ProblemVector& getProblemVector() const;
	ProblemVector& getProblemVector();

	bool isModified() const;

	bool isEditEnable() const;
	void enableEdit(bool enable);

	bool isUndoEnable() const;
	bool isRedoEnable() const;

	bool onlyPreferansProblems() const {
		return getProblemVector().onlyPreferansProblems();
	}

	inline int getMaxSolveAllThreads() const {
		return MAX_THREADS[getGameType()];
	}

	gint getComboPosition(GtkWidget*w) {
		return gtk_combo_box_get_active(GTK_COMBO_BOX(w));
	}
	void setComboPosition(GtkWidget*w, gint position) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(w), position);
	}

	inline int maxTableCards() const {
		return getProblem().maxTableCards();
	}

	CARD_INDEX getNextPlayer(CARD_INDEX player) const {
		return getProblem().getNextPlayer(player);
	}

//	GtkWidget* getToolbarImage(TOOLBAR_BUTTON id, bool small);
//	GtkWidget* getToolbarImage(TOOLBAR_BUTTON id, bool small, bool enabled);
	GtkWidget* getToolbarImage(TOOLBAR_BUTTON id, bool small, BUTTON_STATE state);
//	GdkPixbuf* getToolbarPixbuf(TOOLBAR_BUTTON id, bool small);
//	GdkPixbuf* getToolbarPixbuf(TOOLBAR_BUTTON id, bool small, bool enabled);
	GdkPixbuf* getToolbarPixbuf(TOOLBAR_BUTTON id, bool small, BUTTON_STATE state);

	int getArrowNumber() const {
		return gconfig->m_arrowNumber;
	}

	int& getArrowSize() const {
		return gconfig->m_arrowSize;
	}

	inline int getActiveCardShift() const {
		return gconfig->getActiveCardShift();
	}
	inline int getArrowMargin() const {
		return gconfig->getArrowMargin();
	}

	CSize getCardSize() const;
	int& getCardWidth();
	int& getCardHeight();

	int indexOfPreferansPlayer(CARD_INDEX player) const {
		//can't use INDEX_OF macro for pointer
		return indexOf(getPreferansPlayer(), 3, player);
	}

	CARD_INDEX getPreferansPlayer(int i) const {
		return getPreferansPlayer()[i];
	}

	const CARD_INDEX* getPreferansPlayer() const {
		return getProblem().m_preferansPlayer;
	}

	ProblemSelector& getProblemSelector() const;
	Problem& getProblem() const;
	CARD_INDEX getAbsent() const {
		return getProblem().m_absent;
	}
public:

	Widget(GtkWidget*widget);
	virtual ~Widget();

	static void staticInit();

	inline void show() {
		gtk_widget_show(getWidget());
	}

	inline void hide() {
		gtk_widget_hide(getWidget());
	}

	void updateModified();

	void setDragDrop(GtkWidget* widget);

	void showAll() {
		gtk_widget_show_all(m_widget);
	}

	inline void setWidget(GtkWidget *widget) {
		m_widget = widget;
	}

	DrawingArea& getArea() const;
	LastTrick& getLastTrick() const;
	Toolbar& getToolbar() const;

	void setFont(cairo_t* cairo);

	void addFileExtension(std::string& filepath, FILE_TYPE filetype);
	//correctFileExtension if need
	void correctFileExtension(std::string& filepath, FILE_TYPE filetype);

	inline gboolean isVisible() const {
		return gtk_widget_get_visible(getWidget());
	}

	inline GtkWidget *getWidget() const {
		return m_widget;
	}

	//begin helper functions (short names), to use function() instead of gconfig->function()

	GtkWidget* getSuitImage(int suit, int size) const {
		GdkPixbuf* pixbuf = getSuitPixbuf(suit, size);
		return gtk_image_new_from_pixbuf(pixbuf);
	}

	inline PangoFontDescription* getFont(int height) {
		return gconfig->getFont(height);
	}

	inline PangoFontDescription* getFont() {
		return gconfig->getFont();
	}

	int getFontHeight() const;

	//no correction needed
	FileChooserResult fileChooserOpen(FILE_TYPE filetype);
	//save and correct filename
	FileChooserResult fileChooserSave(FILE_TYPE filetype);

	FileChooserResult fileChooser(MENU_ID menu, const std::string& filepath,
			bool allowSplit);
	FileChooserResult fileChooser(MENU_ID menu, FILE_TYPE filetype,
			CHOOSER_OPTION option, const std::string& filepath);

	virtual void openUris(char**uris);
	void openFiles(const std::string& files) {
		openFiles(files.c_str());
	}
	void openFiles(const char*files);
	//end helper functions

	GtkWidget* createImageCombobox();
	GtkTreeModel* createTrumpModel(bool onlySuits, bool small, GAME_TYPE gt);
	GtkTreeModel* createTrumpModel(bool onlySuits, bool small = false);
	GtkTreeModel* createContractModel();
	GdkPixbuf* getStringPixbuf(bool nt, int size = SUIT_PIXBUF_SIZE);

	static const VString createStringVector(const char* text[], guint size);
	static GtkTreeModel* createTextModel(const VString& text);
	GtkWidget* createTextCombobox(const VString& text);
	GtkWidget* createTextCombobox(const char* text[], guint size) {
		return createTextCombobox(createStringVector(text, size));
	}
	GtkWidget* createTextCombobox(const STRING_ID i1, const STRING_ID i2);

	GtkWidget* createTextCombobox(int from, int to, int step = 1,
			const char* additionalString = NULL);

	const VString& getValidFilesList(char** uris);

	void menuClick(MENU_ID id);

	bool isTrumpChanged() const;
	void setTrumpChanged(bool changed);

	void showToolTip(STRING_ID id);
	void hideToolTip();

	CARD_INDEX getPreviousPlayer(CARD_INDEX player) {
		return getPlayer(player, false);
	}

	CARD_INDEX getNextPlayer(CARD_INDEX player) {
		return getPlayer(player, true);
	}

	CARD_INDEX getPlayer(CARD_INDEX player, bool next, int count = 1) {
		return getProblem().getPlayer(player, next, count);
	}

	void parsePreferansSolveAllDeclarersParameters(int v, int& trump, bool& misere,
			CARD_INDEX& player);

	int getMaxRunThreads() const;

	int getMaxHandCards()const;

	int countTableSize(int cardHeight,int arrowSize,int y);
	int countTableTop(int cardHeight);
	int countAreaHeight(int cardHeight,int arrowSize,int y);
	CSize countMaxCardSizeForY(int arrowSize,int y=MIN_COUNT_SIZE_Y);
	int countMaxArrowSizeForY(int cardHeight,int y=MIN_COUNT_SIZE_Y);

	template<class T> static void createNew(T*& dest, T* source) {
		destroy(dest);
		dest = source;
	}

	static void createNew(GdkPixbuf *& pb,int width,int height) {
		createNew(pb, gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, width, height));
	}

	static void createNew(cairo_t*& cairo, cairo_surface_t*& surface,
			CSize size) {
		createNew(surface,
				cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size.cx, size.cy));
		createNew(cairo, cairo_create(surface));
	}

	GdkPixbuf *& getSvgPixbuf(bool isDeck);

	bool isScalableArrow();
	static bool isScalableArrow(int arrow);

	bool isScalableDeck();
	static bool isScalableDeck(int deck);

	/**
	 * centerx - center horizontally, centery - center vertically
	 * if center=true then draw centered text in rectangle r
	 * if center=false then draw text in point r.left,r.top. Parameters r.right, r.bottom ignore
	 */
	void drawTextToCairo(cairo_t* ct, TextWithAttributes text, CRect r, bool centerx,
			bool centery);

	GdkRGBA getTextColor() const;
	PangoLayout* createPangoLayout(TextWithAttributes text,cairo_t *cr);

	CSize getMaxSize()const;

	void updateUndoRedoAll();
	void updateFindBestStateAll();
	void updateThinkAll();
	int recentSize();

	Menu& getMenu();

	bool isToolbarButtonEnabled(TOOLBAR_BUTTON id);
	BUTTON_STATE getToolbarButtonState(TOOLBAR_BUTTON id);
	BUTTON_STATE getFindBestState(bool checkTrump = true) const;

	bool isValidDeal(bool checkTrump = true) const;
	bool isNewDeal() const;
	bool isEmptyDeal() const;
	DEAL_STATE getDealState(bool checkTrump = true) const;

	void showAllExclude(VGtkWidgetPtr const& v);
	static void showAllExclude(GtkWidget*w,VGtkWidgetPtr const&v);

	cairo_surface_t * getBackgroundFullSurface();

	static inline void copy(cairo_surface_t * source, cairo_t * dest, CRect r) {
		copy(source, dest, r.left, r.top, r.width(), r.height(), r.left, r.top);
	}

	static inline void copy(cairo_surface_t * source, cairo_t * dest, int destx,
			int desty, int width, int height) {
		copy(source, dest, destx, desty, width, height, destx, desty);
	}

	static inline void copy(cairo_surface_t * source, cairo_t * dest, int destx,
			int desty, int width, int height, int sourcex, int sourcey) {
		cairo_set_source_surface(dest, source, destx - sourcex, desty - sourcey);
		cairo_rectangle(dest, destx, desty, width, height);
		cairo_fill(dest);
	}

	static inline void copy(cairo_surface_t * source, cairo_t * dest) {
		cairo_set_source_surface(dest, source, 0, 0);
		cairo_paint(dest);
	}

};


#endif /* WIDGET_H_ */
