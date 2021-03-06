/*
 * ProblemSelector.h
 *
 *       Created on: 27.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef PROBLEMSELECTOR_H_
#define PROBLEMSELECTOR_H_

#include "base/FrameItemArea.h"
#include "helper/SvgParameters.h"
#include "problem/ProblemVectorModified.h"

/* Problem selector and problem description
 */

class ProblemSelector: public FrameItemArea, public ProblemVectorModified {
	GtkWidget*m_label;
	std::string m_filepath; //utf8 m_filepath="" means empty document
	GtkToolItem* m_button[TOOLBAR_BUTTON_SIZE]; //toolbar buttons
	GtkWidget*m_commentView;
public:
	GtkWidget*m_toolbar;
	std::string m_comment;

	CairoSurface m_backgroundFull;

	//this class is first in Frame::m_childs so use deck and skin surface in this class
	CairoSurface m_deck;
	Pixbuf m_arrow[4];
	Pixbuf m_svgArrowPixbuf;
	Pixbuf m_svgDeckPixbuf;
	Pixbuf m_svgScaledPixbuf;
	SvgParameters m_svgDeckParameters[N_VECTOR_DECKS];
	SvgParameters m_svgArrowParameters[N_VECTOR_ARROWS];
	bool m_deckChanged,m_arrowChanged;
	CSize m_bestLineSize;

private:
	bool m_visible;
	guint32 m_lastClickTime;

	void setAreaProblem();

	bool setCheckLabelFit(int option, int width, std::string& s);

	void updateLabel();

	void fullUpdate(); //cann't use update() already in frame

	void setSkin();
	void setDeck();
	void setBestLineSize();
public:
	void createNewGame(GAME_TYPE t, CARD_INDEX a);
	void updateToolbar();
	void updateToolbarButtons();

	ProblemSelector();
	virtual ~ProblemSelector();

	 void draw()override;
	 void init()override;
	 CSize getSize() const override;
	 void updateLanguage()override;
	 void updateSkin()override;
	 void updateDeckSelection()override;
	 void updateFontSelection()override;

	 void updateArrowSelection() override{
		setArrows();
		initResizeRedraw();
	}

	void updateAfterCreation()override;

	std::string getFile() const {
		return m_filepath;
	}

	bool isEmptyFilepath() const {
		return m_filepath.length() == 0;
	}

	bool isModified() const;

	void newGame()override;
	void resize()override;

	void setDeal(bool random)override {
		getProblem().setDeal(random);
		currentProblemChanged(false);
	}

	void updateGameType()override {
		getProblem().changeGameType();
		currentProblemChanged(true);
	}

	void currentProblemChanged(bool gameType);

	void setOriginalModified();

	Problem& getProblem();
	const Problem& getProblem() const;

	const ProblemVector& getProblemVector() const {
		return m_vproblem;
	}
	ProblemVector& getProblemVector() {
		return m_vproblem;
	}

	void setCursorVisible(bool visible);
	void setComment(std::string s, bool updateTextView = true);
	void proceedCommentChanges();
	std::string getCommentTextView() const;
	void updateCommentTextView();

	GtkWidget* getLabel() {
		return m_label;
	}

	void clickToolbar(GtkToolItem* w);
	bool mouseClick(guint32 time);

	void changeShowOption()override;
	void openUris(char**uris)override; //=set
	void openFiles(const char*files); //=set

	//set or add files
	void set(VString const& v, bool add);

	void save(std::string filepath, bool split);

	void save() {
		save(m_filepath, false);
	}

	void setFilepathUpdateRecent(std::string filepath);

	std::string getTitle() const;

	void setFrom(ProblemVectorModified const& pvm);

	void setArrows();

	void fillSvgParameters();
	void drawSvg(CSize const& size,int n,bool isDeck,gdouble value);
	void createSvgPixbufs(bool isDeck);
	double getSvgMaxWHRatio()const;
	SvgParameters& getSvgParameters(int n,bool isDeck);

	void updateThink()override;

};

extern ProblemSelector* gproblemselector;

#endif /* PROBLEMSELECTOR_H_ */
