/*
 * Config.cpp
 *
 *       Created on: 08.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Config.h"
#include "Widget.h"
#include "aslov.h"

char LANGUAGE_EXTENSION[] = "lng";
const char PATH_FORBIDDEN_CHAR='|';
const char VERSION_SIGNATURE[] = "version";
const char RECENT_FILES_SIGNATURE[] = "recent files";
const char START_POSITION_SIGNATURE[] = "start position";
const char SUITSORDER_SIGNATURE[] = "suits order";
const char INNERCARDMARGIN_SIGNATURE[] = "inner card margin";
const char INDENTINSIDESUIT_SIGNATURE[] = "indent inside suit";
const char ESTIMATIONINDENT_SIGNATURE[] = "estimation indent";
const char SYSTEMVARIABLE_SIGNATURE[] = "system variables";
const char ALLOW_ONLY_ONE_INSTANCE_SIGNATURE[] = "allow only one instance";
const char SVGDECKPARAMETERS_SIGNATURE[] = "svg deck parameters";
const bool ALLOW_ONE_INSTANCE_DEFAULT_VALUE = true;
const char ENGLISH_THOUSANDS_SEPARATOR = ',';
const char CUSTOM_SKIN_BACKGROUND_COLOR_SIGNATURE[] = "custom skin background color";
const char SKIN_FONT_COLOR_SIGNATURE[] = "skin font color";
const char CUSTOM_SKIN_FONT_COLOR_SIGNATURE[] = "custom skin font color";
const char FONT_SIGNATURE[] = "font";

Config* gconfig;

const int Config::INDENT_INSIDE_SUIT[] = { 13, 13, 13, 13, 13, 17, 20,20 };
const int Config::ESTIMATION_INDENT[] = { 32, 28, 28, 26, 32, 42, 35,45 };

Config::Config() {
	GSList *formats;
	GSList* elem;
	GdkPixbufFormat*pf;
	GdkRectangle rect;
	char*p;
	CSize sz;

	gconfig = this;

	static_assert(N_RASTER_DECKS==SIZE(INDENT_INSIDE_SUIT));
	static_assert(N_RASTER_DECKS==SIZE(ESTIMATION_INDENT));

	GdkDisplay *display = gdk_display_get_default();
	GdkMonitor *monitor = gdk_display_get_monitor(display, 0);
	gdk_monitor_get_workarea(monitor, &rect);

	m_workareaRect = CRect(rect.x, rect.y, rect.x + rect.width,
			rect.y + rect.height);

#ifndef NDEBUG
	//standard notebook screen resolution, leave for debug
	//m_workareaRect=CRect(0,0,1366,768-40);
#endif

	//always show images for buttons
	g_object_set(gtk_settings_get_default(), "gtk-button-images", TRUE, NULL);

#ifndef FINAL_RELEASE
	//if no file do nothing, otherwise truncate
	f = openApplicationLog("r");
	if (f) {
		fclose(f);
		//truncate log file, after g_chdir
		f = openApplicationLog("w+");
		assert(f!=NULL);
		fclose(f);
	}
#endif

	m_options = { &m_showLastTrick,	//0
			&m_animation,
			&m_showToolTips,
			&m_showCommonTricks,
			&m_showPlayerTricks,
			&m_htmlShowDialog,
			&m_htmlStoreWithImages,
			&m_htmlPreview,
			&m_documentModifiedWarning,
			&m_autoPlaySequence,
			&m_splitEveryFile,
			&m_allowOnlyOneInstance };
	assert(m_options.size()==SIZE(CHECKED_MENU));

/*
	START_TIMER
	CSize maxCardSize;
	CSize rasterDeckCardSize[N_RASTER_DECKS];
	int rasterArrowSize[N_RASTER_ARROWS];
	int w,h;

	for (i = 0; i < N_RASTER_DECKS; i++) {
		sz=getPixbufSize(getDeckFileName(i));
		w=sz.cx/13;
		h=sz.cy/4;
		rasterDeckCardSize[i]={w,h};
		if (i == 0 || w * h > maxCardSize.cx * maxCardSize.cy) {//one deck has size 71x96, another one 72x96, so cann't use just height, use square of card
			maxCardSize = CSize(w, h);
		}
	}

	for (i = 0; i < N_RASTER_ARROWS; i++) {
		sz=getPixbufSize(getArrowFileName(i));
		rasterArrowSize[i]=sz.cx;
	}
	//0.54 seconds
	OUT_TIMER

	//counting of sizes take a long time, so use predefined arrays
	//leave this code if add some decks or arrows
	std::string s;
	i=0;
	s="const CSize RASTER_DECK_CARD_SIZE[]={";
	for(auto& q:rasterDeckCardSize){
		if(i){
			s+=",";
		}
		s+=format("{%d,%d}",q.cx,q.cy);
		i=1;
	}
	s+="};";
	println(s.c_str());

	i=0;
	s="const int RASTER_ARROW_SIZE[]={";
	for(auto& q:rasterArrowSize){
		if(i){
			s+=",";
		}
		s+=std::to_string(q);
		i=1;
	}
	s+="};";
	println(s.c_str());

	println("const CSize MAX_CARD_SIZE(%d,%d);",maxCardSize.cx,maxCardSize.cy)
*/


	initVarables();	//store variable to load/save
	load();

	//load language (needed variables was set in loadConfig)
	loadLanguageFile();

	loadCSS();

	formats = gdk_pixbuf_get_formats();
	for (elem = formats; elem; elem = elem->next) {
		pf = (GdkPixbufFormat*) elem->data;

		p = gdk_pixbuf_format_get_name(pf);

//#define SHOW_FORMATS
#ifdef SHOW_FORMATS
		std::string s=p;
		s+=' ';
		s+=gdk_pixbuf_format_get_description(pf);
#endif

		if (m_allImageFormatString.length() != 0) {
			m_allImageFormatString += " ";
		}
		m_allImageFormatString += p;

		if (gdk_pixbuf_format_is_writable(pf)) {
			m_storeImageFormat.push_back(p);
#ifdef SHOW_FORMATS
			s+=" writable";
#endif
			if (m_storeImageFormatString.length() != 0) {
				m_storeImageFormatString += " ";
			}
			m_storeImageFormatString += m_storeImageFormat.back();
		}
#ifdef SHOW_FORMATS
		println("%s",s.c_str())
#endif
	}
	g_slist_free(formats);

	Widget::staticInit();
	Problem::staticInit();
}

Config::~Config() {
	pango_font_description_free(m_font);
}

void Config::initVarables() {
	storeVariablesInt = {
			&m_activeCardShift,
			&m_allowOnlyOneInstance,
			&m_animation,
			&m_arrowMargin,
			&m_arrowNumber,
			&m_arrowSize,
			&m_ascending,
			&m_autoPlaySequence,
			&m_cardWidth,
			&m_cardHeight,
			&m_deckNumber,
			&m_documentModifiedWarning,
			&m_eastWestCardsMargin,
			(int*) &m_estimateType,
			(int*) &m_gameType,
			&m_htmlStoreWithImages,
			&m_htmlStoreBestMove,
			&m_htmlStoreNumberOfTricks,
			&m_htmlPreview,
			&m_htmlShowDialog,
			&m_indentBetweenSuits,
			&m_maxRecent,
			&m_maxRecentLength,
			&m_maxThreads,
			&m_resizeOnDeckChanged,
			&m_showCommonTricks,
			&m_splitEveryFile,
			&m_showLastTrick,
			&m_showPlayerTricks,
			&m_showToolTips,
			&m_firstSplitNumber,
			&m_lastTrickMinimalMargin,
			&m_frameDelta,
			(int*) &m_absent,
			&m_bridgeSolveAllFoeAbsentNS,
			&m_customSkinBackgroundIsColor,
			&m_skin,
	};

	storeVariablesIntNote = {
			"active card shift",
			ALLOW_ONLY_ONE_INSTANCE_SIGNATURE,
			"animation",
			"margin between arrow and borders of table",
			"arrow number",
			"arrow size",
			"ascending",
			"auto play sequence",
			"card width",
			"card height",
			"deck number",
			"document modified warning",
			"east west cards margin",
			"estimate type",
			"game type",
			"store html with images",
			"html store best move",
			"html store number of tricks",
			"html preview",
			"html show dialog",
			"indent between suits",
			"max recent files",
			"menu max recent file length",
			"max number of threads",
			"resize window on deck changed",
			"show common tricks",
			"split every file",
			"show last trick window",
			"show player tricks",
			"show tool tips",
			"first split number",
			"last trick minimal margin",
			"frame delta (system variable)",
			"absent player (preferans)",
			"bridge solve all foe absent north/south",
			"custom background is color",
			"skin",
	};
	assert(storeVariablesInt.size()==storeVariablesIntNote.size());

	storeVariablesString = { &m_version, &m_languageFileName, &m_customSkinBackgroundImagePath };
	storeVariablesStringNote = { VERSION_SIGNATURE, "language file", "custom skin background image path" };
	assert(storeVariablesString.size()==storeVariablesStringNote.size());
}

void Config::load() {
	std::string s;
	VString vs;
	VStringCI itString;
	GDir *dir;
	const gchar *filename;
	VMenuString v_language;
	const char*b;
	VStringI it;
	unsigned u;
	int i;

	//load m_language before possible reset() because reset() use setLanguageFileName(0);
	auto LS=getLanguageDir();
	dir = g_dir_open(LS.c_str(), 0, 0);
	assert(dir);
	while ((filename = g_dir_read_name(dir))) {
		b = strrchr(filename, '.');
		//skip subdirs, files without extension and file with not LANGUAGE_EXTENSION
		if (isDir(LS+G_DIR_SEPARATOR+filename) || b == NULL || !cmp(b + 1, LANGUAGE_EXTENSION)) {
			continue;
		}
		m_language.push_back(std::string(filename, b - filename));
	}
	it = find(std::string("english"),m_language );
	if (it != m_language.end()) {
		std::iter_swap(m_language.begin(), it);
	}
	//now first language is english

	//reset anyway to setup parameters which don't exist in cfg file
	reset();

	loadConfig(m_map);

//Since version 5.2 config file if moved to another directory so don't need check older versions
//	const double INVALID_VERSION=1000;
//	double readedVersion=INVALID_VERSION;
//	if(getStringBySignature(VERSION_SIGNATURE,s) ){
//		setlocale(LC_NUMERIC, "C");
//		readedVersion=std::stod(s);
//	}

	auto itStringPtr = storeVariablesString.begin();
	for(auto& signature:storeVariablesStringNote){
		if(getStringBySignature(signature,s)){
			*(*itStringPtr) = s;
		}
		itStringPtr++;
	}

#define LOAD_ARRAY(a,signature) loadIntArray(a,SIZEI(a),signature);
	LOAD_ARRAY(m_suitsOrder, SUITSORDER_SIGNATURE)
	LOAD_ARRAY(m_innerCardMargin, INNERCARDMARGIN_SIGNATURE)
	LOAD_ARRAY(m_indentInsideSuit, INDENTINSIDESUIT_SIGNATURE)
	LOAD_ARRAY(m_estimationIndent, ESTIMATIONINDENT_SIGNATURE)
#undef LOAD_ARRAY

	if(getStringBySignature(START_POSITION_SIGNATURE,s)){
		vs=split(s," ");
		if (vs.size() == 2) {
			m_startPosition.x = std::stoi(vs[0]);
			m_startPosition.y = std::stoi(vs[1]);
		}
	}

	auto itIntPtr = storeVariablesInt.begin();
	for(auto& signature:storeVariablesIntNote){
		if(getStringBySignature(signature,s)){
			*(*itIntPtr)=std::stoi(s);
		}
		itIntPtr++;
	}

	//load recent files
	if (getStringBySignature(RECENT_FILES_SIGNATURE, s) && !s.empty()) {
		m_recent = split(s, PATH_FORBIDDEN_CHAR);
	}

	if (getStringBySignature(FONT_SIGNATURE, s)) {
		m_font=pango_font_description_from_string(s.c_str());
	}
	if (getStringBySignature(CUSTOM_SKIN_BACKGROUND_COLOR_SIGNATURE, s) && parseString(s,u,16) ) {
		unsignedToGdkRGBA(u, m_customSkinBackgroundColor);
	}

	if (getStringBySignature(CUSTOM_SKIN_FONT_COLOR_SIGNATURE, s) && parseString(s,u,16) ) {
		unsignedToGdkRGBA(u, m_customSkinFontColor);
	}

	if (getStringBySignature(SKIN_FONT_COLOR_SIGNATURE, s) ) {
		vs=split(s," ");
		if(vs.size()==N_SKINS){
			i=0;
			for(auto a:vs){
				if(parseString(a,u,16)){
					unsignedToGdkRGBA(u, m_skinFontColor[i++]);
				}
			}
		}

	}
}

void Config::loadIntArray(int*a, int size, const char* signature) {
	int i;
	std::string s;
	if (!getStringBySignature(signature, s)){
#ifndef NDEBUG
		println("signature not found [%s]",signature)
#endif
		return;
	}
	VString vs = split(s, " ");
	if (int(vs.size()) != size) {
#ifndef NDEBUG
		println("invalid length %d %d",int(vs.size()), size)
#endif
		return;
	}
	for (i = 0; i < size; i++) {
		a[i] = std::stoi(vs[i]);
	}
}

void Config::save(GAME_TYPE gt,int x,int y) {
	VStringCI itString;
	VString v;

	m_gameType = gt;
	m_startPosition = CPoint(x, y);

	//at first setup first variable
	m_version = CURRENT_VERSION_STR;

#define S(a,b) f<<a<<" = "<<b<<"\n";

	std::ofstream f(getConfigPath());
	if(!f.is_open()){
		return;
	}

	itString = storeVariablesStringNote.begin();
	for (std::string *itStringPtr : storeVariablesString) {
		S( *itString++ , *itStringPtr);
	}

	S( SUITSORDER_SIGNATURE , JOIN(m_suitsOrder) )
	S( INNERCARDMARGIN_SIGNATURE , JOIN(m_innerCardMargin) )
	S( INDENTINSIDESUIT_SIGNATURE , JOIN(m_indentInsideSuit) )
	S( ESTIMATIONINDENT_SIGNATURE , JOIN(m_estimationIndent) )

	S(START_POSITION_SIGNATURE,forma(m_startPosition.x,m_startPosition.y));

	itString = storeVariablesIntNote.begin();
	for (int* itIntPtr : storeVariablesInt){
		S(*itString++,*itIntPtr);
	}

	//save recent files
	S(RECENT_FILES_SIGNATURE,joinV(m_recent,PATH_FORBIDDEN_CHAR));

	S(FONT_SIGNATURE,pango_font_description_to_string(m_font));
	S(CUSTOM_SKIN_BACKGROUND_COLOR_SIGNATURE,format("%x",rgbaToUnsigned(m_customSkinBackgroundColor)));
	S(CUSTOM_SKIN_FONT_COLOR_SIGNATURE,format("%x",rgbaToUnsigned(m_customSkinFontColor)));

	//v.clear();
	for(auto a:m_skinFontColor){
		v.push_back(format("%x", rgbaToUnsigned(a)));
	}
	S(SKIN_FONT_COLOR_SIGNATURE,joinV(v));

#undef S
}

void Config::reset() {
	int i;

	//order is same with loadConfig & saveConfig & declarations in Frame.h.
	//It's more convenient
	m_font = pango_font_description_from_string("Times New Roman, 14");
	setLanguageFileName(0);

	m_recent.clear();

	for (i = 0; i < 4; i++) {
		m_suitsOrder[i] = i;
	}

#define M(a,b) for(i=0;i<SIZEI(a);i++){a[i]=b[i];}
	M(m_innerCardMargin,INNER_CARD_MARGIN )
	M(m_indentInsideSuit,INDENT_INSIDE_SUIT )
	M(m_estimationIndent,ESTIMATION_INDENT )
#undef M

	m_startPosition = m_workareaRect.topLeft();

	m_activeCardShift = 10;
	m_allowOnlyOneInstance = ALLOW_ONE_INSTANCE_DEFAULT_VALUE;
	m_animation = 1;
	m_arrowMargin = 2;
	m_arrowNumber=0;
	//m_arrowSize = 0;//set in problem selector
	m_ascending = 0;
	m_autoPlaySequence = 1;
	//m_cardWidth=0;//set in problem selector
	//m_cardHeight=0;//set in problem selector
	m_deckNumber = 0;
	m_documentModifiedWarning = 1;
	m_eastWestCardsMargin = 3;
	m_estimateType = ESTIMATE_ALL_TOTAL;
	m_gameType = BRIDGE;
	m_htmlStoreWithImages = 1;
	m_htmlStoreBestMove = 0;
	m_htmlStoreNumberOfTricks = 0;
	m_htmlPreview = 1;
	m_htmlShowDialog = 1;
	m_indentBetweenSuits = 27;
	m_maxRecent = 8;
	m_maxRecentLength = 40;
	m_maxThreads = getNumberOfCores();
	m_resizeOnDeckChanged = 1;
	m_showCommonTricks = 1;
	m_splitEveryFile = 1;
	m_showLastTrick = 1;
	m_showPlayerTricks = 1;
	m_showToolTips = 1;
	m_skin = 0;
	m_firstSplitNumber = 0;
	m_lastTrickMinimalMargin = 0;
	m_frameDelta = 120;	//got from real measurement
	m_absent = CARD_INDEX_SOUTH;
	m_bridgeSolveAllFoeAbsentNS=0;
	m_thousandsSeparatorString=ENGLISH_THOUSANDS_SEPARATOR;

	//m_skin, m_font - is set
	m_customSkinBackgroundColor={232/255.,232/255.,232/255.,1};
	m_customSkinBackgroundImagePath="";
	const GdkRGBA black{0,0,0,1};
	const GdkRGBA white{1,1,1,1};
	m_customSkinFontColor=black;
	for(i=0;i<N_SKINS;i++){
		m_skinFontColor[i]=i>=2 && i<=5 ? white:black;
	}
	m_customSkinBackgroundIsColor=1;
}

void Config::setLanguageFileName(int index) {
	m_languageFileName = getLanguageFileNameByIndex(index);
}

std::string Config::getLanguageDir()const{
	return getResourcePath("lng");
}

std::string Config::getLanguageFileNameByIndex(int index) const {
	assert(index >= 0 && index < int(m_language.size()));
	return getLanguageDir()+ G_DIR_SEPARATOR+m_language[index]+"."+LANGUAGE_EXTENSION;
}

int Config::getLanguageIndex() const {
	std::size_t pos = m_languageFileName.rfind(G_DIR_SEPARATOR);
	assert(pos != std::string::npos);
	std::string s = m_languageFileName.substr(pos + 1,
			m_languageFileName.length() - pos - 5);

	return indexOf(s,m_language);
}

/* load language file to vector
 * if v.first=id exists replace string
 */
bool Config::loadLanguage(std::string filename,VIntString& v){
	const int BUFF_SIZE = 512;
	char buff[BUFF_SIZE], *b, *e;
	int i,previous=0;
	FILE*f;

	f = open(filename.c_str(), "r");
	if (!f) {
		return false;
	}

	while (fgets(buff, BUFF_SIZE, f)) {
		for(b=buff;*b=='\t' || *b==' ';b++);
		if(*b=='"'){
			i=++previous;
		}
		else{
			i = strtol(buff, &b, 10);
			if (b == buff) {	//no digits found
				continue;
			}
			previous=i;
			b = strchr(buff, '"');
		}
		e = strrchr(buff, '"');
		if (b != 0 && e != 0) {
			b++;
			*e = 0;
			auto it = find_if(v.begin(), v.end(), [&i](const PairIntString &a) {
				return a.first == i;
			});
			if(it==v.end()){
				v.push_back({i,b});
			}
			else{
				it->second=b;
			}
		}
	}
	fclose(f);
	return true;
}

void Config::loadLanguageFile() {
	int i, j;
	VIntString v;
	auto se = "english"+std::string(".")+LANGUAGE_EXTENSION;
	auto sr = "russian"+std::string(".")+LANGUAGE_EXTENSION;

	//auto s = getLanguageDir()+G_DIR_SEPARATOR+"english."+LANGUAGE_EXTENSION;
	auto s = getLanguageDir()+format("%cenglish.%s",G_DIR_SEPARATOR, LANGUAGE_EXTENSION);
	//auto s = format("%s%cenglish.%s", m_languageDir,G_DIR_SEPARATOR, LANGUAGE_EXTENSION);
	i=0;
	if(!endsWith(m_languageFileName,se) && !endsWith(m_languageFileName,sr) ){
		i=1;
		loadLanguage(s,v);
	}

	//if project moved to another folder, and language file location is wrong
	if(!loadLanguage(m_languageFileName,v) && i==0){
		printl("CRITICAL ERROR couldn't open language file, use english language file");
		m_languageFileName=s;
		loadLanguage(s,v);
	}

	m_vectorMenuString.clear();
	for (auto& a : v) {
		i = a.first;
		auto& b=a.second;

		if (i < STRING_SIZE) {	//load normal strings & errors not! STRING_SIZE

			if (i == STRING_ABOUT) {
				S[i] = format(b.c_str(), CURRENT_VERSION_STR.c_str(), EMAIL.c_str(),
						BASE_ADDRESS.substr(0, BASE_ADDRESS.length() - 1).c_str());
			}
			else {
				S[i] = b;
			}
			S[i] = localeToUtf8(replaceAll(S[i], "\\n", "\n"));
		}
		else {
			//insert before
			if (i == MENU_LOAD_LANGUAGE_FILE) {
				j = 0;
				for (auto& l : m_language) {
					m_vectorMenuString.push_back( {
							MENU_ID(MENU_LANGUAGE_FIRST + j),
							localeToUtf8(l) });
					j++;
				}
			}
			else if (i == MENU_CALCULATOR) {
				m_vectorMenuString.push_back(MenuString(MENU_GAME_TYPE, ""));
			}

			m_vectorMenuString.push_back({MENU_ID(i), localeToUtf8(b)});

			//insert after
			if (i == MENU_REDOALL) {
				m_vectorMenuString.push_back({MENU_FIND_BEST_MOVE, ""});
			}

		}

	}

	m_thousandsSeparatorString=S[STRING_THOUSANDS_SEPARATOR];
}

int Config::getFontHeight(const PangoFontDescription*desc) {
	if (pango_font_description_get_size_is_absolute(desc)) {//this type of fonts is not checked
		assert(0);
		//see pango_font_description_set_size () help
		//return pango_font_description_get_size(desc)*96/72/PANGO_SCALE;//may be like this
		return pango_font_description_get_size(desc) * 72 / 96 / PANGO_SCALE;
	}
	else {
		return pango_font_description_get_size(desc) / PANGO_SCALE;
	}
}

void Config::loadCSS(){

	std::string p,t;
	PangoStyle ps = pango_font_description_get_style(m_font);
	switch (ps) {
	case PANGO_STYLE_NORMAL:
		t = "normal";
		break;
	case PANGO_STYLE_OBLIQUE:
		t = "oblique";
		break;
	case PANGO_STYLE_ITALIC:
		t = "italic";
		break;
	}

	auto fc=rgbaToString(m_skin==CONFIG_CUSTOM_SKIN ?m_customSkinFontColor : m_skinFontColor[m_skin]);
	bool b=true;
	if(m_skin==CONFIG_CUSTOM_SKIN){
		if(m_customSkinBackgroundIsColor){
			p="background:"+rgbaToString(m_customSkinBackgroundColor);
			b=false;
		}
		else{
			p=m_customSkinBackgroundImagePath;
		}
	}
	else{
		p="bridge/images/bg"+std::to_string(m_skin)+".jpg";
	}
	if(b){
		p="background-image:url('"+p+"')";
	}

	std::string s ="@define-color font_color "+fc+";"
			+"GtkDialog,dialog,notebook stack{"+p+";}"
			+"textview, entry, label, progressbar, scale{"+
			+"font-size:"+std::to_string(getFontHeight(m_font))+"pt;"
			+"font-family:"+pango_font_description_get_family(m_font)+";"
			+"font-style:"+t+";"
			+"font-weight:"+std::to_string(int(pango_font_description_get_weight(m_font)))+";"
			+"}";
	//printl(s)

	::loadCSS(s);
}

void Config::updateCSS(){
	loadCSS();
}

void Config::setSkin(int skin) {
	m_skin = skin;
	updateCSS();
}

PangoFontDescription* Config::getFont(int height) const {
	char*p = pango_font_description_to_string(m_font);
	std::string s(p);
	g_free(p);
	int h = getFontHeight(m_font);
	if (h != height) {
		s = replaceAll(s, format("%d", h), format("%d", height));
	}
	return pango_font_description_from_string(s.c_str());
}

const gchar* Config::getPlayerString(CARD_INDEX player) const {
	int i = indexOfPlayer(player);
	return getString(static_cast<STRING_ID>(STRING_NORTH + i));
}

GdkPixbuf* Config::languagePixbuf(int id) const {
	return pixbuf(m_language[id - MENU_LANGUAGE_FIRST] + ".png");
}

std::string Config::getTitle() {
	std::string s = getString(STRING_ABOUT);
	auto p=s.find('\n');
	for(int i=0;i<2;i++){
		p=s.rfind(' ',p-1);
	}
	s = s.substr(0, p);
	printzi("[",s,"]")
#ifndef FINAL_RELEASE
	s += " FINAL_RELEASE not defined";
#endif
#ifndef NDEBUG
	s += " NDEBUG not defined";
#endif
	return s;
}

std::string Config::getUniqueApplicationName() {
	std::string s = BASE_ADDRESS.substr(0, BASE_ADDRESS.rfind(SLASH));
	s = s.substr(s.rfind(SLASH) + 1);
	const std::string SEPARATOR = ".";
	VString vs = split(s, SEPARATOR);
	VString::reverse_iterator it;
	for (s = "", it = vs.rbegin(); it != vs.rend(); it++) {
		if (it != vs.rbegin()) {
			s += SEPARATOR;
		}
		s += *it;
	}
	return s;
}

bool Config::allowOnlyOneInstance() {
	MapStringString m;
	MapStringString::iterator it;
	if (loadConfig(m)
			&& (it = m.find(ALLOW_ONLY_ONE_INSTANCE_SIGNATURE)) != m.end()) {
		return std::stoi(it->second) != 0;
	}

	return ALLOW_ONE_INSTANCE_DEFAULT_VALUE;
}

bool Config::getStringBySignature(const char*signature,std::string& s){
	return getStringBySignature(std::string(signature),s);
}

bool Config::getStringBySignature(const std::string&signature,std::string& s){
	auto it=m_map.find(signature);
	if(it==m_map.end()){
		//println("signature not found [%s]",signature.c_str())
		return false;
	}
	else{
		s=it->second;
		return true;
	}
}

int Config::getIndentInsideSuit() const {
	return isScalableDeck()? getSvgIndentInsideSuit() : m_indentInsideSuit[m_deckNumber];
}

int Config::getEstimationIndent() const {
	return isScalableDeck()? getSvgEstimationIndent() :m_estimationIndent[m_deckNumber];
}

bool Config::isScalableArrow(int arrow){
	return arrow>=N_RASTER_ARROWS;
}

bool Config::isScalableArrow()const{
	return isScalableArrow(m_arrowNumber);
}

bool Config::isScalableDeck(int deck){
	return deck>=N_RASTER_DECKS;
}

bool Config::isScalableDeck()const{
	return isScalableDeck(m_deckNumber);
}

CSize Config::getCardSize()const{
	return {m_cardWidth,m_cardHeight};
}

int& Config::getCardWidth(){
	return m_cardWidth;
}

int& Config::getCardHeight(){
	return m_cardHeight;
}

int Config::getSvgIndentInsideSuit()const{
	//got coefficient manually
	return m_cardWidth/4.5;
}

int Config::getSvgEstimationIndent()const{
	//got coefficient manually
	return m_cardHeight/3.5;
}

void Config::setArrowParameters(int arrow,int arrowSize){
	m_arrowNumber=arrow;
	m_arrowSize=arrowSize;
}

void Config::setDeckParameters(int deck,bool resizeOnDeckChanged,CSize cardSize){
	m_deckNumber=deck;
	m_resizeOnDeckChanged=resizeOnDeckChanged;
	m_cardWidth=cardSize.cx;
	m_cardHeight=cardSize.cy;
}

int Config::recentSize(){
	return m_recent.size();
}

bool Config::isWritableImage(std::string const& s) const {
	return oneOf(s,m_storeImageFormat);
}

GdkRGBA& Config::getFontColor(){
	return m_skinFontColor[m_skin];
}
