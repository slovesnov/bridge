/*
 * Config.h
 *
 *       Created on: 08.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <map>
#include "Base.h"

typedef std::map<std::string, std::string> MapStringString;
typedef std::pair<std::string,std::string> PairStringString;

typedef std::pair<int, std::string> PairIntString;
typedef std::vector<PairIntString> VIntString;

typedef std::pair<MENU_ID, std::string> MenuString;
typedef std::vector<MenuString> VMenuString;

class Config {

	static constexpr int INNER_CARD_MARGIN[] = { 3, 3 };
	static const int INDENT_INSIDE_SUIT[];
	static const int ESTIMATION_INDENT[];
	static const int SVG_DECK_DEFAULT_PARAMETERS[];

	static const char SLASH = '/';
	static const int MAX_BUFF=4096;

	VIntPtr storeVariablesInt;
	VString storeVariablesIntNote;
	VStringPtr storeVariablesString;
	VString storeVariablesStringNote;

	VIntPtr m_options;

	std::string S[STRING_SIZE];
	MapStringString m_map;

public:
	VString m_language;
	std::string m_storeImageFormatString;
	VString m_storeImageFormat;
	std::string m_allImageFormatString;
	CRect m_workareaRect;

	//css varables (stored in css files)
	PangoFontDescription*m_font; //font for drawing area stored in bridge.css
	GdkRGBA m_fontColor; //text color on screen & dialog, stored in bridge*.css
	int m_skin; //stored in bridge.css
	GdkRGBA m_customBackgroundColor; //stored in bridge-1.css
	std::string m_customBackgroundImage; //stored in bridge-1.css
	CARD_INDEX m_absent;
	int m_bridgeSolveAllFoeAbsentNS;
	std::string m_thousandsSeparatorString;
private:
	std::string getLanguageDir()const;
	std::string getLanguageFileNameByIndex(int index) const;

	static const char* getFontFamily(const PangoFontDescription *desc) {
		return pango_font_description_get_family(desc);
	}
	static int getFontHeight(const PangoFontDescription *desc);

	void initVarables();
	void load();
	void loadIntArray(int*a, int size, const char* signature);
	void reset();
	void loadCss(); //load css and setup font
	static std::string getBasePath(const char *argv0);//TODO remove

	bool getStringBySignature(const char*signature,std::string& s){//TODO
		return getStringBySignature(std::string(signature),s);
	}
	bool getStringBySignature(const std::string& signature,std::string& s);//TODO

	static bool loadLanguage(std::string filename,VIntString& v);
public:
	Config();
	virtual ~Config();

	void setSkin(int skin, REWRITE_CSS_OPTION o) {
		m_skin = skin;
		writeAndLoadCss(o);
	}

	std::string getCssFilePath(int skin = -2);

	PangoFontDescription* getFont(int height) const;

	PangoFontDescription* getFont() const {
		return m_font;
	}

	const char* getFontFamily() {
		return pango_font_description_get_family(m_font);
	}

	PangoWeight getFontWeight() {
		return pango_font_description_get_weight(m_font);
	}

	PangoStyle getFontStyle() {
		return pango_font_description_get_style(m_font);
	}

	int getFontHeight() const {
		return getFontHeight(m_font);
	}

	inline const gchar* getString(STRING_ID id) const {
		return S[id].c_str();
	}

	void save(GAME_TYPE gt,int x,int y);

	int getLanguageIndex() const;
	void loadLanguageFile();
	void setLanguageFileName(int index);

	inline bool isChecked(int i) const {
		assert(i>=0 && i<SIZEI(CHECKED_MENU));
		return (*m_options[i]) != 0;
	}

	void switchOption(int i) {
		assert(i>=0 && i<SIZEI(CHECKED_MENU));
		*m_options[i] = !(*m_options[i]);
	}

	int getAscending() const {
		return m_ascending;
	}

	void setAscending(int ascending) {
		m_ascending = ascending;
	}

	int getSuitsOrder(int i) const {
		return m_suitsOrder[i];
	}

	void setSuitsOrder(int i, int v) {
		m_suitsOrder[i] = v;
	}

	int getIndentInsideSuit() const;

	int getEstimationIndent() const;

	inline int getCard(int suit, int card) const {
		return m_suitsOrder[suit] * 13 + (!m_ascending ? card : 12 - card);
	}

	inline int getActiveCardShift() const {
		return m_activeCardShift;
	}

	inline int getArrowMargin() const {
		return m_arrowMargin;
	}

	CSize getInnerCardMargin() const {
		return CSize(m_innerCardMargin[0], m_innerCardMargin[1]);
	}

	//[BEGIN STORING VARIABLES]order of storing/loading/reset in configuration file
	//to store and load cann't use const std::string,so use additional variable
	std::string m_version; //on save m_version=CURRENT_VERSION_STR,on load loaded from file

	std::string m_languageFileName;
	VString m_recent;
private:
	int m_suitsOrder[4];
	int m_innerCardMargin[SIZE(INNER_CARD_MARGIN)]; //margin of inside cards from center of table.
	int m_indentInsideSuit[N_RASTER_DECKS];
	int m_estimationIndent[N_RASTER_DECKS];
public:
	CPoint m_startPosition;  //Frame position

	//Note for simple type variables use alphabetical order
	int m_activeCardShift;  //upper shift of active card.
	int m_allowOnlyOneInstance;
	int m_arrowMargin;  //minimal margin between arrow and borders of table
	int m_arrowNumber;
	int m_arrowSize;
	int m_animation;
	int m_ascending;
	int m_autoPlaySequence;
	int m_cardWidth;
	int m_cardHeight;
	int m_deckNumber;
	int m_documentModifiedWarning;
	int m_eastWestCardsMargin;
private:
	ESTIMATE m_estimateType;
public:
	GAME_TYPE m_gameType;
	int m_htmlStoreWithImages;
	int m_htmlStoreBestMove;
	int m_htmlStoreNumberOfTricks;
	int m_htmlPreview;
	int m_htmlShowDialog;
	int m_indentBetweenSuits;
	int m_maxRecent;
	int m_maxRecentLength;
	int m_maxThreads;
	int m_resizeOnDeckChanged;
	int m_showCommonTricks;
	int m_splitEveryFile;
	int m_showLastTrick;
	int m_showPlayerTricks;
	int m_showToolTips;
	int m_firstSplitNumber;
	int m_lastTrickMinimalMargin;
	int m_frameDelta;
	//[END STORING VARIABLES]
	VMenuString m_vectorMenuString;

	GdkPixbuf* languagePixbuf(int id) const;
	std::string getTitle();

	//"bridge-studio.sourceforge.net" -> "net.sourceforge.bridge-studio"
	static std::string getUniqueApplicationName();

	const gchar * getPlayerString(CARD_INDEX player) const;

	bool isWritableImage(std::string const& s) const;

	int getAreaMaxHeight() {
		/* areaMaxHeight+m_frameDelta <= m_workareaRect.height()
		 *areaMaxHeight <= m_workareaRect.height() - frameDelta()
		 */
		return m_workareaRect.height() - m_frameDelta;
	}

	static bool allowOnlyOneInstance(const char* argv0);

	void writeAndLoadCss(REWRITE_CSS_OPTION o);

	ESTIMATE getEstimateType() const {
		return m_estimateType;
	}

	void setEstimateType(ESTIMATE e) {
		m_estimateType = e;
	}

	bool isScalableArrow()const;
	static bool isScalableArrow(int arrow);

	bool isScalableDeck()const;
	static bool isScalableDeck(int deck);
	//uses for svg and png deck
	CSize getCardSize()const;
	int& getCardWidth();
	int& getCardHeight();
	int getSvgIndentInsideSuit()const;
	int getSvgEstimationIndent()const;

	void setArrowParameters(int arrow,int arrowSize);
	void setDeckParameters(int deck,bool resizeOnDeckChanged,CSize cardSize);
	int recentSize();
};

extern Config* gconfig;

#endif /* CONFIG_H_ */
