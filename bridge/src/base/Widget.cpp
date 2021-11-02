/*
 * Widget.cpp
 *
 *       Created on: 14.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Widget.h"
#include "../Frame.h"

const GdkRGBA BLACK_COLOR = { 0., 0., 0., 1. };
const int ARROW_K_IN_AREA_HEIGHT=2;

const STRING_ID ALL_SUPPORTED[] = {
		STRING_FILE_FILTER_ALL_SUPPORTED,
		STRING_FILE_FILTER_BTS_PTS,
		STRING_FILE_FILTER_PBN,
		STRING_FILE_FILTER_DF,
		STRING_FILE_FILTER_HTML,
		STRING_FILE_FILTER_ALL //should goes last
};
const int ALL_SUPPORTED_SIZE = SIZEI(ALL_SUPPORTED);

const STRING_ID STRING_FILTER_EXT_FROM[] = {
		STRING_FILE_FILTER_BTS_PTS,
		STRING_FILE_FILTER_PBN,
		STRING_FILE_FILTER_DF,
		STRING_FILE_FILTER_HTML,
		STRING_FILE_FILTER_LANGUAGE,
		STRING_IMAGE_FILES,
		STRING_FILE_FILTER_ALL_SUPPORTED //should goes after all simple filters, see 'if(option==CHOOSER_OPTION_DEFAULT){' in Widget::fileChooser()
		,
		STRING_FILE_FILTER_ALL //should goes last
};

const char* STRING_FILTER_EXT_TO[] = {
		"bts pts",
		"pbn",
		"txt dat",
		"htm html",
		"lng",
		NULL //STRING_IMAGE_FILES filled in Widget::staticInit()
		,
		"bts pts pbn txt dat" //Note in case of 'save' command two strings added "htm", "html", in case of 'open' not need
		,
		"*" };
const int STRING_FILTER_EXT_TO_SIZE = SIZE(STRING_FILTER_EXT_TO);

//should match with FILE_TYPE
const char* DEFAULT_EXTENSION[] = {
NULL //	FILE_TYPE_ANY,
		,
		"txt" //	FILE_TYPE_DF,
		,
		"html" //	FILE_TYPE_HTML,
		,
		"lng" //	FILE_TYPE_LANGUAGE,
		,
		"pbn" //	FILE_TYPE_PBN,
		,
		"bts" //	FILE_TYPE_BRIDGE,
		,
		"pts" //	FILE_TYPE_PREFERANS,
		,
		NULL //FILE_TYPE_IMAGE filled in in Widget::staticInit()
		,
		NULL //	FILE_TYPE_ERROR
};

static void drag_and_drop_received(GtkWidget *, GdkDragContext *context, gint x,
		gint y, GtkSelectionData *data, guint ttype, guint time, Widget* widget) {

	gint l = gtk_selection_data_get_length(data);
	gint a = gtk_selection_data_get_format(data);
	if (l >= 0 && a == 8) {
		gchar **pp = gtk_selection_data_get_uris(data);
		widget->openUris(pp);
		g_strfreev(pp);
		gtk_drag_finish(context, true, false, time);
	}
}

Widget::Widget(GtkWidget* widget) {
	setWidget(widget);
}

Widget::~Widget() {
}

DrawingArea& Widget::getArea() const {
	return gframe->m_area;
}

LastTrick& Widget::getLastTrick() const {
	return *glasttrick;
}

Toolbar& Widget::getToolbar() const {
	return gframe->m_toolbar;
}

bool Widget::isModified() const {
	return getProblemSelector().isModified();
}

bool Widget::isEditEnable() const {
	return getToolbar().isEditEnable();
}

void Widget::enableEdit(bool enable) {
	gframe->enableEdit(enable);
}

bool Widget::isUndoEnable() const {
	return getArea().isUndoEnable();
}

bool Widget::isRedoEnable() const {
	return getArea().isRedoEnable();
}

const ProblemVector& Widget::getProblemVector() const {
	return getProblemSelector().getProblemVector();
}

ProblemVector& Widget::getProblemVector() {
	return getProblemSelector().getProblemVector();
}

void Widget::updateModified() {
	gframe->updateModified();
}

FileChooserResult Widget::fileChooser(MENU_ID menu, FILE_TYPE filetype,
		CHOOSER_OPTION option, const std::string& filepath) {
	GtkFileFilter *gtkFilter;
	FILE_TYPE defaultFiletype;
	GtkWidget *dialog;
	FileChooserResult result;
	GtkWidget* button;
	std::string s;
	VString v;
	VString fname;
	VStringI it;
	int i,j;
	STRING_ID sid;
	const char*p;
	VStringID vid;

	assert(SIZE(STRING_FILTER_EXT_FROM)==SIZE(STRING_FILTER_EXT_TO));

	const bool openAll = option == CHOOSER_OPTION_ADD_ALL_SUPPORTED
			|| option == CHOOSER_OPTION_OPEN_ALL_SUPPORTED
			|| option == CHOOSER_OPTION_OPEN_OR_ADD_ALL_SUPPORTED;
	/* if user click close button in dialog (in top right corner "X" where three buttons)
	 * gtk_dialog_run returns GTK_RESPONSE_DELETE_EVENT
	 * so use for cancel button same code
	 * if user make double click on file then gtk_dialog_run returns code of first button
	 * so it doesn't matter which code use for other buttons (except "cancel" button)
	 */

	//no need 'open' button in converterDialog
	if (option == CHOOSER_OPTION_ADD_ALL_SUPPORTED) {
		p = getString(STRING_ADD);
	}
	else if (option == CHOOSER_OPTION_CONVERTER_SAVE_MANY_FILES) {
		p = getString(STRING_TO_MANY_FILES);
	}
	else {
		if (menu == MENU_OPEN) {
			p = getStringNoDots(MENU_OPEN).c_str();
		}
		else if (((filetype == FILE_TYPE_IMAGE || filetype == FILE_TYPE_HTML)
				&& menu == MENU_SAVE) || option == CHOOSER_OPTION_SAVE_AS_SINGLE) {
			p = getString(MENU_SAVE);
		}
		else {
			p = getString(STRING_TO_ONE_FILE);
		}
	}

	dialog = gtk_file_chooser_dialog_new(getString(menu),
			GTK_WINDOW(gframe->getWidget()),
			menu == MENU_OPEN ?
					GTK_FILE_CHOOSER_ACTION_OPEN : GTK_FILE_CHOOSER_ACTION_SAVE, p,
			GTK_RESPONSE_OK, NULL);

	if (option == CHOOSER_OPTION_ADD_ALL_SUPPORTED) {
		s = "add24.png";
	}
	else {
		s = menu == MENU_OPEN ? "folder24.png" : "save24.png";
	}

	button = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog),
			GTK_RESPONSE_OK);
	gtk_button_set_image(GTK_BUTTON(button), image(s));
	addClass(button, "sbutton");

	if (option == CHOOSER_OPTION_SAVE_AS_MANY
			|| option == CHOOSER_OPTION_OPEN_OR_ADD_ALL_SUPPORTED) {
		button = gtk_dialog_add_button(GTK_DIALOG(dialog),
				getString(menu == MENU_OPEN ? STRING_ADD : STRING_TO_MANY_FILES),
				GTK_RESPONSE_APPLY);
		gtk_button_set_image(GTK_BUTTON(button),
				image(menu == MENU_OPEN ? "add24.png" : "split24.png"));
		addClass(button, "sbutton");
	}

	button = gtk_dialog_add_button(GTK_DIALOG(dialog), getString(STRING_CANCEL),
			GTK_RESPONSE_DELETE_EVENT);
	gtk_button_set_image(GTK_BUTTON(button), image("cancel24.png"));
	addClass(button, "sbutton");

	if (openAll) {
		gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), true);
	}

	if (option == CHOOSER_OPTION_DEFAULT) {
		p = DEFAULT_EXTENSION[filetype];
		assert(p);
		s = p;
		for (i = 0; i < STRING_FILTER_EXT_TO_SIZE; i++) {
			v = split(STRING_FILTER_EXT_TO[i], " ");
			if (oneOf(s,v)) {
				break;
			}
		}
		assert(i < STRING_FILTER_EXT_TO_SIZE);
		sid = STRING_FILTER_EXT_FROM[i];

		assert(
				sid != STRING_FILE_FILTER_ALL_SUPPORTED
						&& sid != STRING_FILE_FILTER_ALL);
		vid.push_back(sid);
		vid.push_back(STRING_FILE_FILTER_ALL);
	}
	else {
		for (i = 0; i < ALL_SUPPORTED_SIZE; i++) {
			sid = ALL_SUPPORTED[i];
			//skip html filter for open files
			if (openAll && sid == STRING_FILE_FILTER_HTML) {
				continue;
			}
			vid.push_back(ALL_SUPPORTED[i]);
		}
	}

	j=0;
	for (auto vit : vid) {
		if (filetype == FILE_TYPE_IMAGE && menu == MENU_OPEN
				&& j == 0) {
			s = gconfig->m_allImageFormatString;
		}
		else {
			i = INDEX_OF(vit,STRING_FILTER_EXT_FROM);
			assert(i != -1);
			s = STRING_FILTER_EXT_TO[i];
		}

		/* add html extensions for 'save' option and 'all supported' filter
		 * if 'open' option "all supported formats (*.bts *.pts *.pbn *.txt *.dat)"
		 * if 'save' option all supported formats (*.bts *.pts *.pbn *.txt *.dat *.htm *.html)
		 */
		if (option != CHOOSER_OPTION_DEFAULT && !openAll
				&& vit == STRING_FILE_FILTER_ALL_SUPPORTED) {
			i = INDEX_OF(STRING_FILE_FILTER_HTML,STRING_FILTER_EXT_FROM);
			assert(i != -1);
			s += " ";
			s += STRING_FILTER_EXT_TO[i];
		}

		v = split(s, " ");
		s = " (*." + replaceAll(s, " ", " *.") + ")";
		fname.push_back(getString(vit) + s);

		gtkFilter = gtk_file_filter_new();
		gtk_file_filter_set_name(gtkFilter, fname.back().c_str());

		for (it = v.begin(); it != v.end(); it++) {
			gtk_file_filter_add_pattern(gtkFilter, ("*." + *it).c_str());
		}

		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), gtkFilter);

		if (fname.size() == 1) {
			gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), gtkFilter);
		}

		j++;
	}

	if (filepath.length() > 0) {
		if(isDir(filepath)){
			gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), filepath.c_str());
		}
		else{
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), filepath.c_str());
		}
	}

#ifndef NDEBUG
	//exploreAllChildrenRecursive(dialog);
#endif

	result.m_response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (result.ok()) {
		if (menu == MENU_OPEN) {
			GSList* elem;
			GSList* list = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
			gchar* item;
			for (elem = list; elem; elem = elem->next) {
				item = (gchar*) elem->data;
				result.add(item);
				g_free(item);
			}
			g_slist_free(list);
		}
		else {
			char *file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			result.add(file); //utf8 locale
			g_free(file);

			if (option == CHOOSER_OPTION_SAVE_AS_MANY
					|| option == CHOOSER_OPTION_SAVE_AS_SINGLE
					|| option == CHOOSER_OPTION_CONVERTER_SAVE_MANY_FILES
					|| option == CHOOSER_OPTION_CONVERTER_SAVE_ONE_FILE) {
				gtkFilter = gtk_file_chooser_get_filter(GTK_FILE_CHOOSER(dialog));
				assert(gtkFilter!=NULL);
				s = gtk_file_filter_get_name(gtkFilter);
				it = find(s,fname);
				assert(it != fname.end());

				sid = vid[int(it - fname.begin())];

				defaultFiletype = getFileType(result[0]);

				if (defaultFiletype == FILE_TYPE_ERROR) { //check filter on invalid file extension
					if (sid == STRING_FILE_FILTER_ALL_SUPPORTED
							|| sid == STRING_FILE_FILTER_ALL) {
						defaultFiletype =
								isBridge() ? FILE_TYPE_BRIDGE : FILE_TYPE_PREFERANS;
					}
					else {
						i = INDEX_OF(sid,STRING_FILTER_EXT_FROM);
						assert(i != -1);
						v = split(STRING_FILTER_EXT_TO[i], " ");
						defaultFiletype = getFileType("a." + v[0]);
					}
					addFileExtension(result[0], defaultFiletype);
				}
				//now filename is valid
			}
			else {
				//simple correction
				correctFileExtension(result[0], filetype);
			}

		}
	}
	gtk_widget_hide(dialog);
	gtk_widget_destroy(dialog);

#ifndef NDEBUG
	if (option == CHOOSER_OPTION_DEFAULT) {
		assert(result.size() <= 1);
	}
#endif

	return result;
}

FileChooserResult Widget::fileChooserOpen(FILE_TYPE filetype) {
	assert(filetype != FILE_TYPE_ANY);
	return fileChooser(MENU_OPEN, filetype, CHOOSER_OPTION_DEFAULT, "");
}

FileChooserResult Widget::fileChooserSave(FILE_TYPE filetype) {
	return fileChooser(MENU_SAVE, filetype, CHOOSER_OPTION_DEFAULT, "");
}

FileChooserResult Widget::fileChooser(MENU_ID menu, const std::string& filepath,
		bool allowSplit) {
	FILE_TYPE ft = getFileType(filepath);
	return fileChooser(menu, ft,
			allowSplit ? CHOOSER_OPTION_SAVE_AS_MANY : CHOOSER_OPTION_SAVE_AS_SINGLE,
			filepath);
}

void Widget::setDragDrop(GtkWidget* widget) {
	gtk_drag_dest_set(widget, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(widget);
	g_signal_connect(widget, "drag-data-received",
			G_CALLBACK(drag_and_drop_received), gpointer(this));
}

void Widget::addFileExtension(std::string& filepath, FILE_TYPE filetype) {
	const char*p = DEFAULT_EXTENSION[filetype];
	assert(p);
	filepath += ".";
	filepath += p;
}

void Widget::correctFileExtension(std::string& filepath, FILE_TYPE filetype) {
	/* if user set invalid extension or empty extension add it to make filename valid
	 * For example "name.1.2" -> "name.1.2.html"
	 */
	FILE_TYPE type = getFileType(filepath);
	if (type != filetype) { //filepath type should be =filetype
		addFileExtension(filepath, filetype);
	}
}

void Widget::openUris(char** uris) {
	getProblemSelector().openUris(uris);
}

void Widget::openFiles(const char*files) {
	getProblemSelector().openFiles(files);
}

void Widget::setFont(cairo_t* cr,int height) {
	cairo_select_font_face(cr, gconfig->getFontFamily(),
			cairo_font_slant_t(gconfig->getFontStyle()),
			gconfig->getFontWeight() >= PANGO_WEIGHT_BOLD ?
					CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, height); //height in pixels
}

GtkWidget* Widget::createImageCombobox() {
	GtkWidget*combo = gtk_combo_box_new();
	GtkCellRenderer *renderer;
	renderer = gtk_cell_renderer_pixbuf_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "pixbuf", 0,
	NULL);
	return combo;
}

GtkTreeModel* Widget::createTrumpModel(bool onlySuits, bool small) {
	return createTrumpModel(onlySuits, small, getGameType());
}

GtkTreeModel* Widget::createTrumpModel(bool onlySuits, bool small,
		GAME_TYPE gt) {
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GtkTreeStore *store;
	gint i;

	const int size = small ? 28 : SUIT_PIXBUF_SIZE;

	store = gtk_tree_store_new(1, GDK_TYPE_PIXBUF);
	for (i = 0; i < (onlySuits ? NT : TRUMP_MODEL_SIZE[gt]); i++) {
		if (i < NT) {
			pixbuf = getSuitPixbuf(i, size);
		}
		else {
			pixbuf = getStringPixbuf(i == NT, size);
		}

		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, pixbuf, -1);
		g_object_unref(pixbuf);
	}

	return GTK_TREE_MODEL(store);
}

GtkTreeModel* Widget::createContractModel() {
	GdkPixbuf *pixbuf;
	GtkTreeIter iter;
	GtkTreeStore *store;
	gint i;

	store = gtk_tree_store_new(1, GDK_TYPE_PIXBUF);

	for (i = MIN_CONTRACT[getGameType()]; i <= MAX_CONTRACT[getGameType()]; i++) {
		pixbuf = getContractPixbuf(i);

		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, pixbuf, -1);
		g_object_unref(pixbuf);
	}

	return GTK_TREE_MODEL(store);

}

GdkPixbuf* Widget::getStringPixbuf(bool nt, int size) {
	//if nt=false then pixbuf for 'misere' string
	int i;
	double x, y;
	GdkPixbuf* pixbuf = NULL;
	cairo_t *cr=0;
	cairo_surface_t *surface=0;
	cairo_text_extents_t extents;
	std::string p = nt ? getNTString() : getString(STRING_MISERE);
	for (i = 0; i < 2; i++) { //i==0 get needed width,i=1 draw
		createNew(cr,surface,CSize(i == 0 ? size : extents.width, size));
		setFont(cr,nt ? size : size / 2);
		if (i == 0) {
			cairo_text_extents(cr, p.c_str(), &extents);
		}
		else {
			cairo_set_source_rgb(cr, 0, 0, 0); //font color
			x = extents.width / 2 - (extents.width / 2 + extents.x_bearing);
			y = size / 2 - (extents.height / 2 + extents.y_bearing);
			cairo_move_to(cr, x, y);
			cairo_show_text(cr, p.c_str());
			pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, extents.width, size);
		}
	}
	destroy(surface);
	destroy(cr);
	return pixbuf;
}

const VString Widget::createStringVector(const char* text[], guint size) {
	guint i;
	VString vs;
	for (i = 0; i < size; i++) {
		vs.push_back(text[i]);
	}
	return vs;
}

GtkWidget* Widget::createTextCombobox(const VString& text) {
	GtkWidget*w = gtk_combo_box_new_with_model(createTextModel(text));
	gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(w), renderer, FALSE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(w), renderer, "text", 0, NULL);

	//g_object_set (G_OBJECT (renderer),"font", family,"style", style,"size", 20,NULL);//don'n remove leave for further needs
	char*p = pango_font_description_to_string(getFont());
	g_object_set(G_OBJECT(renderer), "font", p, NULL);
	g_free(p);

	return w;
}

GtkWidget* Widget::createTextCombobox(const STRING_ID i1, const STRING_ID i2) {
	VString vs={getString(i1),getString(i2)};
	return createTextCombobox(vs);
}

GtkWidget* Widget::createTextCombobox(const STRING_ID id, int length){
	VString vs;
	int j=id;
	for(int i=0;i<length;i++,j++){
		vs.push_back(getString(STRING_ID(j)));
	}
	return createTextCombobox(vs);
}


GtkTreeModel* Widget::createTextModel(const VString& text) {
	GtkTreeIter iter;
	GtkTreeStore *store;
	VStringCI it;

	store = gtk_tree_store_new(1, G_TYPE_STRING);
	for (it = text.begin(); it != text.end(); it++) {
		gtk_tree_store_append(store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, it->c_str(), -1);
	}
	return GTK_TREE_MODEL(store);

}

GtkWidget* Widget::createTextCombobox(int from, int to, int step,
		const char* additionalString) {
	int i;
	assert(from < to);
	assert(step > 0);
	VString vs;
	for (i = from; i <= to; i += step) {
		vs.push_back(format("%d", i));
	}
	if (additionalString != NULL) {
		vs.push_back(additionalString);
	}
	return createTextCombobox(vs);

}

int Widget::getFontHeight() const {
	return gconfig->getFontHeight();
}

void Widget::widgetAddFile(char* url) {
	FILE_TYPE t = getFileType(url);
	if (t != FILE_TYPE_ERROR && t != FILE_TYPE_HTML && t != FILE_TYPE_IMAGE) {
		m_widgetFiles.push_back(url);
	}
}

void Widget::widgetAddDir(char* url) {
	GDir *dir;
	const gchar *filename;
	std::string s;
	gchar name[PATH_MAX];

	dir = g_dir_open(url, 0, 0);
	if (dir == NULL) {
		//error
		return;
	}
	while ((filename = g_dir_read_name(dir))) {
		sprintf(name, "%s%c%s", url, G_DIR_SEPARATOR, filename);
		if (isDir(name)) {
			widgetAddDir(name);
		}
		else {
			widgetAddFile(name);
		}
	}

}

const VString& Widget::getValidFilesList(char** uris) {
	int i;
	gchar *fn;
	m_widgetFiles.clear();
	for (i = 0; uris[i] != 0; i++) {
		fn = g_filename_from_uri(uris[i], NULL, NULL);
		if (isDir(fn)) {
			widgetAddDir(fn);
		}
		else {
			widgetAddFile(fn);
		}
		g_free(fn);
	}

	return m_widgetFiles;
}

void Widget::menuClick(MENU_ID id) {
	gframe->menuClick(id);
}

bool Widget::isTrumpChanged() const {
	return getToolbar().isTrumpChanged();
}

void Widget::setTrumpChanged(bool changed) {
	getToolbar().setTrumpChanged(changed);
}

void Widget::showToolTip(STRING_ID id) {
	std::string s = getString(id);
	if(id==STRING_CLICK_TO_ROTATE_BY_90_DEGREES){
		//lower string & and remove dot at the end of the string
		s=utf8ToLowerCase(s.substr(0, s.length()-1));
	}
	if (gconfig->m_showToolTips) {
		getToolbar().showToolTip(s.c_str());
	}
}

void Widget::hideToolTip() {
	if (gconfig->m_showToolTips) {
		getToolbar().hideToolTip();
	}
}

/*
GtkWidget* Widget::getToolbarImage(TOOLBAR_BUTTON id, bool small) {
	return getToolbarImage(id, small, isToolbarButtonEnabled(id));
}

GtkWidget* Widget::getToolbarImage(TOOLBAR_BUTTON id, bool small, bool enabled) {
	return getToolbarImage(id, small,boolToButtonState(enabled));
}
*/

GtkWidget* Widget::getToolbarImage(TOOLBAR_BUTTON id, bool small, BUTTON_STATE state) {
	return gtk_image_new_from_pixbuf(getToolbarPixbuf(id, small, state));
}

/*
GdkPixbuf* Widget::getToolbarPixbuf(TOOLBAR_BUTTON id, bool small) {
	return getToolbarPixbuf(id, small, boolToButtonState(isToolbarButtonEnabled(id)));
}


GdkPixbuf* Widget::getToolbarPixbuf(TOOLBAR_BUTTON id, bool small, bool enabled) {
	return getToolbarPixbuf(id, small, boolToButtonState(enabled));
}
*/

GdkPixbuf* Widget::getToolbarPixbuf(TOOLBAR_BUTTON id, bool small,
		BUTTON_STATE state) {
	return getToolbar().getPixbuf(id, small, state);
}

void Widget::staticInit() {
	int i;
	i = INDEX_OF(STRING_IMAGE_FILES,STRING_FILTER_EXT_FROM); //cann't move this code to Config G_N_ELEMENTS doesn't work for extern
	assert(i != -1);
	STRING_FILTER_EXT_TO[i] = gconfig->m_storeImageFormatString.c_str();

	DEFAULT_EXTENSION[FILE_TYPE_IMAGE] = gconfig->m_storeImageFormat[0].c_str();
}

int Widget::getArrowSize() {
	return gconfig->m_arrowSize;
}

void Widget::setArrowParameters(int arrow, int arrowSize/*=SKIP_ARROW_SIZE*/){
	gconfig->setArrowParameters(arrow, arrowSize);
}

CSize Widget::getCardSize() const {
	return gconfig->getCardSize();
}

int& Widget::getCardWidth(){
	return gconfig->getCardWidth();
}

int& Widget::getCardHeight(){
	return gconfig->getCardHeight();
}

ProblemSelector& Widget::getProblemSelector()const{
	return *gproblemselector;
}

Problem& Widget::getProblem() const {
	return getProblemSelector().getProblem();
}

void Widget::parsePreferansSolveAllDeclarersParameters(int v, int& trump,
		bool& misere, CARD_INDEX& player) {
	if (v < 6) {
		trump = NT;
		misere = v < 3;
	}
	else {
		trump = (v - 6) / 3;
		misere = false;
	}
	player = getPreferansPlayer(v % 3);
}

int Widget::getMaxRunThreads() const {
	return gconfig->m_maxThreads;
}

int Widget::getMaxHandCards()const{
	return isBridge() ? MAX_BRIDGE_HAND_CARDS : MAX_PREFERANS_HAND_CARDS;
}

int Widget::countTableSize(int cardHeight, int arrowSize, int y) {
	return gconfig->countTableSize(cardHeight, arrowSize, y);
}

int Widget::countTableTop(int cardHeight) {
	return gconfig->countTableTop(cardHeight);
}

int Widget::countAreaHeight(int cardHeight, int arrowSize, int y) {
	return gconfig->countAreaHeight(cardHeight, arrowSize, y);
}

CSize Widget::countMaxCardSizeForY(int arrowSize, int y) {
	return gconfig->countMaxCardSizeForY(arrowSize, y);
}

int Widget::countMaxArrowSizeForY(int cardHeight,int y){
	int i=countAreaHeight(cardHeight,0,y);
	return (getAreaMaxHeight()-i)/ARROW_K_IN_AREA_HEIGHT;
}

GdkPixbuf *& Widget::getSvgPixbuf(bool isDeck){
	auto& p=getProblemSelector();
	return isDeck? p.m_svgDeckPixbuf:p.m_svgArrowPixbuf;
}

void Widget::drawTextToCairo(cairo_t* cr, TextWithAttributes text, CRect rect,
		bool centerx, bool centery) {
	const GdkRGBA rgba = text.isBlackColor() ? BLACK_COLOR : getTextColor();
	int w, h;
	gdk_cairo_set_source_rgba(cr, &rgba);
	PangoLayout *layout = createPangoLayout(cr,text);
	pango_layout_get_pixel_size(layout, &w, &h);
	double px = rect.left;
	double py = rect.top;
	if (centerx) {
		px += (rect.width() - w) / 2;
	}
	if (centery) {
		py += (rect.height() - h) / 2;
	}

	cairo_move_to(cr, px, py);
	pango_cairo_update_layout(cr, layout);
	pango_cairo_show_layout(cr, layout);

	g_object_unref(layout);
}

bool Widget::isScalableArrow(){
	return gconfig->isScalableArrow();
}

bool Widget::isScalableArrow(int arrow){
	return gconfig->isScalableArrow(arrow);
}

bool Widget::isScalableDeck(){
	return gconfig->isScalableDeck();
}

bool Widget::isScalableDeck(int deck){
	return gconfig->isScalableDeck(deck);
}

GdkRGBA Widget::getTextColor() const {
	return gconfig->getFontColor();
}

GdkRGBA* Widget::getFontColorPointer(){
	return &gconfig->getFontColor();
}

PangoLayout* Widget::createPangoLayout(cairo_t *cr,TextWithAttributes text) {
	std::string o;
	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription*desc = createPangoFontDescription(gconfig->m_font,text.getHeight());
	pango_layout_set_font_description(layout, desc);

	if (text.isUnderline()) {
		o = "<span underline=\"low\">" + text.getText() + "</span>";
	}
	else if (text.isBold()) {
		o = "<b>" + text.getText() + "</b>";
	}
	else {
		o = text.getText();
	}
	pango_layout_set_markup(layout, o.c_str(), -1);
	pango_font_description_free(desc);
	return layout;
}

CSize Widget::getMaxSize()const{
	return gconfig->m_workareaRect.size();
}

void Widget::updateUndoRedoAll(){
	return gframe->updateUndoRedo();
}

void Widget::updateFindBestStateAll(){
	return gframe->updateFindBestState();
}

void Widget::updateThinkAll(){
	return gframe->updateThink();
}

int Widget::recentSize(){
	return gconfig->recentSize();
}

Menu& Widget::getMenu() {
	return *gmenu;
}

bool Widget::isToolbarButtonEnabled(TOOLBAR_BUTTON id) {
	//BUTTON_STATE_ENABLED, BUTTON_STATE_DISABLED, BUTTON_STATE_FIND_BEST_STOP
	return getToolbarButtonState(id) != BUTTON_STATE_DISABLED;
}

BUTTON_STATE Widget::getToolbarButtonState(TOOLBAR_BUTTON id){
	if (id == TOOLBAR_BUTTON_FIND_BEST) {
		//BUTTON_STATE_ENABLED, BUTTON_STATE_DISABLED, BUTTON_STATE_FIND_BEST_STOP
		return getFindBestState();
	}
	else {
		return boolToButtonState(
				(id == TOOLBAR_BUTTON_UNDO || id == TOOLBAR_BUTTON_UNDOALL) ?
						isUndoEnable() : isRedoEnable());
	}
}

BUTTON_STATE Widget::getFindBestState(bool checkTrump) const {
	return
			think() ?
					BUTTON_STATE_FIND_BEST_STOP :
					boolToButtonState(isValidDeal(checkTrump));
}

bool Widget::isValidDeal(bool checkTrump) const {
	return getDealState(checkTrump) == DEAL_STATE_VALID;
}

bool Widget::isNewDeal() const {
	return getDealState() == DEAL_STATE_NEW;
}

bool Widget::isEmptyDeal() const {
	return getDealState() == DEAL_STATE_EMPTY;
}

DEAL_STATE Widget::getDealState(bool checkTrump) const {
	return getProblem().getDealState(checkTrump);
}

void Widget::showAllExclude(GtkWidget*w , VGtkWidgetPtr const&v){
	if(GTK_IS_CONTAINER(w)){
		GList *children = gtk_container_get_children(GTK_CONTAINER(w));
		for(GList *iter = children; iter != NULL; iter = g_list_next(iter)){
			showAllExclude(GTK_WIDGET(iter->data),v);
		}
		g_list_free(children);
	}
	if(!oneOf(w,v)){
		gtk_widget_show(w);
	}
}

void Widget::showAllExclude(VGtkWidgetPtr const& v){
	showAllExclude(m_widget,v);
}

cairo_surface_t * Widget::getBackgroundFullSurface() {
	return getProblemSelector().m_backgroundFullSurface;
}

void Widget::setSkin(int skin){
	gconfig->setSkin(skin);
	gframe->updateSkin();
}

int Widget::getBestLineHeight()const{
	return gproblemselector->m_bestLineHeight;
}

int Widget::getTricks(CARD_INDEX player) const {
	int index = INDEX_OF(player,PLAYER);	//always PLAYER not PREFERANS_PLAYER
	assert(index != -1);
	return getState().m_tricks[index];
}

int Widget::getResultAdditionalTricks() const{
	CARD_INDEX ci=getDeclarer();
	return isBridge() ? getSideTricks(ci) : getTricks(ci);
}

bool Widget::isDeclarerNorthOrSouth() const {
	assert(isBridge());
	return northOrSouth(getDeclarer());
}

CARD_INDEX Widget::getDeclarer() const {
	return getProblem().getDeclarer();
}

int Widget::getSideTricks(CARD_INDEX player) const {
	assert(isBridge());
	return getTricks(player)+getTricks(getBridgePartner(player));
}

int Widget::languages() {
	return gconfig->m_language.size();
}

bool Widget::isLanguage(MENU_ID id) {
	return id>=MENU_LANGUAGE_FIRST && id<MENU_LANGUAGE_FIRST+languages();
}

int Widget::getActiveCardShift() {
	return gconfig->getActiveCardShift();
}
