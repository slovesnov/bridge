/*
 * Frame.cpp
 *
 *  	   Created on: 10.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include <glib.h>
#include <glib/gstdio.h>
#include "Frame.h"
#include "dialogs/AboutDialog.h"
#include "dialogs/CalculatorDialog.h"
#include "dialogs/ConverterDialog.h"
#include "dialogs/EditDescriptionDialog.h"
#include "dialogs/EditListDialog.h"
#include "dialogs/PbnEditorDialog.h"
#include "dialogs/SaveModifiedWarningDialog.h"
#include "dialogs/SuitsOrderDialog.h"
#include "dialogs/CustomSkinDialog.h"
#include "dialogs/DeckArrowSelectionDialog.h"
#ifndef NDEBUG
#include <cstdint>//for getOptimal(int n,bool printToFile)
#include <cmath>
#endif

Frame*gframe;

static void open_files(GtkWidget*, const char* data) {
	gframe->openFiles(data);
}

static gboolean new_version_message(gpointer) {
	gframe->newVesionMessage();
	return G_SOURCE_REMOVE;
}

static gboolean delete_event(GtkWidget*, GdkEvent*, gpointer) {
	return !gframe->exit();
}

static void size_allocate(GtkWidget*, GtkAllocation*, gpointer) {
	gframe->allocated();
}

static gboolean finish_adjust(gpointer) {
	gframe->finishAdjust();
	return G_SOURCE_REMOVE;
}

static gboolean finish_save_image(gpointer) {
	gframe->finishSaveImage();
	return G_SOURCE_REMOVE;
}

Frame::Frame(GtkApplication *application, const char* filepath) :
		FrameItem(gtk_application_window_new(GTK_APPLICATION(application))) {
	GtkWidget *w;
	CSize sz;

	gframe = this;
	srand((unsigned) time( NULL)); //for random functions randomDeal(), solveAllFoe()
	setlocale(LC_NUMERIC, "C");  //dot interpret as decimal separator, 2may2021 for all threads

	m_vbox1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(m_vbox1), m_area.getWidget());
#ifdef TOOLTIP_IN_STATUSBAR
	addWidget(gconfig->m_showToolTips,m_vbox1, m_toolbar.m_tooltip);
#endif

	m_vbox2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(m_vbox2), m_problemSelector.getWidget());
	addWidget(gconfig->m_showLastTrick,m_vbox2, m_lastTrick.m_full);

	w = gtk_grid_new();

	gtk_grid_attach(GTK_GRID(w), m_menu.getWidget(), 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(w), m_problemSelector.getLabel(), 1, 0, 1, 1);

	gtk_grid_attach(GTK_GRID(w), m_toolbar.getWidget(), 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(w), m_problemSelector.m_toolbar, 1, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(w), m_vbox1, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(w), m_vbox2, 1, 2, 1, 1);

	gtk_container_add(GTK_CONTAINER(getWidget()), w);

	gtk_window_set_resizable(GTK_WINDOW(getWidget()), FALSE);

	//m_problemSelector should goes first because it manage with problem vector
	m_childs = {
			&m_problemSelector,
			&m_menu,
			&m_toolbar,
			&m_area,
			&m_lastTrick };

	updateUndoRedo();
	updateFindBestState();
	updateAfterCreation();

	//it seems that gtk_window_move() works correctly if set sizes of all item[..] correctly
	gtk_window_move(GTK_WINDOW(getWidget()), gconfig->m_startPosition.x,
			gconfig->m_startPosition.y);

	//call the same function with g_signal_emit_by_name from main.cpp
	open_files(NULL, filepath);

	showAll();

	/* if only one problem m_problemSelector toolbar should be hided, no blink
	 * should be called after showAll();
	 */
	m_problemSelector.updateToolbar();

	updateTitle();

	/* NOTE should be called after showAll() to show message after window
	 * & before gtk_main() otherwise works wrong
	 * sometimes take a long time so use thread
	 */
	m_newVersion.start(VERSION_FILE_URL, CURRENT_VERSION,
			new_version_message);

	g_signal_new(OPEN_FILE_SIGNAL_NAME, G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0,
	NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1,
	G_TYPE_POINTER);

	g_signal_connect(getWidget(), OPEN_FILE_SIGNAL_NAME, G_CALLBACK(open_files),
			NULL);
	g_signal_connect(getWidget(), "destroy", G_CALLBACK(gtk_main_quit), NULL); //need for 'exit' menu
	g_signal_connect(getWidget(), "delete-event", G_CALLBACK(delete_event), NULL);
	m_sizeAllocateSignal = g_signal_connect(getWidget(), "size-allocate",
			G_CALLBACK(size_allocate), NULL);

#ifndef NDEBUG

#endif

	gtk_main();
}

Frame::~Frame() {
}

void Frame::setLanguage(int index) {
	gconfig->setLanguageFileName(index);
	updateLanguage();
}

void Frame::setCustomLanguage() {
	FileChooserResult r = fileChooserOpen(FILE_TYPE_LANGUAGE);
	if (r.ok()) {
		gconfig->m_languageFileName = r.file();
		updateLanguage();
	}
}

void Frame::menuClick(MENU_ID id) {
	int i;

	switch (id) {
	case MENU_NEW:
		newGame();
		break;

	case MENU_OPEN:
	case MENU_OPEN_FROM_LIBRARY:
		openAddFile(id);
		break;

	case MENU_SAVE:
		save();
		break;

	case MENU_SAVE_AS:
		saveAs();
		break;

	case MENU_EDIT:
		m_area.edit();
		break;

	case MENU_EDIT_PROBLEM_LIST:
		editList();
		break;

	case MENU_EDIT_DESCRIPTION:
		editDescription();
		break;

	case MENU_PBN_EDITOR:
		editPbn();
		break;

	case MENU_ROTATE_CLOCKWISE:
	case MENU_ROTATE_COUNTERCLOCKWISE:
	case MENU_ROTATE_BY_90_DEGREES_CLOCKWISE:
	case MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE:
		m_area.rotate(
				id == MENU_ROTATE_CLOCKWISE
						|| id == MENU_ROTATE_BY_90_DEGREES_CLOCKWISE,
				id == MENU_ROTATE_BY_90_DEGREES_CLOCKWISE
						|| id == MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE);
		break;

	case MENU_SAVE_HTML:
	case MENU_SAVE_HTML_WITH_PICTURES:
		saveHtml(id == MENU_SAVE_HTML_WITH_PICTURES);
		break;

	case MENU_SAVE_IMAGE:
		saveImage();
		break;

	case MENU_EXIT:
		if (exit()) {
			gtk_widget_destroy(getWidget());//calls destroy_frame()
		}
		break;

		//addons
	case MENU_CONVERT:
		converter();
		break;

	case MENU_SOLVE_FOR_ALL_DECLARERS:
		solveForAllDeclarers();
		break;

	case MENU_CLEAR_DEAL:
	case MENU_RANDOM_DEAL:
		setDeal(id == MENU_RANDOM_DEAL);
		break;

	case MENU_GAME_TYPE:
		changeGameType();
		break;

	case MENU_CALCULATOR:
		calculator();
		break;

	case MENU_SOLVE_ALL_FOE:
		solveAllFoe();
		break;

		/*see upper
		 case MENU_ROTATE_BY_90_DEGREES_CLOCKWISE:
		 case MENU_ROTATE_BY_90_DEGREES_COUNTERCLOCKWISE:
		 */

		//view (all check menu items see in default section of switch)
	case MENU_SELECT_ARROW:
		selectArrow();
		break;

	case MENU_SELECT_DECK:
		selectDeck();
		break;

	case MENU_SELECT_FONT:
		if (selectFont(getString(id), &gconfig->m_font)) {
			gconfig->updateCSS();
			updateFontSelection();
		}
		break;

	case MENU_SELECT_FONT_COLOR:
		if (selectColor(getString(id), getFontColorPointer())) {
			gconfig->updateCSS();
			//new font redraw window
			updateLanguage();  //same reaction on change font and change language
		}
		break;

	case MENU_SUITS_ORDER:
		suitsOrder();
		break;

		//options (all check menu items see in default section of switch)
	case MENU_ESTIMATE_NONE:
	case MENU_ESTIMATE_BEST_LOCAL:
	case MENU_ESTIMATE_BEST_TOTAL:
	case MENU_ESTIMATE_ALL_LOCAL:
	case MENU_ESTIMATE_ALL_TOTAL:
		gconfig->setEstimateType(ESTIMATE(id - MENU_ESTIMATE_NONE));
		updateEstimationType();
		break;

		//move
	case MENU_UNDOALL:
	case MENU_UNDO:
	case MENU_REDO:
	case MENU_REDOALL:
		m_area.undoRedo(id == MENU_UNDOALL || id == MENU_UNDO,
				id == MENU_UNDOALL || id == MENU_REDOALL);
		break;

	case MENU_FIND_BEST_MOVE:
		m_area.findBest(NULL);
		break;

	case MENU_LOAD_LANGUAGE_FILE:
		setCustomLanguage();
		break;

		//skin
	case MENU_SKIN0:
	case MENU_SKIN1:
	case MENU_SKIN2:
	case MENU_SKIN3:
	case MENU_SKIN4:
	case MENU_SKIN5:
	case MENU_SKIN6:
	case MENU_SKIN7:
		setSkin(id - MENU_SKIN0);
		break;

	case MENU_CUSTOM_SKIN:
		setCustomSkin();
		break;

		//help
	case MENU_HOMEPAGE:
		openURL(BASE_ADDRESS);
		break;

	case MENU_ABOUT:
		about();
		break;

	default:
		if (id >= MENU_LANGUAGE_FIRST && id < MENU_NEW) {
			//language
			setLanguage(id - MENU_LANGUAGE_FIRST);
		}
		else if (id < MENU_PROBLEM) {
			//recent click
			openFiles(gconfig->m_recent[id - MENU_RECENT]);
		}
		else if ((i = INDEX_OF(id,CHECKED_MENU)) != -1) {
			gconfig->switchOption(i);  //NOT id
			m_menu.setItemAttributes(id);
			if (id == MENU_LAST_TRICK_GAME_ANLYSIS) {
				addRemoveWidget(gconfig->m_showLastTrick, m_vbox2, m_lastTrick.m_full);
				m_lastTrick.changeShowOption();
				m_problemSelector.changeShowOption();
			}
			else if (id == MENU_TOOLTIPS) {
#ifdef TOOLTIP_IN_STATUSBAR
				addRemoveWidget(gconfig->m_showToolTips, m_vbox1, m_toolbar.m_tooltip);
				m_toolbar.changeShowOption();
				m_lastTrick.changeShowOption();
				m_problemSelector.changeShowOption();

#else
				if (!gconfig->m_showToolTips) {
					hideToolTip();
				}
#endif
			}
			else if (id == MENU_TOTAL_TRICKS || id == MENU_PLAYER_TRICKS) {
				m_area.redraw();
			}
		}
		else {
			assert(0);
		}
	}
}

void Frame::callFrameItemFunction(FrameItemFunction f, int option) {
	int i;

	if (option == 0) {
		for (VFrameItemPointer::iterator it = m_childs.begin();
				it != m_childs.end(); it++) {
			((*it)->*f)();
		}
	}
	else {
		/* use special order for updateLanguage(), m_problemSelector setCheckLabelFit() calls
		 * getString(MENU_PROBLEM), so menu should be changed before m_problemSelector
		 */
		FrameItem* item[] = {
				&m_menu,
				&m_area,
				&m_problemSelector,
				&m_toolbar,
				&m_lastTrick };
		for (i = 0; i < SIZEI(item); i++) {
			(item[i]->*f)();
		}
	}
}

void Frame::saveAs() {
	FileChooserResult r = fileChooser(MENU_SAVE_AS, m_problemSelector.getFile(),
			m_problemSelector.size() > 1);
	if (r.ok()) {
		if (r.m_response == GTK_RESPONSE_APPLY) {
			finishSaveFile(r.file(), true);
		}
		else {
			if (getFileType(r.file()) == FILE_TYPE_HTML) {
				m_area.saveHtml(r.file(), gconfig->m_htmlStoreWithImages);
			}
			else {
				finishSaveFile(r.file(), false);
			}
		}
	}
}

void Frame::save() {
	if (proceedSaveAsSaveAs()) {
		saveAs();
	}
	else {
		m_problemSelector.save();
		updateModified();
	}
}

void Frame::converter() {
	ConverterDialog dialog;
}

void Frame::editList() {
	EditListDialog d;
	if (d.getReturnCode() == GTK_RESPONSE_OK) {
		m_problemSelector.setFrom(d);
	}
}

void Frame::editPbn() {
	PbnEditorDialog d;
	if (d.getReturnCode() == GTK_RESPONSE_OK) {
		m_problemSelector.setFrom(d);
	}
}

void Frame::editDescription() {
	EditDescriptionDialog dialog;
}

void Frame::saveHtml(bool images) {
	FileChooserResult r = fileChooserSave(FILE_TYPE_HTML);
	if (r.ok()) {
		m_area.saveHtml(r.file(), images);
	}
}

void Frame::saveImage() {
	FileChooserResult r = fileChooserSave(FILE_TYPE_IMAGE);
	if (r.ok()) {
		//cann't store here dialog form fileChooserSave() is visible don't know why
		m_saveImagePath = r.file();
		g_idle_add(finish_save_image, NULL);
	}
}

void Frame::openAddFile(MENU_ID id) {
	if (!saveIfModified()) {
		return;
	}
	std::string path =
			id == MENU_OPEN ?
					"" : getApplicationName() + G_DIR_SEPARATOR_S + "problems";
	FileChooserResult r = fileChooser(MENU_OPEN, FILE_TYPE_ANY,
			CHOOSER_OPTION_OPEN_OR_ADD_ALL_SUPPORTED, path);
	if (r.ok()) {
		assert(
				r.m_response == GTK_RESPONSE_OK || r.m_response == GTK_RESPONSE_APPLY);
		m_problemSelector.set(r.m_files, r.m_response == GTK_RESPONSE_APPLY);
	}
}

void Frame::setDeal(bool random) {
	for (auto& i : m_childs) {
		i->setDeal(random);
	}
}

void Frame::updateTitle() {
	gtk_window_set_title(GTK_WINDOW(getWidget()),
			m_problemSelector.getTitle().c_str());
}

void Frame::changeGameType() {
	enableEdit(true);
	updateGameType();
	updateFindBestState();
	updateTitle();
}

void Frame::updateRecent(std::string filepath) {
	VStringI it;

	for (it = gconfig->m_recent.begin(); it != gconfig->m_recent.end(); it++) {
		if (*it == filepath) {
			break;
		}
	}

	if (it == gconfig->m_recent.end()) {
		if (gconfig->m_recent.size() + 1 > unsigned(gconfig->m_maxRecent)) {
			gconfig->m_recent.pop_back();
		}
	}
	else {
		gconfig->m_recent.erase(it);
	}
	gconfig->m_recent.insert(gconfig->m_recent.begin(), filepath);
	m_menu.updateRecent();
}

void Frame::newVesionMessage() {
	std::string s = getString(STRING_NEW_VERSION_FOUND);
	s += "\n" + m_newVersion.m_message;
	if (message(MESSAGE_ICON_NONE, s, BUTTONS_DIALOG_YES_NO_CANCEL)
			== GTK_RESPONSE_YES) {
		//download new version
		openURL(DOWNLOAD_URL);
	}
}

void Frame::solveForAllDeclarers() {
	m_area.solveAllDeclarers();
}

void Frame::solveAllFoe() {
	m_area.solveAllFoe();
}

void Frame::about() {
	AboutDialog dialog;
}

void Frame::finishSaveFile(std::string filepath, bool split) {
	m_problemSelector.save(filepath, split);
	updateModified();
}

void Frame::selectArrow() {
	DeckArrowSelectionDialog dialog(false);
	if (dialog.getReturnCode() == GTK_RESPONSE_OK) {
		updateArrowSize();
	}
}

void Frame::selectDeck() {
	DeckArrowSelectionDialog dialog(true);
	if (dialog.getReturnCode() == GTK_RESPONSE_OK) {
		updateDeckSelection();
	}
}

void Frame::suitsOrder() {
	SuitsOrderDialog d;
}

void Frame::calculator() {
	CalculatorDialog d;
}

#ifndef NDEBUG
void Frame::explorePbn(const char* dir, const char* filename) { //checks score calculator on real data
	VStringI it;
	ProblemVector p;
	std::set<std::string> set;
	std::set<std::string> setA;
	std::set<std::string>::const_iterator seti;
	VProblemCI ci;
	VPbnEntryCI pi;
	int files = 0;
	int tags = 0;
	int problems = 0;
	int errors = 0;
	std::string s;
	int i;
	const char* TN[] = { "score", "contract", "result", "vulnerable", "declarer" };
	const int sz = SIZEI(TN);
	std::string value[sz];
	int validProblems = 0;
	char*q;
	const char*w, *w1;
	bool valid;
	int score, contract, result, doubleRedouble, declarer, vulnerable;
	int trump, countscore;
	const char SUITS_UP[] = "SHDCN";
	const char DECLARER[] = "NESW";
	//should use std::string not const char* because function indexOf() use VULNERABLE[i]==s
	const std::string VULNERABLE[] = { "None", "NS", "EW", "All" };

	const char* ST[] = { PBN_AUCTION_TAG, PBN_DECLARER_TAG, PBN_CONTRACT_TAG };
	const int szst = SIZEI(ST);
	/*	int ftag;*/
	PbnEntry entry[szst];

	explorePbnDirectory(dir);
	for (it = m_pbns.begin(); it != m_pbns.end(); it++) {
		files++;

		ProblemVector p(it->c_str());

		for (ci = p.m_problems.begin(); ci != p.m_problems.end(); ci++) {
			problems++;

			for (i = 0; i < sz; i++) {
				value[i] = "";
			}

			const VPbnEntry& ve = ci->m_pbnEntry;

			for (pi = ve.begin(); pi != ve.end(); pi++) {
				if (cmpnocase(pi->tag, PBN_DATE_TAG)) {
					if (pi->value != "#" && pi->value.length() != 10) {
						println("[%s]", pi->value.c_str());
					}
				}
			}

			assert(ve.size() != 0);
			for (pi = ve.begin(); pi != ve.end(); pi++) {
				tags++;
				set.insert(pi->tag);
				if (pi->add.length() != 0) {
					setA.insert(pi->tag);
				}

				for (i = 0; i < sz; i++) {
					if (cmpnocase(pi->tag, TN[i])) {
						value[i] = pi->value;
						//assert(pi->value.length()>0);
						break;
					}
				}

			}

			for (i = 0; i < sz; i++) {
				if (value[i].length() == 0) {
					break;
				}
			}
			if (i == sz) {
				//parse score

				valid = true;

				s = value[0];
				score = -1;
				if (s.substr(0, 3) == "NS ") {
					w = value[0].c_str();
					score = strtol(w + 3, &q, 10);
					;
					if (q != w + strlen(w)) {
						println("[%s]", w);
						break;
					}
				}

				s = value[1];
				w = s.c_str();
				assert(isdigit(*w));
				contract = *w - '0';
				assert(contract >= 1 && contract <= 7);
				w++;
				w1 = strchr(SUITS_UP, *w);
				assert(w1!=NULL);
				if (*w == 'N') {
					trump = NT;
					w++;
					assert(*w == 'T');
				}
				else {
					trump = w1 - SUITS_UP;
				}
				w++;
				doubleRedouble = 0;
				if (*w != 0) {
					assert(*w == 'X');
					w++;
					doubleRedouble = 1;
					if (*w != 0) {
						assert(*w == 'X');
						doubleRedouble = 2;
						w++;
						assert(*w == 0);
					}
				}

				w = value[2].c_str();
				result = strtol(w, &q, 10);
				if (q != w + strlen(w)) {
					valid = false; //"[^12]"
					break;
				}

				//declarer
				s = value[4];
				w = s.c_str();
				if (s.length() != 1) {
					assert(s.length() == 2);
					assert(*w == '^');
					w++;
				}
				w = strchr(DECLARER, *w);
				assert(w);
				declarer = w - DECLARER;

				vulnerable = INDEX_OF(value[3],VULNERABLE );
				assert(vulnerable != -1);

				if (valid) {
					/* contract 1-7
					 * trump
					 * tricks 0-13
					 * doubleRedouble=0 simple game; =1 double; =2 redouble
					 */
					assert(score != -1);
					countscore = countBridgeScore(contract, trump, result, doubleRedouble,
							declarer, vulnerable);
					if (declarer % 2 == 1) {
						countscore = -countscore;
					}
					if (countscore != score) {
						s = "";
						for (i = 0; i < sz; i++) {
							s += value[i] + " ";
						}
						s += format("countScore(%d %d %d %d %d %d)=%d != %d declarer=%c",
								contract, trump, result, doubleRedouble, declarer, vulnerable,
								countscore, score, DECLARER[declarer]);
						errors++;
						println("%s", s.c_str());
					}

					validProblems++;
				}
			}

		}
	}

	println(
			"files %d, problems %d, tags %d, set %d, setA %d, validProblems %d errors %d\n",
			files, problems, tags, set.size(), setA.size(), validProblems, errors);
}
#endif

#ifndef NDEBUG
void Frame::explorePbnDirectory(const char* url) {
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
			explorePbnDirectory(name);
		}
		else {
			m_pbns.push_back(name);
		}
	}
}
#endif

#ifndef NDEBUG
void Frame::setBtsContracts() {
	GDir *dir;
	const gchar *filename;
	std::string s;
	gchar name[PATH_MAX];
	VString v;
	ProblemVector pv;
	VProblemI it;
	int i, j;
	const char*p;
	int found = 0;
	VFS vs;
	VFSCI vit;
	FILE*f;

	const char url[] = "D:\\slovesno\\MyProjects\\eclipse\\bridge_cpp\\bridge\\problems\\bts\\GeorgeCoffin";
	const std::string tricks[] = {
			"seven",
			"eight",
			"nine",
			"ten",
			"eleven",
			"twelve",
			"thirteen" };
	const int tricksSize = SIZE(tricks);

	const std::string numbers[] = {
			"one",
			"two",
			"three",
			"four",
			"five",
			"six",
			"seven" };
	const int numbersSize = SIZE(numbers);

	const std::string suits[] = { "club", "diamond", "heart", "spade", "no-trump", }; //one club
	const int suitsSize = SIZE(suits);

	dir = g_dir_open(url, 0, 0);
	if (dir == NULL) {
		assert(0);
		return;
	}
	while ((filename = g_dir_read_name(dir))) {
		sprintf(name, "%s%c%s", url, G_DIR_SEPARATOR, filename);
		if (isDir(name)) {
			assert(0);
		}
		else {
			pv.set(name, true);
		}
	}

	for (i = 0; i < tricksSize; i++) {
		vs.push_back(FS(tricks[i] + " tricks", i + 1));
	}
	for (i = 7; i <= 13; i++) {
		vs.push_back(FS(format("%d", i) + " tricks", i - 6));
	}

	for (i = 1; i <= 7; i++) {
		vs.push_back(FS(format("%d", i) + "NT", i));
	}

	for (i = 0; i < numbersSize; i++) {
		vs.push_back(FS("make " + numbers[i], i + 1));
	}

	for (i = 0; i < numbersSize; i++) {
		for (j = 0; j < suitsSize; j++) {
			vs.push_back(FS(numbers[i] + " " + suits[j], i + 1));
		}
	}

	p = strrchr(url, '\\');
	assert(p!=NULL);
	sprintf(name, "%s.bts", p + 1);
	f = fopen(name, "w+");
	assert(f);

	for (it = pv.begin(); it != pv.end(); it++) {
		p = it->m_comment.c_str();

		for (vit = vs.begin(); vit != vs.end(); vit++) {
			if (it->m_comment.find(vit->s) != std::string::npos) {
				break;
			}
		}
		if (vit == vs.end()) {
			println(p);
			it->m_contract = NO_CONTRACT_SET;
		}
		else {
			found++;
			it->m_contract = vit->value;
			if (!(it->m_contract >= 1 && it->m_contract < 8)) {
				println("%d", it->m_contract);
			}
			assert(it->m_contract >= 1 && it->m_contract < 8);
		}

		fprintf(f, "%s",
				it->getContent(FILE_TYPE_BRIDGE, it - pv.begin() + 1,pv.size()).c_str());

	}

	fclose(f);

	println("problems %d, errors %d, found %d, not found %d", pv.size(),
			pv.esize(), found, pv.size() - found);

}
#endif

#ifndef NDEBUG
void Frame::insertRemoveMenuId(int id, bool insert) {
	/* subfolder with lng files
	 * do not use "lng" dir because config
	 * couldn't read language file
	 */
	const char url[] = "lng1";

	GDir *dir;
	const gchar *filename;
	char buff[PATH_MAX];
	char b1[PATH_MAX];
	char*p, *p1;
	int i;
	int c = 0; //count of real symbols, which need to translate
	FILE*in, *out;
	bool atLeast1FileFound = false;

	dir = g_dir_open(url, 0, 0);
	if (dir == NULL) {
		println("ERROR cann't open dir %s", url);
		return;
	}

	while ((filename = g_dir_read_name(dir))) {
		sprintf(buff, "%s%c%s", url, G_DIR_SEPARATOR, filename);
		p = strrchr(filename, '.');
		assert(p);
		if (isDir(buff) || !cmp(p + 1, "lng")) {
			continue;
		}
		atLeast1FileFound = true;

		c = 0;
		sprintf(b1, "%s%c1%s", url, G_DIR_SEPARATOR, filename);
		rename(buff, b1);
		in = fopen(b1, "r");
		assert(in);
		out = fopen(buff, "w+");
		assert(out);

		while (fgets(buff, PATH_MAX, in)) {
			p = strchr(buff, '\"');
			if (p) {
				p1 = strrchr(buff, '\"');
				assert(p1 > p);
				c += p1 - p;
			}
			i = int(strtol(buff, &p, 10));
			//i>=1000 variable items manage manually
			if (p > buff && i >= id && i < 1000) {
				if (i == id) {
					if (insert) {
						fprintf(out, " %d \"\"\n", i);
					}
					else {
						continue;
					}
				}
				if (insert) {
					i++;
				}
				else {
					i--;
				}
				for (p1 = buff; *p1 != 0 && !isdigit(*p1); p1++)
					;
				fprintf(out, "%.*s%d%s", int(p1 - buff), buff, i, p);
			}
			else {
				fprintf(out, "%s", buff);
			}
		}

		fclose(in);
		fclose(out);
		remove(b1);
		println("proceed %s iseful chars=%d", filename, c);
	}

	if (!atLeast1FileFound) {
		println("ERROR no files to proceed found");
	}
}
#endif

#ifndef NDEBUG
int Frame::getOptimal(int n, bool printToFile) {
	int i, j, k = 0, size;
	uint64_t sum, o = 0;
	int so = 0;

	FILE*f = NULL;

	if (printToFile) {
		f = fopen("out.csv", "w");
		assert(f);
	}

	for (size = 2; size < n; size++) { //button is +size or -size
		sum = 0;
		for (i = 1; i < n; i++) {
			j = i % size;
			k = i / size;

			//n times "size" and
			//clicks[i]=k+std::min(j,1+size-j);
			//times=2*(n-i);
			assert(1 + size - j >= 0);
			sum += (k + std::min(j, 1 + size - j)) * (2 * (n - i));
		}
		assert(sum > 0);
		if (sum < o || o == 0) {
			o = sum;
			so = k;
		}
		else if (!printToFile) {
			break;
		}

		if (printToFile) {
			fprintf(f, "%2d;%.4lf\n", size, sum / double(n * n));
		}
	}

	if (printToFile) {
		fclose(f);
	}

	assert(so > 0);
	return so;

}
#endif

#ifndef NDEBUG
void Frame::optimalToFile() {
	int i, o;
	const int MAX = 100 * 1000;
	const int STEP = 500;

	FILE*f = fopen("outOptimal.csv", "w");
	assert(f);
	fprintf(f, "n;optimal;sqrt\n");

	clock_t begin = clock();
	printf("count optimal %dk, step=%d\n", MAX / 1000, STEP);

	for (i = STEP; i <= MAX; i += STEP) {
		o = getOptimal(i, false);
		fprintf(f, "%d;%d;%.4lf\n", i, o, sqrt(i));

		if (i % (STEP * 30) == 0) {
			printf("%dk time %5.2lf\n", i / 1000,
					double(clock() - begin) / CLOCKS_PER_SEC);
			fflush(stdout);
		}
	}

	fclose(f);
	printf("the end %dk time %5.2lf\n", i / 1000,
			double(clock() - begin) / CLOCKS_PER_SEC);
}
#endif

#ifndef NDEBUG
const char* HTTP_SUITS[] = { "&#9824", "&#9829", "&#9830", "&#9827" };
const char HTTP_BYDR[] = "by_DR.html";
#endif

#ifndef NDEBUG
void Frame::loadHttpProblems() {
#define SITE_SECTION 4
	int i, j;
	FILE*f;
	char*content;
	const char*p, *q;
	VString v;
	VStringCI it;
	std::string s, s1, s2, s3, sn, cards[4];
	GFile* gf;
	GError* error;
	gsize length;
	Problem problem;

	problem.m_gameType = BRIDGE;
	println("begin %s", __func__);

	//                           0        1         2          3           4           5
	const char* LINK_PAGE[] = {
			"Archive",
			"Coffin",
			"Bergholt",
			"Yarborough",
			"Pachabo",
			"CompetitionProblems" };
	assert(SITE_SECTION>=0 && SITE_SECTION<SIZE(LINK_PAGE));

	const char HREF[] = "a href=\"";
	const char BEGIN[] = "U-Page S-Target startspan --><nobr>";
	const char URL[] = "http://www.doubledummy.net/";
	const char BK[] = "</blockquote>";

#if SITE_SECTION==4
	//for http://www.doubledummy.net/Pachabo***.html problems
	const char* ALTERNATIVE_NAMES[] = { "major", "doctor", "colonel", "judge" };
#elif SITE_SECTION==5
	const char* INVALID_PROBLEMS[]= {"CompetitionProblem0003.html","CompetitionProblem0096b.html"};
#endif

	/* Hugh Darwen change some pages on references from
	 * http://www.doubledummy.net/Archive.html
	 * http://www.doubledummy.net/Coffin.html
	 * http://www.doubledummy.net/CompetitionProblems.html
	 * so parser can be incorrect
	 *
	 * finally tested on next links
	 * http://www.doubledummy.net/Bergholt.html
	 * http://www.doubledummy.net/Yarborough.html
	 * http://www.doubledummy.net/Pachabo.html
	 */
	s = URL;
	s += LINK_PAGE[SITE_SECTION];
	s += ".html";
	gf = g_file_new_for_uri(s.c_str());
	error = NULL;
	content = NULL;
	if (g_file_load_contents(gf, NULL, &content, &length, NULL, &error)) {
		g_object_unref(gf);
	}
	else {
		g_object_unref(gf);
		assert(0);
	}

	p = strstr(content, BEGIN);
	assert(p);
	p += strlen(BEGIN);

	while ((p = strstr(p, HREF)) != NULL) {
		p += strlen(HREF);
		q = strchr(p, '\"');
		assert(q);
		v.insert(v.begin(), std::string(p, q - p));
	}
	g_free(content);

	f = fopen(format("%s.bts", LINK_PAGE[SITE_SECTION]).c_str(), "w+");
	assert(f);
	fprintf(f, "%s %s", BTS_SIGNATURE, CURRENT_VERSION_STR.c_str());
	for (it = v.begin(); it != v.end(); it++) {
#if SITE_SECTION==5
		for(i=0;i<SIZEI(INVALID_PROBLEMS);i++) {
			if(cmp(*it,INVALID_PROBLEMS[i])) {
				break;
			}
		}
		if(i<SIZEI(INVALID_PROBLEMS)) {
			continue;
		}
#endif
		gf = g_file_new_for_uri((URL + *it).c_str());			//f is always not null
		error = NULL;
		content = NULL;

		for (p = it->c_str(); !isdigit(*p) && *p != 0; p++)
			;
		assert(*p != 0);
		sn = p;

		/*
		 if ( (it-v.begin()) % 100==1){
		 println("%s %.1lf%% %d",sn.c_str(),(it-v.begin())*100./v.size(),it-v.begin() );
		 }
		 */

		if (g_file_load_contents(gf, NULL, &content, &length, NULL, &error)) {
			//DR
			p = content;
			s = "";
#if SITE_SECTION==5
			if(*it=="CompetitionProblem0130c.html") {
				s1="No DR";
			}
			else if(*it=="CompetitionProblem0060.html") {
				s1="DR?";
			}
			else if(*it=="CompetitionProblem0133a.html") {
				s1="DR6";
			}
			else if(*it=="CompetitionProblem0133b.html") {
				s1="DR3";
			}
			else if(*it=="CompetitionProblem0136a.html") {
				s1="DR7";
			}
			else {
				s1=getNotTaggedAfter(&p,HTTP_BYDR,1,"\n");
			}
#else
			s1 = getNotTaggedAfter(&p, HTTP_BYDR, 1, "\n");
#endif

			//cards
			for (j = 0; j < 4; j++) {
				cards[j] = "";
				for (i = 0; i < 4; i++) {
#if SITE_SECTION==0
					int tt[]= {0,1,2,3};
					if(*it=="Problem0240.html" && j==0) {
						tt[1]=2;
						tt[2]=1;
					}

					if(*it=="Problem0002.html") {
						s2= i==3 ? "</td>" : "</p>";
					}
					else {
						s2="\n";
					}
					s2=getNotTaggedAfter(&p,HTTP_SUITS[tt[i]],0, s2.c_str() );
					//println("[%s]",s2.c_str());
#elif SITE_SECTION==2 || SITE_SECTION==3 || SITE_SECTION==4
					s2 = getNotTaggedAfter(&p, HTTP_SUITS[i], 0,
							i == 3 ? "</td>" : "</p>");
#else
					s2=getNotTaggedAfter(&p,HTTP_SUITS[i],0,"\n");
#endif
					s2 = replaceAll(s2, " ", "");
					if (cmpnocase(s2, "none")) {
						s2 = "";
					}
					cards[j] += replaceAll(s2, "10", "T");

					if (i != 3) {
						cards[j] += '.';
					}
				}
			}
#if SITE_SECTION==0
			if(*it=="Problem0452.html") {
				// remove 2c from West error on web page "jt7.aj..qj8765432" -> "jt7.aj..qj876543"
				cards[1]="JT7.AJ..QJ876543";
			}
#endif

			const int t[] = { 0, 2, 3, 1 };
			for (j = 0; j < 4; j++) {
				s += cards[t[j]];
				s += " ";
			}

			s2 = s.substr(0, s.length() - 1);
			std::transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

			for (i = 0; i < 52; ++i) {
				problem.m_states[0].m_cid[i] = CARD_INDEX_ABSENT;
			}
			try {
				problem.parseDealString(s2.c_str(), CARD_INDEX_NORTH);
			}
			catch (ParseException& ex) {
				println("problem skipped %s %d %s [%s]", (URL + *it).c_str(),
						it - v.begin(), ex.getErrorString().c_str(), s2.c_str());
				continue;
			}

			//comment
			q = NULL;
			p = content;
			while ((p = strstr(p, BK)) != 0) {
				p += strlen(BK);
				if (q != NULL && p > q + 100) {			//some problems have two BK - blocks
					break;
				}
				q = p;
			}
			assert(q);

			p = q;
			while (strchr("\r\n", *p) != 0) {
				p++;
			}

			//comment
			s3 = getNotTaggedAfter(&p, NULL, 0, "</p>");
			//remove leading spaces
			for (s2 = "", p = s3.c_str(); *p == ' '; p++)
				;
			//sometimes several spaces in the row, remove them
			for (; *p != 0; p++) {
				if (p[0] == ' ' && p[1] == ' ') {
					continue;
				}
				s2 += *p;
			}
//			println("[%s]",s2.c_str());

#if SITE_SECTION==4
			for (i = 0; i < SIZEI(ALTERNATIVE_NAMES); i++) {
				s2 = replaceAll(s2,
						format("The %c%s", toupper(ALTERNATIVE_NAMES[i][0]),
								ALTERNATIVE_NAMES[i] + 1), LEADER[i]);
				s2 = replaceAll(s2,
						format("the %c%s", toupper(ALTERNATIVE_NAMES[i][0]),
								ALTERNATIVE_NAMES[i] + 1), LEADER[i]);
				if (strstr(s2.c_str(), "to to") != NULL) {
					println("%s", (URL + *it).c_str())
				}
				s2 = replaceAll(s2, "to to", "to");
			}

			s3 = s2;
			std::transform(s3.begin(), s3.end(), s3.begin(), ::tolower);

			for (i = 0; i < SIZEI(ALTERNATIVE_NAMES); i++) {
				if (strstr(s3.c_str(), ALTERNATIVE_NAMES[i]) != NULL) {
					println("[%s][%s][%s]", s2.c_str(), s3.c_str(), it->c_str())
				}
				assert(strstr(s3.c_str(),ALTERNATIVE_NAMES[i])==NULL);
			}
#endif
//			println("[%s]",s3.c_str());

//			break;
			s += s2 + " " + s1 + " " + (URL + *it);
			stringToBtsFile(f, s, it - v.begin() + 1);
		}
		else {
			println("error %s", it->c_str());
			g_free(content);
			g_object_unref(gf);
			break;
		}
		g_free(content);
		g_object_unref(gf);
	}
	fclose(f);

	println("end %s %s problems %d", __func__, LINK_PAGE[SITE_SECTION], v.size());
}
#endif

#ifndef NDEBUG
void Frame::test() {
}
#endif

#ifndef NDEBUG
std::string Frame::getNotTaggedAfter(const char**content, const char* find,
		int startc, const char*stop) {
	int i;
	std::string e, e1;
	std::string s;
	const char*p = (find == NULL ? *content : strstr(*content, find));

	if (!p && cmp(find, HTTP_BYDR)) {
		p = strstr(*content, "DR?");
	}

	if (!p) {
		println("%s [%.*s]", find, 60, *content);
	}
	assert(p);

	if (find != NULL) {
		p += strlen(find);
	}

	//println("[%d] [%s] [%.*s]",startc,stop,160,p);
	for (i = startc; !startsWith(p, stop); p++) {
		if (*p == '<') {
			i++;
			continue;
		}
		if (*p == '>') {
			i--;
			continue;
		}

		assert(i >= 0);

		if (i == 0) {
			if (*p != '\r') {
				s += *p == '\n' ? ' ' : *p;			//'\n' appears in comment
			}
		}
	}
	s = replaceAll(s, "&nbsp;", "");
	for (i = 0; i < 4; i++) {
		e = "";
		e += toupper(SUITS_CHAR[i]);

		e1 = HTTP_SUITS[i];
		e1 += ' ';
		s = replaceAll(s, e1, e);

		e1 = HTTP_SUITS[i];
		e1 += ';';
		s = replaceAll(s, e1, e);

		e1 = HTTP_SUITS[i];
		s = replaceAll(s, e1, e);
	}
	if (s.length() > 0 && s[0] == ';') {
		s = s.substr(1);
	}
	if (s.length() > 0 && s[s.length() - 1] == ' ') {
		s = s.substr(0, s.length() - 1);
	}

	*content = p;
	return s;
}
#endif

#ifndef NDEBUG
void Frame::correctLngFiles() {
	const int id[] = { 12, 13, 23, 36, 37, 38, 39, 52 };

	/* subfolder with lng files
	 * do not use "lng" dir because config
	 * couldn't read language file
	 */
	const char url[] = "lng1";

	GDir *dir;
	const gchar *filename;
	char buff[PATH_MAX];
	char b1[PATH_MAX];
	char*p;
	int i, j;
	FILE*in, *out;
	bool atLeast1FileFound = false;

	dir = g_dir_open(url, 0, 0);
	if (dir == NULL) {
		println("ERROR cann't open dir %s", url);
		return;
	}

	while ((filename = g_dir_read_name(dir))) {
		sprintf(buff, "%s%c%s", url, G_DIR_SEPARATOR, filename);
		p = strrchr(filename, '.');
		assert(p);
		if (isDir(buff) || !cmp(p + 1, "lng")) {
			continue;
		}
		atLeast1FileFound = true;

		sprintf(b1, "%s%c1%s", url, G_DIR_SEPARATOR, filename);
		rename(buff, b1);
		in = fopen(b1, "r");
		assert(in);
		out = fopen(buff, "w+");
		assert(out);

		j = 0;
		while (fgets(buff, PATH_MAX, in)) {
			p = strchr(buff, '\"');
			i = int(strtol(buff, &p, 10));
			if (p > buff && i == id[j]) {
				fprintf(out, "%d \"\"\n", i);
				j++;
			}
			else {
				fprintf(out, "%s", buff);
			}

		}
		if (j != SIZEI(id)) {
			println("ERROR j!=SIZEI(id) %d %d", j, SIZEI(id));
		}

		fclose(in);
		fclose(out);
		remove(b1);
	}

	if (!atLeast1FileFound) {
		println("ERROR no files to proceed found");
	}
}
#endif

#ifndef NDEBUG
void Frame::stringToBtsFile(FILE*o, std::string& _s, int deal) {
	int i, j, k;
	const int BSIZE = 1024;
	char b[BSIZE], bl[BSIZE];
	char*p, *q;
	std::string s, name, comment;

	const char* LEADERL[] = { "north", "east", "south", "west" };

	const std::string numbers[] = {
			"one",
			"two",
			"three",
			"four",
			"five",
			"six",
			"seven" };
	const int numbersSize = SIZE(numbers);

	const std::string suits[] = { "club", "diamond", "heart", "spade", "no-trump", };	//one club
	const int suitsSize = SIZE(suits);

	const char* LEAD[] = { " to lead", " on lead", "’s best lead", " is on lead" };
	const char* MAKE[] = {
			" to make",
			" make",
			" can make",
			"'s contract",
			" is in",
			" is declarer",
			", as declarer" };

	sprintf(b, "%s", _s.c_str());
	std::transform(b, b + strlen(b) + 1, bl, ::tolower);//+1 b[strlen(b)]=0 need to copy also

	fprintf(o, "\ndeal %d ", deal);
	p = strrchr(b, '/');
	assert(p);
	p++;
	name = std::string(p, strlen(p) - 1);			//remove last '\n'

	p = strchr(b, ' ');
	assert(p);
	p++;
	for (i = 0; i < 3; i++) {
		p = strchr(p, ' ');
		assert(p);
		p++;
	}
	comment = p;			//use later

	fprintf(o, "%.*s\ncontract ", int(p - b - 1), b);

	q = strstr(p, "DR");
	assert(q);

	sprintf(b, "%.*s", int(q - p), p);

	for (i = 0; i < numbersSize; i++) {
		p = strstr(b, numbers[i].c_str());
		if (p != NULL) {
			break;
		}
	}
	fprintf(o, "%c",
			i == numbersSize ? NO_CONTRACT_OR_NO_TRUMP_CHAR : char(i + '1'));

	for (i = 0; i < suitsSize; i++) {
		p = strstr(bl, suits[i].c_str());
		if (p != NULL) {
			break;
		}
	}

	if (i == suitsSize) {
		println("%s", comment.c_str());
	}

	fprintf(o, "%c",
			i == suitsSize ? NO_CONTRACT_OR_NO_TRUMP_CHAR : toupper(suits[i][0]));
	if (i == 4) {
		fprintf(o, "T");
	}

	for (p = b; p[1] != 0; p++) {
		if (isupper(*p) && (isupper(p[1]) || isdigit(p[1]))) {
			break;
		}
	}

	if (p[1] != 0) {
		if (p[1] == '1') {
			assert(p[2] == '0');
		}
		fprintf(o, "\nplay %c%c", *p, p[1] == '1' ? 'T' : p[1]);
	}
	else {
		//println("%d",int(p[1]))
		/* check lead at first "South to lead with spades trumps. North-South to make eight tricks"
		 * found "South to make" & "South to lead" means south lead
		 */

		for (k = 0; k < 2; k++) {
			for (i = 0; i < SIZEI(LEADERL); i++) {
				//for(j=0;j< SIZEI(l==0 ? LEAD : MAKE );j++){//IT'S ERROR
				for (j = 0; j < (k == 0 ? SIZEI(LEAD) : SIZEI(MAKE)); j++) {
					if (strstr(bl,
							format("%s%s", LEADERL[i], k == 0 ? LEAD[j] : MAKE[j]).c_str())) {
						goto l1863;
					}
				}
			}
		}

		l1863: if (k == 2) {
			println("ERROR %s", comment.c_str());
		}
		else {
			fprintf(o, "\nplay %c", toupper(LEADERL[(i + k) % 4][0]));
		}
	}

	fprintf(o, "\nturns %d", p[1] == 0 ? 0 : 1);

	s = replaceAll(comment, "  ", " ");
	fprintf(o, "\ncomment %s\n", s.c_str());
}
#endif

gboolean Frame::exit() {
	if (!saveIfModified()) {
		return FALSE;
	}

	gint x, y;
	gtk_window_get_position(GTK_WINDOW(getWidget()), &x, &y);
	gconfig->save(getGameType(),x,y);

	gdraw->stopCountThread();

	return TRUE;
}

gboolean Frame::saveIfModified() {
	if (getProblem().m_filepath != "" && gconfig->m_documentModifiedWarning
			&& isModified()) {
		SaveModifiedWarningDialog dialog;
		if (dialog.getReturnCode() == GTK_RESPONSE_CANCEL
				|| dialog.getReturnCode() == GTK_RESPONSE_DELETE_EVENT) {
			//not close window, if click 'cancel' or close GTK_RESPONSE_DELETE_EVENT
			return FALSE;
		}
		updateModified();
	}

	return TRUE;
}

void Frame::allocated() {
	//not need further proceeding
	g_signal_handler_disconnect(getWidget(), m_sizeAllocateSignal);

	GdkWindow *gdk_window = gtk_widget_get_window(getWidget());
	GdkRectangle rect;
	gdk_window_get_frame_extents(gdk_window, &rect);			//with title and borders
	const int delta = rect.height - m_area.getSize().cy;

	//println("%d",delta)
	if (delta == gconfig->m_frameDelta) {
		return;
	}

	gconfig->m_frameDelta = delta;

	if (m_area.getSize().cy > getAreaMaxHeight()) {
		//finish_adjust() doesn't work in may be because it's event proceeding
		//need to call this as rarely as possible
		g_idle_add_full(G_PRIORITY_HIGH, finish_adjust, NULL, NULL);
	}
}

void Frame::finishAdjust() {
	m_problemSelector.initResizeRedraw();
	m_area.initResizeRedraw();
}

void Frame::finishSaveImage() {
	GdkWindow *gdk_window = gtk_widget_get_window(getWidget());

	//need to store with menu because it can have label of ProblemSelector
	//also should store right part of window, because it has comment
	GtkAllocation a;
	gtk_widget_get_allocation(getWidget(), &a);

	GdkPixbuf * pb = gdk_pixbuf_get_from_window(gdk_window, a.x, a.y, a.width,
			a.height);

	std::string ext = getFileInfo(m_saveImagePath, FILEINFO::LOWER_EXTENSION);
	if (ext == "jpeg") {
		gdk_pixbuf_save(pb, m_saveImagePath.c_str(), ext.c_str(), NULL, "quality",
				"100", NULL);
	}
	else {
		gdk_pixbuf_save(pb, m_saveImagePath.c_str(), ext.c_str(), NULL, NULL);
	}
	g_object_unref(pb);
}

void Frame::setCustomSkin() {
	CustomSkinDialog d;
}

void Frame::addWidget(bool add,GtkWidget* container,GtkWidget* child){
	if(add){
		gtk_container_add(GTK_CONTAINER(container), child);
	}
}

void Frame::addRemoveWidget(bool add, GtkWidget *container, GtkWidget *child) {
	if(add){
		gtk_container_add(GTK_CONTAINER(container), child);
	}
	else{
		gtk_container_remove(GTK_CONTAINER(container), child);
	}

}
