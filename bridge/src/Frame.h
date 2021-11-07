/*
 * Frame.h
 *
 *   	   Created on: 10.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef FRAME_H_
#define FRAME_H_

#include "base/Config.h"
#include "Menu.h"
#include "Toolbar.h"
#include "DrawingArea.h"
#include "LastTrick.h"
#include "ProblemSelector.h"
#include "CheckNewVersion.h"

const gchar OPEN_FILE_SIGNAL_NAME[] = "bridge_open_file";

class Frame: public FrameItem {
	VFrameItemPointer m_childs;
	gulong m_sizeAllocateSignal;
	std::string m_saveImagePath;
	GtkWidget*m_vbox1,*m_vbox2;
	CheckNewVersion m_newVersion;
public:
	Menu m_menu;
	Toolbar m_toolbar;
	DrawingArea m_area;
	LastTrick m_lastTrick;
	ProblemSelector m_problemSelector;

private:

	void setLanguage(int index);
	void setCustomLanguage();

	//if filepath.length()==0 then open file with filter otherwise open file=filepath
	void openAddFile(MENU_ID id);

	void save();
	void saveAs();
	void solveForAllDeclarers();
	void solveAllDeals();
	void converter();
	void editList();
	void editPbn();
	void editDescription();
	void saveHtml(bool images);
	void saveImage();

public:
	gboolean saveIfModified();
	void callFrameItemFunction(FrameItemFunction f, int option);

	Frame(GtkApplication *application, const char* filepath);
	virtual ~Frame();

	void enableEdit(bool enable);

	void menuClick(MENU_ID id);

	bool proceedSaveAsSaveAs() const {
		return m_problemSelector.isEmptyFilepath();
	}
	void finishSaveFile(std::string filepath, bool split);

	void updateTitle();

	void updateModified() {
		m_menu.setItemAttributes(MENU_SAVE);
		updateTitle();
	}

	void newVesionMessage();

	void changeGameType();

	void setDeal(bool random)override;

#define CALL_FRAME_ITEM_FUNCTION(a,p) callFrameItemFunction(&FrameItem::a,p)

	void newGame() override{
		if (!saveIfModified()) {
			return;
		}
		CALL_FRAME_ITEM_FUNCTION(newGame,0);
		enableEdit(true);
		updateUndoRedo(); //after FrameItem::newGame
		updateFindBestState();
		updateModified(); //also updates title
	}

	void updateLanguage() override{
		gconfig->loadLanguageFile();
		updateTitle();
		CALL_FRAME_ITEM_FUNCTION(updateLanguage, 1);
	}

	void resetSettings();

#define M(a) void a()override{CALL_FRAME_ITEM_FUNCTION(a,0);}
	M(updateAfterCreation)
	M(updateArrowSelection)
	M(updateEdit)
	M(updateGameType)
	M(updateFindBestState)
	M(updateEstimationType)
	//M(newGame)//defined upper
	//M(updateLanguage)//defined upper
	M(updateThink)
	M(updateSkin)
	M(updateUndoRedo)
	M(updateDeckSelection)
	M(updateFontSelection)
	M(updateResetSettings)
#undef M

	void updateRecent(std::string filepath);

	void about();
	void selectArrow();
	void selectDeck();
	void suitsOrder();

	void calculator();

	gboolean exit();

	void allocated();
	void finishAdjust();
	void finishSaveImage();

	void showIfNotVisible() {
		if (!gtk_widget_get_visible(getWidget())) {
			showAll();
		}
	}

	void setCustomSkin();

	static void addWidget(bool add,GtkWidget* container,GtkWidget* child);
	static void addRemoveWidget(bool add,GtkWidget* container,GtkWidget* child);

#ifndef NDEBUG
	struct FS {
		std::string s;
		int value, value1, value2;
		FS(const std::string& p, int v = 0) {
			s = p;
			value = v;
			value1 = value2 = 0;
		}
		bool operator<(const FS&f) const {
			return value > f.value;
		}
	};
	typedef std::vector<FS> VFS;
	typedef VFS::const_iterator VFSCI;

	/* helper functions for pbn files exploration
	 * 1 searching possible tag as set
	 * 2 check
	 */
	VString m_pbns;
	void explorePbn(const char* url, const char* filename);
	void explorePbnDirectory(const char* url);

	/* helper functions for setup contract
	 * (number of tricks) for Darwen, GeorgeCoffin, Competition  deals
	 */
	void setBtsContracts();

	/* iterate through language files and insert or remove "id" from menu
	 * all later items will have id=idOriginal-1 in case of remove
	 * or id==idOriginal+1 in case of insert (insert new item before id)
	 */
	void insertRemoveMenuId(int id, bool insert);

	void insertMenuId(int id) {
		insertRemoveMenuId(id, true);
	}
	void removeMenuId(int id) {
		insertRemoveMenuId(id, false);
	}

	/** for arbitrary 0<= i,j <n
	 * we got
	 * distance 0 - n times
	 * distance 1 - 2*(n-1) times
	 * distance 2 - 2*(n-2) times
	 * ....
	 * distance k - 2*(n-k) times
	 * ....
	 * distance n-1 - 2 times
	 *
	 * so it's weight
	 *
	 * total times =2+4+..+2*(n-1)+n=2(1+2+..+(n-1))+n=2*n(n-1)/2+n=n^2
	 */
	/* to get optimal button size graph for many N=number of problems call optimalToFile();
	 * to get graph for particular N=number of problems call getOptimal(N=1000,true);
	 */
	int getOptimal(int n, bool printToFile);

	void optimalToFile();

	void test();

	//load http://www.doubledummy.net problems and stores them to bts file
	void loadHttpProblems();
	//helper function for loadHttpProblems()
	std::string getNotTaggedAfter(const char**content, const char* find,
			int startc, const char*stop);
	//parse comment and output to FILE in bts format, helper function for loadHttpProblems()
	void stringToBtsFile(FILE*f, std::string& s, int deal);

	void correctLngFiles();

#endif

};

extern Frame* gframe;

#endif /* FRAME_H_ */
