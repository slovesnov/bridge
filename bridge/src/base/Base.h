/*
 * Base.h
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef BASE_H_
#define BASE_H_

#include <set>

#include "../helper/Enums.h"
#include "CRect.h"
#include "../helper/AuctionTagParseResult.h"
#include "../solver/BridgeCommon.h"

//======================== BEGIN TYPEDEFS =========================================================
typedef VString::iterator VStringI;
typedef VString::const_iterator VStringCI;

typedef std::vector<bool> VBool;

typedef std::vector<int> VInt;
typedef std::vector<double> VDouble;

typedef std::vector<STRING_ID> VStringID;
typedef std::vector<CARD_INDEX> VCardIndex;

typedef std::set<std::string> SString;
typedef std::set<int> SInt;

typedef std::vector<int*> VIntPtr;
typedef std::vector<std::string*> VStringPtr;
typedef std::vector<GtkWidget*> VGtkWidgetPtr;
typedef std::vector<GThread*> VGThreadPtr;
//======================== END TYPEDEFS ===========================================================

//======================== BEGIN CONSTANTS ========================================================
const int ESTIMATE_CLEAR = 50; //clear estimate
/* Note. Even if  DrawingArea::countSize(int y) called with y=-48 for deck5
 * need to use -49, because from DrawingArea::countSize(int y) description
 * next condition is true, but more nice view make window height a little
 * bit more than working area height
 *  if(i%2==1){
 * 		i++;
 * 	}
 * so valid value is -49, need to check condition in DeckArrowSelectionDialog
 * constructor
 * if(m_isDeck && !Widget::isScalableDeck(i) && m_maxCardHeight<gconfig->getRasterDeckCardHeight(k))
 * where m_maxCardHeight=countMaxCardSizeForY(getArrowSize()).cy;
 */
const int MIN_COUNT_SIZE_Y = -49;
const CSize INNER_CARD_MARGIN = { 3, 3 };
const int MIN_PROBLEM_SELECTOR_WIDTH = 4 * (63 + 6);
const int MIN_LAST_TRICK_WIDTH = 118;
const int MIN_GRID_SIZE_WIDTH = MIN_PROBLEM_SELECTOR_WIDTH - MIN_LAST_TRICK_WIDTH;

const CSize RASTER_DECK_CARD_SIZE[] = { { 71, 96 }, { 54, 72 }, { 54, 72 }, {
		72, 96 }, { 71, 96 }, { 75, 107 }, { 95, 125 }, { 95, 125 } };
const int N_RASTER_DECKS = SIZEI(RASTER_DECK_CARD_SIZE);
const auto minmax61 = std::minmax_element(RASTER_DECK_CARD_SIZE,RASTER_DECK_CARD_SIZE+N_RASTER_DECKS,
        [] (CSize const& a, CSize const& b) {return a.cx < b.cx;});
const int MIN_CARD_WIDTH=minmax61.first->cx;
const int MIN_CARD_HEIGHT=minmax61.first->cy;
const CSize MAX_CARD_SIZE=*minmax61.second;

//need to change MAX_SVG_WH_RATIO in Config.cpp if do changes for vector decks
const int N_VECTOR_DECKS=4;

const int RASTER_ARROW_SIZE[] = { 64, 48, 48, 64, 72, 96, 128 };
const int N_RASTER_ARROWS = SIZEI(RASTER_ARROW_SIZE);
const int MIN_ARROW_SIZE = *std::min_element(RASTER_ARROW_SIZE,
		RASTER_ARROW_SIZE + N_RASTER_ARROWS);
const int N_VECTOR_ARROWS=3;

//all arrays index is m_gameType, MIN_CONTRACT[m_gameType] MAX_CONTRACT[..] MAX_THREADS[..] TRUMP_MODEL_SIZE[..]
const int MIN_CONTRACT[] = { 1, 6 };
const int MAX_CONTRACT[] = { 7, 10 };
const int MAX_THREADS_BRIDGE =  NT + 1;
const int MAX_THREADS_PREFERANS = 18;
const int MAX_THREADS[] = { MAX_THREADS_BRIDGE, MAX_THREADS_PREFERANS };
const int TRUMP_MODEL_SIZE[] = { NT + 1, NT + 2 };
const MENU_ID TOOLBAR_MENU_ID[] = {
		MENU_FIND_BEST_MOVE,
		MENU_UNDOALL,
		MENU_UNDO,
		MENU_REDO,
		MENU_REDOALL };
const int N_SKINS = 8;
const int CONFIG_CUSTOM_SKIN = -1;
const int SKIP_ARROW_SIZE = -1;
const int MAX_BRIDGE_HAND_CARDS = 13;
const int MAX_PREFERANS_HAND_CARDS = 10;
const int MAX_RESULT_SIZE = MAX_BRIDGE_HAND_CARDS + 1;

const double CURRENT_VERSION = 5.2;
const std::string CURRENT_VERSION_STR = forma(CURRENT_VERSION);
const std::string BASE_ADDRESS = "http://bridge-studio.sourceforge.net/";
const std::string DOWNLOAD_URL = "http://sourceforge.net/projects/bridge-studio/files/latest/download";
#ifdef FINAL_RELEASE
	const char VERSION_FILE_URL[] = "http://slovesnov.users.sourceforge.net/bridge/version.txt";
#else
	const char VERSION_FILE_URL[]="file:///C:/slovesno/site/bridge/version.txt";
#endif

const char HTML_IMAGE_DIRECTORY[] = "http://slovesnov.users.sourceforge.net/img/bridge";
const std::string EMAIL = "slovesnov@yandex.ru";

const char PLAYER_CHAR[] = "nesw"; //Note. Should be coordinated with PLAYER
//for save/add html
const char SUIT_ARROW_CHAR[] = "shdcneow";
const char ROUND_CORNER_CLASS[] = "round-corner";

//ordered by alphabet
const char PBN_AUCTION_TAG[] = "Auction";
const char PBN_BOARD_TAG[] = "Board";
const char PBN_CONTRACT_TAG[] = "Contract";
const char PBN_DATE_TAG[] = "Date";
const char PBN_DEAL_TAG[] = "Deal";
const char PBN_DEALER_TAG[] = "Dealer";
const char PBN_DECLARER_TAG[] = "Declarer";
const char PBN_EAST_TAG[] = "East";
const char PBN_EVENT_TAG[] = "Event";
const char PBN_HOMETEAM_TAG[] = "HomeTeam";
const char PBN_NORTH_TAG[] = "North";
const char PBN_PLAY_TAG[] = "Play";
const char PBN_RESULT_TAG[] = "Result";
const char PBN_ROOM_TAG[] = "Room";
const char PBN_ROUND_TAG[] = "Round";
const char PBN_SCORE_TAG[] = "Score";
const char PBN_SECTION_TAG[] = "Section";
const char PBN_SITE_TAG[] = "Site";
const char PBN_SOUTH_TAG[] = "South";
const char PBN_TABLE_TAG[] = "Table";
const char PBN_VISITTEAM_TAG[] = "VisitTeam";
const char PBN_VULNERABLE_TAG[] = "Vulnerable";
const char PBN_WEST_TAG[] = "West";

//defined in Base.cpp (ordered by alphabet)
extern const char* DEALER[4];
extern const char* DOUBLE_REDOUBLE[3];
extern const char* LEADER[4];
extern const char* ROOM[2];
extern const char* SCORE[2];
extern const char* VULNERABLE[4];

const TOOLBAR_BUTTON TOOLBAR_BUTTON_ARRAY[] = {
		TOOLBAR_BUTTON_FIND_BEST,
		TOOLBAR_BUTTON_UNDOALL,
		TOOLBAR_BUTTON_UNDO,
		TOOLBAR_BUTTON_REDO,
		TOOLBAR_BUTTON_REDOALL };

const CARD_INDEX PLAYER[] = {
		CARD_INDEX_NORTH,
		CARD_INDEX_EAST,
		CARD_INDEX_SOUTH,
		CARD_INDEX_WEST };

//should match with Config::m_options which is filling in constructor of Config class
const MENU_ID CHECKED_MENU[] = {
		MENU_LAST_TRICK_GAME_ANLYSIS,
		MENU_ANIMATION,
		MENU_TOOLTIPS,
		MENU_TOTAL_TRICKS,
		MENU_PLAYER_TRICKS,

		MENU_SHOW_HTML_OPTIONS,
		MENU_SAVE_HTML_FILE_WITH_IMAGES,
		MENU_PREVIEW_HTML_FILE,
		MENU_SHOW_MODIFIED_WARNING,
		MENU_AUTOPLAY_SEQUENCE,
		MENU_SPLIT_EVERY_FILE,
		MENU_ONLY_ONE_INSTANCE };

const int TOTAL_STATES = 53;
const int SUIT_PIXBUF_SIZE = 48;

const int INVALID_CONTRACT = -2;
const int INVALID_TRUMP = -2;

const int NO_CONTRACT_SET = -1;
const int NO_TRUMP_SET = -1;

//constants for DrawingArea::m_htmlTricks & Problem::getHTMLContent for both types of game
const int HTML_TRICKS_NORTH_SOUTH=0;
const int HTML_TRICKS_EAST_WEST=1;
const int HTML_TRICKS_PLAYER=0;
const int HTML_TRICKS_WHISTERS=1;

//spades, hearts, diamond, clubs same order with SUITS_CHAR
const std::string UTF8SUITS[] = { "\xe2\x99\xa0", "\xe2\x99\xa5",
		"\xe2\x99\xa6", "\xe2\x99\xa3" };
//======================== END CONSTANTS ==========================================================
const gchar * getString(const STRING_ID& id);
const gchar * getString(const MENU_ID& id);
std::string getStringNoDots(MENU_ID id);
FILE_TYPE getFileType(std::string filepath);
int indexOfPlayer(CARD_INDEX player);
CARD_INDEX playerFromChar(char c);
int countBridgeScore(const int contract, const int trump, const int tricks,
		const int doubleRedouble, const bool vulnerable);
int countBridgeScore(const int contract, const int trump, const int tricks,
		const int doubleRedouble, const int declarer, const int vulnerable);
std::string getSuitString(int suit);
std::string getContractString(int n);
const std::string getNTString();

std::string getLowercasedPlayerString(CARD_INDEX player);
const gchar * getPlayerString(CARD_INDEX player);

/* getCardRankString() call in EditListDialog & Problem::getHTMLContent
 *
 * for queen of hearts return Q (for english language)
 * for ten of diamonds return 10
 */
std::string getCardRankString(int index);

/* getCardString() is calling from getBtsContent() getPbnContent()
 * when parsing bts and pbn files
 * so do not changes
 *
 * for queen of hearts return HQ
 * for ten of diamonds return DT
 */
std::string getCardString(int index);

int getSuitsOrder(int i);

//define some functions here to avoid using DrawingArea::getInner ...
CARD_INDEX getInner(CARD_INDEX index);
CARD_INDEX getOuter(CARD_INDEX index);
bool isInner(CARD_INDEX index);
bool isOuter(CARD_INDEX index);

int getIndentInsideSuit();

int getAreaMaxHeight();

int countEndX(std::string const& s);

AuctionTagParseResult parseAuctionTag(const std::string & auctionValue,
		const std::string & auctionAdd);

/*[en] s,h,d,c,nt
[ru] ο,χ,α,ς,ακ
*/
//std::string getLowerTrumpString(int trump);
//std::string getUpperTrumpString(int trump);
std::string getEnglishTrumpString(int trump);

std::string getBgImageName(int i);

/* gdk_rgba_to_string() sometimes return ugly string
 * for example rgba(0,255,0,2.15684e-314)
 */
std::string rgbaToString(const GdkRGBA c);

unsigned rgbaToUnsigned(const GdkRGBA c);

void copyFromPixbuf(GdkPixbuf* source, cairo_t * dest, CRect const& rect);

#ifndef FINAL_RELEASE
void exploreAllChildrenRecursive(GtkWidget* w);
#endif

CARD_INDEX getPlayerForArray(CARD_INDEX player, const CARD_INDEX*a,
		int size, bool next, int count = 1);
CARD_INDEX getBridgePlayer(CARD_INDEX player, bool next,
		int count = 1);
CARD_INDEX getNextBridgePlayer(CARD_INDEX player);
CARD_INDEX getPreviousBridgePlayer(CARD_INDEX player);
CARD_INDEX getBridgePartner(CARD_INDEX player);

ESTIMATE getEstimateType();

int isBridgeSolveAllDealsAbsentNS();
void setBridgeSolveAllDealsAbsentNS(int ns);

BUTTON_STATE boolToButtonState(bool b);

gint message(MESSAGE_ICON_TYPE t, const char* s,
		BUTTONS_DIALOG_TYPE type);
gint message(MESSAGE_ICON_TYPE t, const std::string& s,
		BUTTONS_DIALOG_TYPE type);
void message(MESSAGE_ICON_TYPE t, const char* s);
void message(MESSAGE_ICON_TYPE t, const std::string& s);
void message(MESSAGE_ICON_TYPE t, STRING_ID id,BUTTONS_DIALOG_TYPE type=BUTTONS_DIALOG_OK);

GdkPixbuf* getSuitPixbuf(int suit, int size = SUIT_PIXBUF_SIZE);

GdkPixbuf* getContractPixbuf(int n);

//use same mechanism for selectColor & selectFont
bool selectColor(const char* s, GdkRGBA* color);
bool selectFont(const char* s, PangoFontDescription*& font);

bool think();

std::string getArrowFileName(int n,bool svg=false);
std::string getDeckFileName();
std::string getDeckFileName(int n,bool svg=false);

std::string intToStringLocaled(int v);
void unsignedToGdkRGBA(unsigned v,GdkRGBA&c);

GtkWidget* createMarkupLabel(std::string const& s,int maxChars=0);
GtkWidget* createMarkupLabel(STRING_ID id,int maxChars=0);
GtkWidget* createBoldLabel(STRING_ID id);
GtkWidget* createBoldLabel(std::string const& s);
GtkWidget* createUnderlinedLabel(STRING_ID id);
GtkWidget* createUnderlinedLabel(CARD_INDEX id);
GtkWidget* createUnderlinedLabel(std::string const& s);

GtkWidget* containerGetChild(GtkWidget* w,int n);

std::string getPlayerString(const CARD_INDEX cid[52],CARD_INDEX player,bool includeInner=false);
void showOpenFileError();
#endif /* BASE_H_ */
