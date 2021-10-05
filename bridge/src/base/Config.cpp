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

char LANGUAGE_EXTENSION[] = "lng";
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

Config* gconfig;

const int Config::INDENT_INSIDE_SUIT[] = { 13, 13, 13, 13, 13, 17, 20,20 };
const int Config::ESTIMATION_INDENT[] = { 32, 28, 28, 26, 32, 42, 35,45 };

Config::Config(const char *argv0) {
	int i;
	GSList *formats;
	GSList* elem;
	GdkPixbufFormat*pf;
	GdkRectangle rect;
	FILE*f;
	const int size = 1024;
	char b[size];
	char*p, *q;
	GtkStyleContext *context;
	GtkWidgetPath *path;
	CSize sz;

	gconfig = this;

	static_assert(N_RASTER_DECKS==SIZE(INDENT_INSIDE_SUIT));
	static_assert(N_RASTER_DECKS==SIZE(ESTIMATION_INDENT));
	m_exePath = argv0;//TODO
	m_path = getBasePath(argv0);
	sprintf(m_cfgFilePath, getCfgPath(m_path).c_str());
	sprintf(m_languageDir, "%s",getResourcePath("lng").c_str() );
	sprintf(m_imagePath, "%simages%c", m_path.c_str(), G_DIR_SEPARATOR);

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

	loadCss();

	//load global css variables
	f = open(getCssFilePath(), "r");
	assert(f);
	while (fgets(b, size, f) != NULL) {
		if (strstr(b, "@import") != NULL && (p = strstr(b, "bridge")) != NULL) {
			p += strlen("bridge");
			m_skin = atoi(p);
			break;
		}
	}
	fclose(f);

	/* takes "label" font from css
	 *
	 * this way works for "GtkLabel" in css
	 * path = gtk_widget_path_new ();
	 * gtk_widget_path_append_type (path, GTK_TYPE_LABEL);
	 * context=gtk_style_context_new();
	 * gtk_style_context_set_path(context, path);
	 *
	 */
	path = gtk_widget_get_path(gtk_label_new(""));
	context = gtk_style_context_new();
	gtk_style_context_set_path(context, path);
	gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL, GTK_STYLE_PROPERTY_FONT,
			&m_font, NULL);

	/* if user select 'custom skin' menu option need to setup m_customBackgroundColor for color selection dialog
	 * m_customBackgroundColor variable is in only one file so it can be managed as global variables m_font & m_skin
	 */
	f = open(getCssFilePath(CONFIG_CUSTOM_SKIN), "r");
	assert(f);
	while (fgets(b, size, f) != NULL) {
		if ((p = strstr(b, "background")) != NULL) {
			p = strchr(p, ':');
			assert(p);
			p++;
			q = strchr(p, ';');
			assert(q);
			*q = 0;
			i = gdk_rgba_parse(&m_customBackgroundColor, p);
			if (!i) {	//if error background-image is set
				m_customBackgroundColor.red = m_customBackgroundColor.green = m_customBackgroundColor.blue = 0;
				m_customBackgroundColor.alpha = 1;
			}
			break;
		}
	}
	fclose(f);

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
	};
	assert(storeVariablesInt.size()==storeVariablesIntNote.size());

	storeVariablesString = { &m_version, &m_languageFileName };
	storeVariablesStringNote = { "version", "language file" };
	assert(storeVariablesString.size()==storeVariablesStringNote.size());
}

void Config::load() {
	std::string s;
	VString vs;
	VStringCI itString;
	FILE*f;
	GDir *dir;
	const gchar *filename;
	gchar name[PATH_MAX + 2];	//+2 remove gcc warning 5.02
	VMenuString v_language;
	const char*b;
	VStringI it;
	char bu[MAX_BUFF];

	//load m_language before possible reset() because reset() use setLanguageFileName(0);
	dir = g_dir_open(m_languageDir, 0, 0);
	assert(dir);
	while ((filename = g_dir_read_name(dir))) {
		sprintf(name, "%s%c%s", m_languageDir, G_DIR_SEPARATOR, filename);
		b = strrchr(filename, '.');
		//skip subdirs, files without extension and file with not LANGUAGE_EXTENSION
		if (isDir(name) || b == NULL || !cmp(b + 1, LANGUAGE_EXTENSION)) {
			continue;
		}
		m_language.push_back(std::string(filename, b - filename));
	}
	it = find(m_language, std::string("english"));
	if (it != m_language.end()) {
		std::iter_swap(m_language.begin(), it);
	}
	//now first language is english

	//reset anyway to setup parameters which don't exist in cfg file
	reset();

	f = open(m_cfgFilePath, "r+");
	if (!f) {
		return;
	}

	//order of strings in file is not important
	while (fgets(bu, MAX_BUFF, f) ) {
		m_map.insert(pairFromBuffer(bu));
	}
	fclose(f);

	/* for version 5.2 "deck" -> "deck number",
	 * "big arrow" -> "arrow number" big arrow=1 means arrow number=0 and
	 * big arrow=0 means arrow number=1
	 */
	if(getStringBySignature("version",s) && std::stod(s)<5.2 ){
		if (getStringBySignature("deck", s)) {
			m_map.insert( { "deck number", s });
		}
		if (getStringBySignature("big arrow", s)) {
			m_map.insert( { "arrow number", s == "0" ? "1" : "0" });
		}
	}


	auto itStringPtr = storeVariablesString.begin();
	for(auto& signature:storeVariablesStringNote){
		if(getStringBySignature(signature,s)){
			*(*itStringPtr) = localeToUtf8(s);
		}
		itStringPtr++;
	}

#define LOAD_ARRAY(a,signature) loadIntArray(f,a,SIZEI(a),signature);
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
		vs = split(s, G_SEARCHPATH_SEPARATOR_S);
		for (itString = vs.begin(); itString != vs.end(); itString++) {
			m_recent.push_back(localeToUtf8(*itString));
		}
	}

}

void Config::loadIntArray(FILE*f, int*a, int size, const char* signature) {
	int i;
	std::string s;
	if (!getStringBySignature(signature, s)){
		println("signature not found [%s]",signature)
		return;
	}
	VString vs = split(s, " ");
	if (int(vs.size()) != size) {
		println("invalid length %d %d",int(vs.size()), size)
		return;
	}
	for (i = 0; i < size; i++) {
		a[i] = std::stoi(vs[i]);
	}
}

void Config::save(GAME_TYPE gt,int x,int y) {
	int i;
	VStringI itString;

	m_gameType = gt;
	m_startPosition = CPoint(x, y);

	FILE* f = open(m_cfgFilePath, "w+");
	assert(f!=NULL);
	if (!f) {
		return;
	}

	//at first setup first variable
	m_version = CURRENT_VERSION_STR;

	itString = storeVariablesStringNote.begin();
	for (std::string* itStringPtr : storeVariablesString){
		fprintf(f, "%s = %s\n", itString->c_str(), utf8ToLocale(*itStringPtr).c_str());
		itString++;
	}

#define SAVE_ARRAY(a,signature) fprintf(f,"%s =",signature);for(i=0;i<SIZEI(a);i++){fprintf(f," %d",a[i]);}fprintf(f,"\n");
	SAVE_ARRAY(m_suitsOrder, SUITSORDER_SIGNATURE)
	SAVE_ARRAY(m_innerCardMargin, INNERCARDMARGIN_SIGNATURE)
	SAVE_ARRAY(m_indentInsideSuit, INDENTINSIDESUIT_SIGNATURE)
	SAVE_ARRAY(m_estimationIndent, ESTIMATIONINDENT_SIGNATURE)
#undef SAVE_ARRAY

	fprintf(f, "%s = %d %d\n", START_POSITION_SIGNATURE, m_startPosition.x,
			m_startPosition.y);

	itString = storeVariablesIntNote.begin();
	for (int* itIntPtr : storeVariablesInt){
		fprintf(f, "%s = %d\n", itString->c_str(), *itIntPtr );
		itString++;
	}

	//save recent files
	fprintf(f, "%s = ", RECENT_FILES_SIGNATURE);
	i=0;
	for (auto s : m_recent) {
		if (i) {
			fprintf(f, G_SEARCHPATH_SEPARATOR_S);
		}
		fprintf(f, "%s", utf8ToLocale(s).c_str());
		i=1;
	}
	fprintf(f, "\n");

	fclose(f);

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

}

void Config::setLanguageFileName(int index) {
	m_languageFileName = getLanguageFileNameByIndex(index);
}

std::string Config::getLanguageFileNameByIndex(int index) const {
	assert(index >= 0 && index < int(m_language.size()));
	return format("%s%c%s.%s", m_languageDir, G_DIR_SEPARATOR,
			m_language[index].c_str(), LANGUAGE_EXTENSION);
}

int Config::getLanguageIndex() const {
	std::size_t pos = m_languageFileName.rfind(G_DIR_SEPARATOR);
	assert(pos != std::string::npos);
	std::string s = m_languageFileName.substr(pos + 1,
			m_languageFileName.length() - pos - 5);

	return indexOf(m_language, s);
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
	if (!f ) {
		return false;
	}

	while (fgets(buff, BUFF_SIZE, f)) {
		for(b=buff;*b=='\t' || *b==' ';b++);
		if(*b=='"'){
			i=++previous;
		}
		else{
			i = int(strtol(buff, &b, 10));
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
			auto it = find_if(v.begin(), v.end(), [&i] (const PairIntString& a) { return a.first == i; } );
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

	auto s = format("%s%cenglish.%s", m_languageDir,G_DIR_SEPARATOR, LANGUAGE_EXTENSION);
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
			if (i == MENU_OTHER_LANGUAGE) {
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

void Config::writeAndLoadCss(REWRITE_CSS_OPTION o) {
	const int size = 1024;
	char b[size], c[size];
	char *p1, *p2;
	VString v;
	VStringI it;
	FILE*f;
	std::string s;

	if (o & REWRITE_CSS_MAIN_FILE) {
		s = getCssFilePath();
		v = readFileToVString(s);

		f = open(s, "w+");
		for (it = v.begin(); it != v.end(); it++) {
			sprintf(b, "%s", it->c_str());
			if (strstr(b, "@import") && strstr(b, "bridge")) {
				sprintf(b, "@import url(\"bridge%d.css\");\n", m_skin);
			}
			else if (strstr(b, "##1") != NULL) {
				p1 = strchr(b, ':');
				p2 = strchr(b, ';');
				assert(p1!=NULL);
				assert(p2!=NULL);

				strncpy(c, p1 + 1, p2 - p1);
				c[p2 - p1 - 1] = 0;
				//change font
				*(p1 + 1) = 0;

				if (strstr(b, "font-size")) {
					s = format("%dpt", getFontHeight(m_font));
				}
				else if (strstr(b, "font-family")) {
					s = pango_font_description_get_family(m_font);
				}
				else if (strstr(b, "font-weight")) {
					s = format("%d", int(pango_font_description_get_weight(m_font)));
				}
				else if (strstr(b, "font-style")) {
					PangoStyle ps = pango_font_description_get_style(m_font);
					switch (ps) {
					case PANGO_STYLE_NORMAL:
						s = "normal";
						break;
					case PANGO_STYLE_OBLIQUE:
						s = "oblique";
						break;
					case PANGO_STYLE_ITALIC:
						s = "italic";
						break;
					}
				}
				else {
					assert(0);
				}

				sprintf(c, "%s%s%s", b, s.c_str(), p2);
				sprintf(b, c);
			}
			fprintf(f, b);

		}

		fclose(f);
	}

	if ((o & REWRITE_CSS_CUSTOM_FILE_FONT_COLOR)
			|| (o & REWRITE_CSS_CUSTOM_FILE_BACKGROUND_COLOR)
			|| (o & REWRITE_CSS_CUSTOM_FILE_BACKGROUND_IMAGE)) {
		s = getCssFilePath(m_skin);
		v = readFileToVString(s);

		const char DEFINE_COLOR[]="@define-color";
		const char GTK_DIALOG_DIALOG[]="GtkDialog,dialog";

		f = open(s, "w+");
		std::string a;
		for(auto&s:v){
			if(startsWith(s,DEFINE_COLOR)){
				if (o & REWRITE_CSS_CUSTOM_FILE_FONT_COLOR) {
					s="@define-color font_color "+rgbaToString(m_fontColor)+";\n";
				}
			}
			else if(startsWith(s,GTK_DIALOG_DIALOG)){
				if ((o & REWRITE_CSS_CUSTOM_FILE_BACKGROUND_COLOR)
						|| (o & REWRITE_CSS_CUSTOM_FILE_BACKGROUND_IMAGE)) {
					assert(m_skin == CONFIG_CUSTOM_SKIN);
					s="GtkDialog,dialog{";
					if (o & REWRITE_CSS_CUSTOM_FILE_BACKGROUND_COLOR) {
						s+="background:"+rgbaToString(m_customBackgroundColor);
					}
					else {
						s+="background-image:url('"+m_customBackgroundImage+"')";
					}
					s+=";}";
				}

			}
			fprintf(f, s.c_str());
		}
		fclose(f);
	}

	loadCss();
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

void Config::loadCss() {
	GtkCssProvider *provider;
	GdkScreen *screen;
	GtkStyleContext *context;
	GtkWidgetPath *path;
	GdkDisplay* display;

	display = gdk_display_get_default();
	screen = gdk_display_get_default_screen(display);

	provider = gtk_css_provider_new();
	gtk_style_context_add_provider_for_screen(screen,
			GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	gtk_css_provider_load_from_path(provider,
			g_filename_to_utf8(getCssFilePath().c_str(), getCssFilePath().length(),
			NULL, NULL, NULL), NULL);
	g_object_unref(provider);

	//set font color
	path = gtk_widget_path_new();
	gtk_widget_path_append_type(path, GTK_TYPE_LABEL);
	context = gtk_style_context_new();
	gtk_style_context_set_path(context, path);
	gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, &m_fontColor);

}

std::string Config::getCssFilePath(int skin /*= -2*/) {
	std::string s = getWorkingDirectory()+G_DIR_SEPARATOR+"bridge";
	if (skin != -2) {
		s += std::to_string(skin);
	}
	return s + ".css";
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

std::string Config::getTitle() {
	std::string s = getString(STRING_ABOUT);
	s = s.substr(0, s.find('\n'));
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

std::string Config::getBasePath(const char* argv0) {
	std::string s = argv0;
	std::size_t pos = s.rfind(G_DIR_SEPARATOR);
	//if pos====std::string::npos nothing to do already in working directory
	if (pos == std::string::npos) {
		s = "";	//base_path="" need to run from bat files without absolute path "bridge.exe 1.bts"
	}
	else {
		s = s.substr(0, pos + 1);		//last symbol G_DIR_SEPARATOR
	}

#ifndef FINAL_RELEASE
	/* run under eclipse remove last Release or Debug directory
	 * argv0="D:\\slovesno\\MyProjects\\eclipse\\bridge_cpp\\bridge\\Release\\bridge.exe"
	 * s="D:\\slovesno\\MyProjects\\eclipse\\bridge_cpp\\bridge\\Release\\"
	 */
	pos = s.rfind(G_DIR_SEPARATOR, s.length() - 2);
	assert(pos != std::string::npos);
	s = s.substr(0, pos + 1);

	// s="D:\\slovesno\\MyProjects\\eclipse\\bridge_cpp\\bridge\\"
#endif
	return localeToUtf8(s);
}

PairStringString Config::pairFromBuffer(const char*b){
	char*w=strchr(b, '=');
	if(!w){
		return {"",""};
	}
	const char*p = w + 2;
	char*f;
	const char *search;
	const char s[] = "\r\n";
	for (search = s; *search != '\0'; search++) {
		f = strrchr(p, *search);
		if (f != NULL) {
			*f = 0;
		}
	}
	if(w>b && w[-1]==' '){
		w--;
	}
	return {std::string(b,w-b),std::string(p)};
}

bool Config::allowOnlyOneInstance(const char* argv0) {
	char b[MAX_BUFF];
	std::string s = getCfgPath(getBasePath(argv0));
	FILE*f = fopen(s.c_str(), "r");

	if (f == NULL) {
		return ALLOW_ONE_INSTANCE_DEFAULT_VALUE;
	}

	while (fgets(b, MAX_BUFF, f) != NULL) {
		if (startsWith(b, ALLOW_ONLY_ONE_INSTANCE_SIGNATURE)) {
			s = pairFromBuffer(b).second;
			fclose(f);
			return atoi(s.c_str()) != 0;
		}
	}
	fclose(f);
	return ALLOW_ONE_INSTANCE_DEFAULT_VALUE;
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
	return oneOf(m_storeImageFormat, s);
}
