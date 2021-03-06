/*
 * Base.cpp
 *
 *       Created on: 12.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include <glib.h>
#include <glib/gstdio.h>
#include "Base.h"
#include "../Frame.h"
#include "../Menu.h"
#include "../DrawingArea.h"
#include "../ProblemSelector.h"
#include "../dialogs/MessageDialog.h"

//ordered by alphabet
const char* DEALER[] = { "N", "E", "S", "W" }; //same it's in pbn
const char* DOUBLE_REDOUBLE[] = { "", "X", "XX" }; //same it's in pbn
const char* LEADER[] = { "North", "East", "South", "West" }; //not depend on the current language used to save df and pbn files
const char* ROOM[] = { "Open", "Closed" }; //same it's in pbn
const char* SCORE[] = { "NS", "EW" }; //same it's in pbn
const char* VULNERABLE[] = { "None", "NS", "EW", "All" }; //same it's in pbn

const char SVG[]="svg";
const char PNG[]="png";


FILE_TYPE getFileType(std::string filepath) {
	std::string ext = getFileInfo(filepath, FILEINFO::LOWER_EXTENSION);
	if (ext == "pbn") {
		return FILE_TYPE_PBN;
	}
	else if (ext == "bts") {
		return FILE_TYPE_BRIDGE;
	}
	else if (ext == "pts") {
		return FILE_TYPE_PREFERANS;
	}
	else if (ext == "txt" || ext == "dat") {
		return FILE_TYPE_DF;
	}
	else if (ext == "htm" || ext == "html") {
		return FILE_TYPE_HTML;
	}
	else if (ext == "csv") {
		return FILE_TYPE_CSV;
	}
	else {
		if (gconfig->isWritableImage(ext)) {
			return FILE_TYPE_IMAGE;
		}
		else {
			return FILE_TYPE_ERROR;
		}
	}
}

CARD_INDEX playerFromChar(char c) {
	const char*p = strchr(PLAYER_CHAR, c);
	if (p == NULL) {
		return CARD_INDEX_INVALID;
	}
	else {
		return PLAYER[p - PLAYER_CHAR];
	}
}

const gchar* getString(const STRING_ID& id) {
	return gconfig->getString(id);
}

const gchar* getString(const MENU_ID& id) {
	return gmenu->getString(id);
}

std::string getStringNoDots(MENU_ID id) {
	const gchar *p = getString(id);
	const gchar *p1 = strchr(p, '.');
	return p1 == NULL ? p : std::string(p, p1 - p);
}

/* contract 1-7
 * trump //for order see SUITS_CHAR
 * tricks 0-13
 * doubleRedouble=0 simple game; =1 double; =2 redouble
 * declarer 0 - north, 1 - east, 2 - south, 3 - west
 * vulnerable 0 - none, 1 - north/south, 2 - east/west, 3 - all
 */
int countBridgeScore(const int contract, const int trump, const int tricks,
		const int doubleRedouble, const int declarer, const int vulnerable) {
	bool zone;
	//checked!
	if (vulnerable == 0 || vulnerable == 3) {
		zone = vulnerable == 3;
	}
	else {
		zone = declarer % 2 != vulnerable % 2;
	}

	return countBridgeScore(contract, trump, tricks,
			doubleRedouble, zone);
}

int countBridgeScore(const int contract, const int trump, const int tricks,
		const int doubleRedouble, const bool vulnerable){
	int i, j;
	int res = 0;
	int additionalTricks = tricks - contract - 6;
	const int suitType = trump == NT ? 2 : (trump < 2 ? 1 : 0);
	const bool zone = vulnerable;

	if (additionalTricks >= 0) {
		//1 partial notation
		res = (suitType == 0 ? 20 : 30) * contract;
		if (suitType == 2) {
			res += 10;
		}
		if (doubleRedouble != 0) {
			res *= (doubleRedouble == 1 ? 2 : 4);
		}

		//2 premium for partial notation,game, small helmet and big helmet
		if (res < 100) {
			res += 50;
		}
		else {
			res += zone ? 500 : 300;
			if (contract == 6) {
				res += zone ? 750 : 500;
			}
			else if (contract == 7) {
				res += zone ? 1500 : 1000;
			}
		}

		//3 additional tricks above bidding
		if (additionalTricks > 0) {
			if (doubleRedouble == 0) {
				res += (suitType == 0 ? 20 : 30) * additionalTricks;
			}
			else {
				res += 100 * (zone + 1) * doubleRedouble * additionalTricks;
			}
		}

		//4 premium for contra & recontra
		res += 50 * doubleRedouble;
	}
	else { //lost contract
		j = -additionalTricks;
		if (doubleRedouble == 0) {
			res = -(zone ? 100 : 50) * j;
		}
		else {
			for (i = 1; i < j + 1; i++) {
				if (i == 1) {
					res -= zone ? 200 : 100;
				}
				else if (i == 2 || i == 3) {
					res -= zone ? 300 : 200;
				}
				else {
					res -= 300;
				}
			}
			if (doubleRedouble == 2) {
				res *= 2;
			}
		}
	}
	return res;
}

const std::string getNTString() {
	std::string s = getString(STRING_NT);
	return utf8ToUpperCase(s);
}

std::string getLowercasedPlayerString(CARD_INDEX player){
	std::string s=getPlayerString(player);
	return utf8ToLowerCase(s);
}

const gchar* getPlayerString(CARD_INDEX player) {
	return gconfig->getPlayerString(player);
}

std::string getCardRankString(int index) {
	int card = 12 - index % 13; //0 - two, 8 - ten, 9 - jack
	if (card < 9) {
		return format("%d", card + 2);
	}
	else {
		return getString(static_cast<STRING_ID>(STRING_J + card - 9));
	}
}

std::string getCardString(int index) {
	assert(index >= 0 && index < 52);
	return format("%c%c", toupper(SUITS_CHAR[index / 13]),
			toupper(RANK[index % 13]));
}

int getSuitsOrder(int i) {
	return gconfig->getSuitsOrder(i);
}

int getInverseSuitsOrder(int i){
	int j;
	for(j=0;j<4 && gconfig->getSuitsOrder(j)!=i;j++);
	assert(j<4);
	return j;
}

CARD_INDEX getInner(CARD_INDEX index) {
	return DrawingArea::getInner(index);
}

CARD_INDEX getOuter(CARD_INDEX index) {
	return DrawingArea::getOuter(index);
}

bool isInner(CARD_INDEX index) {
	return DrawingArea::isInner(index);
}

bool isOuter(CARD_INDEX index) {
	return DrawingArea::isOuter(index);
}

int getIndentInsideSuit() {
	return gconfig->getIndentInsideSuit();
}

int getAreaMaxHeight() {
	return gconfig->getAreaMaxHeight();
}

int countEndX(const std::string& s) {
	int i, j = s.length() - 1;
	for (i = j; i >= 0 && tolower(s[i]) == 'x'; i--)
		;
	return j - i;
}

AuctionTagParseResult parseAuctionTag(const std::string& auctionValue,
		const std::string& auctionAdd) {
	VStringCI it, ito;
	VString a, o;
	std::string s;
	const char*p, *q;
	char c;
	int i, pass, ncontract, ntrump, contract, trump, declarer;
	int doubleredouble;
	int firstDeclarer[2][NT + 1];
	const char SO[] = "cdhsn";

	for (i = 0; i < 2; i++) {
		for (declarer = 0; declarer < NT + 1; declarer++) {
			firstDeclarer[i][declarer] = -1;
		}
	}

	p = auctionAdd.c_str();
	do {
		q = p;
		for (; *p != 0 && strchr("\n \t", *p) == NULL; p++)
			;
		s = std::string(q, p - q);
		o.push_back(s);
		std::transform(s.begin(), s.end(), s.begin(), ::tolower);
		a.push_back(s);
		for (; *p != 0; p++) {
			if (strchr("\n \t", *p) == NULL) {
				break;
			}
			if (*p == '\n') {
				if (a.size() % 4 != 0) {
					return STRING_ERROR_AUCTION_TAG_NUMBER_OF_CALLS_IN_ONE_ROW_SHOULD_BE_EQUALS_OR_LOWER_THAN_FOUR;
				}
			}
		}
	} while (*p != 0);

	//check all pass
	if (a.size() == 4) {
		for (it = a.begin(); it != a.end(); it++) {
			if (*it != "pass") {
				break;
			}
		}
		if (it == a.end()) {
			return AuctionTagParseResult(STRING_OK, "Pass", 0);
		}
	}

	if (a.size() < 4) {
		return STRING_ERROR_AUCTION_TAG_NUMBER_OF_CALLS_IS_LOWER_THAN_FOUR;
	}

	//check last three pass
	for (i = int(a.size()) - 3; i < int(a.size()); i++) {
		if (a[i] != "pass") {
			return STRING_ERROR_AUCTION_TAG_LAST_THREE_CALLS_SHOULD_BE_PASS;
		}
	}

	pass = 0;
	contract = -1;
	trump = -1;
	declarer = -1;
	doubleredouble = 0;

	for (it = a.begin(), ito = o.begin(); it != a.end(); it++, ito++) {
		if (*it == "pass") {
			pass++;
			if (pass > 3) {
				return STRING_ERROR_AUCTION_TAG_MORE_THAH_THREE_PASS_IN_A_ROW_FOUND;
			}
			continue;
		}
		pass = 0;

		if (*it == "x") {
			if (declarer == -1 || (it - a.begin()) % 2 == declarer % 2
					|| doubleredouble != 0) {
				return STRING_ERROR_AUCTION_TAG_INVALID_DOUBLE_CALL;
			}
			doubleredouble = 1;
			continue;
		}

		if (*it == "xx") {
			if (declarer == -1 || (it - a.begin()) % 2 != declarer % 2
					|| doubleredouble == 0) {
				return STRING_ERROR_AUCTION_TAG_INVALID_REDOUBLE_CALL;
			}
			doubleredouble = 2;
			continue;
		}

		if (it->length() < 2 || it->length() > 3) {
			return AuctionTagParseResult(STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
					*ito);
		}

		c = (*it)[0];
		if (!isdigit(c)) {
			return AuctionTagParseResult(STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
					*ito);
		}

		if (it->length() == 3 && (*it)[2] != 't') {
			return AuctionTagParseResult(STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
					*ito);
		}

		ncontract = c - '0';
		if (ncontract > 7) {
			return AuctionTagParseResult(STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
					*ito);
		}

		p = strchr(SO, (*it)[1]);
		if (p == NULL) {
			return AuctionTagParseResult(STRING_ERROR_AUCTION_TAG_INVALID_CALL_STRING,
					*ito);
		}
		ntrump = p - SO;

		if (ncontract < contract || (ncontract == contract && ntrump < trump)) {
			return AuctionTagParseResult(
					STRING_ERROR_AUCTION_TAG_FOUND_CALL_WHICH_IS_LOWER_THAN_PREVIOUS_ONE,
					*ito);
		}

		contract = ncontract;
		trump = ntrump;
		declarer = (it - a.begin()) % 4;
		doubleredouble = 0;
		if (firstDeclarer[declarer % 2][trump] == -1) {
			firstDeclarer[declarer % 2][trump] = declarer;
		}
	}

	//need 'trump' for firstDeclarer
	ntrump = INDEX_OF(SO[trump],SUITS_CHAR );
	assert(ntrump != -1);

	s = format("%d", contract) + getEnglishTrumpString(ntrump);
	for (i = 0; i < doubleredouble; i++) {
		s += 'X';
	}

	assert(auctionValue.length() == 1);
	p = strchr(PLAYER_CHAR, tolower(auctionValue[0]));
	assert(p!=NULL);

	return AuctionTagParseResult(STRING_OK, s,
			toupper(
					PLAYER_CHAR[(p - PLAYER_CHAR + firstDeclarer[declarer % 2][trump]) % 4]));
}

/*
std::string getLowerTrumpString(int trump){
	if(trump==NT){
		return getString(STRING_NT);
	}

	std::string s=getString(STRING_ID(STRING_SPADES+trump));
	return utf8Substring(s, 0, 1);
}

std::string getUpperTrumpString(int trump){
	if(trump==NT){
		return getNTString();
	}

	std::string s=getString(STRING_ID(STRING_SPADES+trump));
	s=utf8Substring(s, 0, 1);
	return utf8ToUpperCase(s);
}
*/

std::string getEnglishTrumpString(int trump) {
	std::string s;
	s += toupper(SUITS_CHAR[trump]);
	if (trump == NT) {
		s += 'T';
	}
	return s;
}

std::string rgbaToString(const GdkRGBA c) {
	/* Note use rgba not rgb colors because for 'scale' font color is changed for
	 * example black color became gray, seems wrong opacity float value of alpha
	 * channel rgba(0,0,0,1.00) is not recognized correctly by scale so use
	 * rgba(0,0,0,255)
	 *
	 * */
	return format("rgba(%d,%d,%d,%d)", int(c.red * 255), int(c.green * 255),
			int(c.blue * 255), int(c.alpha * 255));
}

unsigned rgbaToUnsigned(const GdkRGBA c){
	return unsigned(c.red * 255)<<24 | unsigned(c.green * 255)<<16 |
			unsigned(c.blue * 255)<<8 | unsigned(c.alpha * 255);
}

#ifndef FINAL_RELEASE
void exploreAllChildrenRecursive(GtkWidget* w) {
	std::string s, q;
	VString v;
	VStringCI it;
	const char*p;
	GList *children, *iter;
	children = gtk_container_get_children(GTK_CONTAINER(w));

	for (iter = children; iter != NULL; iter = g_list_next(iter)) {
		w = GTK_WIDGET(iter->data);
		println("%llx",w);
		printl(gtk_widget_path_to_string(gtk_widget_get_path(w)));

		if (GTK_IS_CONTAINER(w)) {
			GList* ch = gtk_container_get_children(GTK_CONTAINER(w));
			if (g_list_length(ch) > 7) {
			}
			g_list_free(ch);
			exploreAllChildrenRecursive(w);
		}
		else {
			s = gtk_widget_path_to_string(gtk_widget_get_path(w));
			v = split(s, " ");
			q = "";
			for (it = v.begin(); it != v.end(); it++) {
				p = strchr(it->c_str(), ':');
				assert(p);
				q += it->substr(0, p - it->c_str());
				q += " ";
			}
			if (strstr(s.c_str(), "list")) {
				println("%s", q.c_str());
			}
		}
	}
	g_list_free(children);

}
#endif

CARD_INDEX getPlayerForArray(CARD_INDEX player, const CARD_INDEX*a,
		int size, bool next, int count) {

	const bool inner = isInner(player);
	CARD_INDEX ci = player;
	if (inner) {
		ci = getOuter(player);
	}

	int i = indexOf(ci,a, size);

	assert(i != -1);

	i = (i + (next ? 1 : size - 1) * count) % size;

	ci = a[i];

	if (inner) {
		ci = getInner(ci);
	}

	return ci;

}

ESTIMATE getEstimateType() {
	return gconfig->getEstimateType();
}

int isBridgeSolveAllDealsAbsentNS(){
	return gconfig->m_bridgeSolveAllDealsAbsentNS;
}

void setBridgeSolveAllDealsAbsentNS(int ns){
	gconfig->m_bridgeSolveAllDealsAbsentNS=ns;
}

int indexOfPlayer(CARD_INDEX player) {
	assert(player >= CARD_INDEX_NORTH && player <= CARD_INDEX_WEST);
	return player - CARD_INDEX_NORTH;
}

std::string getSuitString(int suit) {
	return format("suit%d-%d.png", suit, SUIT_PIXBUF_SIZE);
}

std::string getContractString(int n) {
	return format("contract%d.png", n);
}

std::string getBgImageName(int i) {
	assert(i >= 0);
	return format("bg%d.jpg", i);
}

//using CRect so not aslov lib function
void copyFromPixbuf(GdkPixbuf* source, cairo_t * dest,
		CRect const& rect) {
	const int destx = rect.left;
	const int desty = rect.top;
	gdk_cairo_set_source_pixbuf(dest, source, destx, desty);
	cairo_rectangle(dest, destx, desty, rect.width(), rect.height());
	cairo_fill(dest);
}

CARD_INDEX getBridgePlayer(CARD_INDEX player, bool next,
		int count) {
	return getPlayerForArray(player, PLAYER, 4, next, count);
}

CARD_INDEX getNextBridgePlayer(CARD_INDEX player) {
	return getBridgePlayer(player, true);
}

CARD_INDEX getPreviousBridgePlayer(CARD_INDEX player) {
	return getBridgePlayer(player, false);
}

CARD_INDEX getBridgePartner(CARD_INDEX player) {
	return getBridgePlayer(player, true, 2);
}


BUTTON_STATE boolToButtonState(bool b){
	return b? BUTTON_STATE_ENABLED : BUTTON_STATE_DISABLED;
}

gint message(MESSAGE_ICON_TYPE iconType, const char* s,
		BUTTONS_DIALOG_TYPE type) {
	MessageDialog d(iconType, s, type);
	return d.getReturnCode();
}

gint message(MESSAGE_ICON_TYPE t, const std::string& s,
		BUTTONS_DIALOG_TYPE type) {
	return message(t, s.c_str(), type);
}

void message(MESSAGE_ICON_TYPE t, const char* s) {
	message(t, s, BUTTONS_DIALOG_OK);
}

void message(MESSAGE_ICON_TYPE t, const std::string& s) {
	message(t, s.c_str());
}

void message(MESSAGE_ICON_TYPE t, STRING_ID id,BUTTONS_DIALOG_TYPE type){
	message(t, getString(id),type);
}

bool selectColor(const char* s, GdkRGBA* color) {
	/* GLib-GIO-Message: gregistrysettingsbackend: error parsing key /org/gtk/settings/color-chooser/custom-colors: 76-99:number too big for any type
	 * warning appear even if just call GtkWidget *dialog = gtk_color_chooser_dialog_new (getString(MENU_CUSTOM_SKIN),GTK_WINDOW(getWidget()));
	 */
	GtkWidget *dialog = gtk_color_chooser_dialog_new(s,
			GTK_WINDOW(gframe->getWidget()));
	gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(dialog), color);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	bool r = result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY;
	if (r) {
		gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(dialog), color);
	}
	gtk_widget_destroy(dialog);
	return r;
}

bool selectFont(const char* s, PangoFontDescription*& font) {
	GtkWidget *dialog = gtk_font_chooser_dialog_new(s,
			GTK_WINDOW(gframe->getWidget()));
	gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(dialog), font);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	bool r = result == GTK_RESPONSE_OK || result == GTK_RESPONSE_APPLY;
	if (r) {
		font = gtk_font_chooser_get_font_desc(GTK_FONT_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);
	return r;
}

bool think(){
	return gdraw->think();
}

std::string getArrowFileName(int n,bool svg) {
	return format("arrow%d.%s", n,svg?SVG:PNG);
}

std::string getDeckFileName(int n,bool svg) {
	return format("deck%d.%s", n,svg?SVG:PNG);
}

std::string getDeckFileName() {
	return getDeckFileName(gconfig->m_deckNumber);
}

GdkPixbuf* getContractPixbuf(int n) {
	return pixbuf(getContractString(n));
}

GdkPixbuf* getSuitPixbuf(int suit, int size) {
	GdkPixbuf* p = pixbuf(getSuitString(suit));
	if (size != SUIT_PIXBUF_SIZE) {
		p = gdk_pixbuf_scale_simple(p, size, size, GDK_INTERP_BILINEAR);
	}
	return p;
}

std::string intToStringLocaled(int v){
	return toString(v,gconfig->m_thousandsSeparatorString[0]);
}

void unsignedToGdkRGBA(unsigned v, GdkRGBA &c) {
	c.alpha = (v & 0xff) / 255.;
	c.blue = ((v >> 8) & 0xff) / 255.;
	c.green = ((v >> 16) & 0xff) / 255.;
	c.red = ((v >> 24) & 0xff) / 255.;
}

GtkWidget* createMarkupLabel(std::string const& s,int maxChars/*=0*/){
	auto l = gtk_label_new(0);
	auto a = GTK_LABEL(l);
	gtk_label_set_markup(a, s.c_str());
	gtk_label_set_justify(a, GTK_JUSTIFY_FILL);
	gtk_label_set_line_wrap(a, TRUE);
	if(maxChars){
		gtk_label_set_max_width_chars(a, maxChars);
	}
	return l;
}

GtkWidget* createMarkupLabel(STRING_ID id, int maxChars/*=0*/) {
	std::string s=getString(id);
	return createMarkupLabel(s,maxChars);
}

GtkWidget* createBoldLabel(STRING_ID id){
	std::string s=getString(id);
	return createBoldLabel(s);
}

GtkWidget* createBoldLabel(std::string const& s){
	return createMarkupLabel("<b>"+s+"</b>");
}

GtkWidget* createUnderlinedLabel(STRING_ID id){
	std::string s=getString(id);
	return createUnderlinedLabel(s);
}

GtkWidget* createUnderlinedLabel(CARD_INDEX id){
	std::string s=getPlayerString(id);
	return createUnderlinedLabel(s);
}

GtkWidget* createUnderlinedLabel(std::string const& s){
	return createMarkupLabel("<u>"+s+"</u>");
}

GtkWidget* containerGetChild(GtkWidget* w,int n){
	assert(GTK_IS_CONTAINER(w));
	int i=0;
	GList *iter;
	GtkWidget*r=nullptr;
	GList *children = gtk_container_get_children(GTK_CONTAINER(w));
	for(iter = children; iter != NULL; iter = g_list_next(iter)){
		if(i==n){
			r=GTK_WIDGET(iter->data);//store before g_list_free
			break;
		}
		i++;
	}
	g_list_free(children);
	assert(r);
	return r;
}


std::string getPlayerString(const CARD_INDEX cid[52],CARD_INDEX player,bool includeInner/*=false*/){
	std::string s;
	int i,j;
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 13; ++j) {
			if (cid[i * 13 + j] == player || (includeInner && cid[i * 13 + j] == player + 4)) {	//sometimes can be ci+4 on very first state
				s += toupper(RANK[j]);
			}
		}
		if (i != 3) {
			s += '.';
		}
	}
	return s;
}

void showOpenFileError(){
	std::string s=getString(STRING_ERROR_COULD_NOT_OPEN_FILE_FOR_WRITING);
	message(MESSAGE_ICON_ERROR,s+".\n"+strerror(errno)+".");
}

void clearContainer(GtkWidget*container){
	auto children = gtk_container_get_children(GTK_CONTAINER(container));
	if (g_list_length(children) > 0) {
		for (auto iter = children; iter != NULL; iter = g_list_next(iter)) {
			gtk_container_remove(GTK_CONTAINER(container), GTK_WIDGET(iter->data));
		}
	}
	g_list_free(children);
}
