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

typedef std::pair<int, std::string> PairIntString;
typedef std::vector<PairIntString> VIntString;

typedef std::pair<MENU_ID, std::string> MenuString;
typedef std::vector<MenuString> VMenuString;

class Config {
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

	//begin css varables
	PangoFontDescription*m_font;
	int m_skin;
	GdkRGBA m_customSkinBackgroundColor;
	std::string m_customSkinBackgroundImagePath;
	GdkRGBA m_customSkinFontColor;
	GdkRGBA m_skinFontColor[N_SKINS];
	int m_customSkinBackgroundIsColor;//1 if color as background for custom skin is used, otherwise image is used
	//end css variables

	CARD_INDEX m_absent;
	int m_bridgeSolveAllDealsAbsentNS;
	std::string m_thousandsSeparatorString;

private:
	std::string getLanguageDir()const;
	std::string getLanguageFileNameByIndex(int index) const;

	static const char* getFontFamily(const PangoFontDescription *desc) {
		return pango_font_description_get_family(desc);
	}

	void initVarables();
	void load();
	void loadIntArray(int*a, int size, const char* signature);

	bool getStringBySignature(const char*signature,std::string& s);
	bool getStringBySignature(const std::string& signature,std::string& s);

	static bool loadLanguage(std::string filename,VIntString& v);
public:
	Config();
	virtual ~Config();

	void reset(bool fromMenu=false);

	void loadCSS();
	void updateCSS();//if some css parameters are changed
	void setSkin(int skin);

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

	int getFontHeight() const;

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

	//[BEGIN STORING VARIABLES]order of storing/loading/reset in configuration file
	//to store and load cann't use const std::string,so use additional variable
	std::string m_version; //on save m_version=CURRENT_VERSION_STR,on load loaded from file

	std::string m_languageFileName;
	VString m_recent;
private:
	int m_suitsOrder[4];
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

	static bool allowOnlyOneInstance();

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
	void setCardSize(CSize const&size);
	int getCardWidth();
	int getCardHeight();
	int getSvgIndentInsideSuit()const;
	int getSvgEstimationIndent()const;

	void setArrowParameters(int arrow, int arrowSize=SKIP_ARROW_SIZE);
	void setDeckParameters(int deck,bool resizeOnDeckChanged,CSize cardSize);
	int recentSize();
	GdkRGBA& getFontColor();

	int countTableSize(int cardHeight,int arrowSize,int y);
	int countTableTop(int cardHeight);
	int countAreaHeight(int cardHeight,int arrowSize,int y);
	CSize countMaxCardSizeForY(int arrowSize,int y=MIN_COUNT_SIZE_Y);
	void resetSettings();
};

extern Config* gconfig;

#endif /* CONFIG_H_ */
