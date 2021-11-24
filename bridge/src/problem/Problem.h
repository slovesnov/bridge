/*
 * Problem.h
 *
 *       Created on: 07.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef PROBLEM_H_
#define PROBLEM_H_

#include "State.h"
#include "Stream.h"
#include "ParseException.h"
#include "PbnEntry.h"

const char BTS_SIGNATURE[] = "% bts version ";
const char BTS_BEGIN_MARKER[] = "deal ";
const char BTS_CONTRACT[] = "contract";
const char BTS_PLAY[] = "play";
const char BTS_PLAYER[] = "player";
const char BTS_ABSENT[] = "absent";
const char BTS_COMMENT[] = "comment";
const char BTS_MISERE[] = "misere";
const char BTS_MISERE_OLD[] = "mizer";
const char BTS_TURNS[] = "turns";
const char BTS_PBNINFO[] = "pbninfo";
const char NO_CONTRACT_OR_NO_TRUMP_CHAR = '?'; //use for bts & html

/* lower versions defined as constants, because they are used for parsing,
 * and upper versions used for getContent()
 * upper versions DF_BEGIN_MARKER_L[]="deal:" -> DF_BEGIN_MARKER[]="Deal:"
 * defined in Problem.cpp and counted automatically
 */
const char DF_BEGIN_MARKER_L[] = "deal:";
const char DF_CONTRACT_L[] = "contract:";
const char DF_ONLEAD_L[] = "onlead:";
const char DF_LEAD_L[] = "lead:";
const char DF_RESULT_L[] = "result:";
const char DF_COMMENTARY_L[] = "commentary:";

const char PROBLEM_HTML_BEGIN[] = "<html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body>";
const char PROBLEM_HTML_END[] = "</body></html>";

class Problem {
private:
	/* only for inner using. on parsing fill and then make moves to fill all states
	 * before storing count m_firstCard to getXXContent() functions
	 */
	int m_firstCard[TOTAL_STATES];

public:
	std::string m_content; //parsed content (locale, not utf8)

	std::string m_comment; //utf8
	std::string m_filepath; //utf8 - full file path in Frame, in vector special names
	GAME_TYPE m_gameType;
	State m_states[TOTAL_STATES];
	int m_currentState;
	int m_maxState;
	int m_trump;
	/* contract as it in pbn, df files 1-7, PASS_CONTRACT means all players say "pass" in pbn file contract="pass"
	 * for preferans m_contract=[6,10]
	 */
	int m_contract;
	VPbnEntry m_pbnEntry;

	//for preferans
	CARD_INDEX m_player;
	bool m_misere;
	CARD_INDEX m_absent;
	CARD_INDEX m_preferansPlayer[3];

	void setAbsent(CARD_INDEX absent);

private:
	std::string getDfContent(int nproblem = 1) const;
	std::string getPbnContent(int nproblem, bool caption); //can change m_pbnEntry so not const
	std::string getBtsContent(int nproblem = 1, bool caption = true); //can change m_pbnEntry so not const

	/* always should call initialization before and check after, so make
	 * this functions private, use parse() function
	 */
	void parsePbn(const std::string& s);
	void parseDf(const std::string& s);
	void parseBts(const std::string& s);
	void parseBtsOldFormat(const std::string& s, GAME_TYPE gameType);

	//========== BEGIN HELRER PARSER FUNCTIONS ==================================
public:
	//string should be in lowercase
	void parseDealString(const char*s, CARD_INDEX player);
private:
	//string should be in lowercase
	void parsePlayString(const char*s);
	//string should be in lowercase
	const char* parseDfHand(const char*s, CARD_INDEX player);

	//helper function for Df format
	std::string getDfContentHelper(int n) const;

	void fillPbnTags(const std::string& s);

	void checkFirstCardsRepeat();
	//========== END HELRER PARSER FUNCTIONS ====================================

	//========== BEGIN MOVE FUNCTIONS ===========================================

	bool isTableFullOrEmpty(int stateIndex) const {
		assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
		const State & s = m_states[stateIndex];
		const int i = s.countInnerCards();
		return i == 0 || i == maxTableCards();
	}

	bool compare(int index1, int index2) const;

	CARD_INDEX dealFirst() const {
		return
				isPreferans() && m_absent == CARD_INDEX_NORTH ?
						CARD_INDEX_EAST : CARD_INDEX_NORTH;
	}
public:
	void makeMove(int index, int stateIndex);

	inline bool isTableFull() const {
		return isTableFull(m_currentState);
	}

	inline bool isTableFull(int stateIndex) const {
		assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
		const State & s = m_states[stateIndex];
		return s.countInnerCards() == maxTableCards();
	}

	CARD_INDEX getNextMove() const {
		return getNextMove(m_currentState);
	}

	CARD_INDEX getNextMove(int stateIndex) const;
	bool isValidTurn(int index, int stateIndex, bool drawarea = false);
	CARD_INDEX getTaker(const int inner, int stateIndex) const;
	int getLeadingSuit(int stateIndex) const;

	bool hasLeadingSuit(CARD_INDEX player, int stateIndex) const {
		assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
		const State & s = m_states[stateIndex];
		return s.hasSuit(player, getLeadingSuit(stateIndex));
	}

	inline int maxTableCards() const {
		return MAX_TABLE_CARDS[m_gameType];
	}

	inline CARD_INDEX getFirst() const {
		return isTableFull() ? getNextMove() : getFirstMove();
	}

	void getClearCid(CARD_INDEX*cid) const;
	//========== END MOVE FUNCTIONS =============================================
	static void staticInit();

	//type=0 just create, type=1 newGame, type=2 randomDeal
	Problem(GAME_TYPE gt = BRIDGE, CARD_INDEX a = CARD_INDEX_SOUTH, int type = 0);
	const Problem& operator=(const Problem& p);
	bool operator==(const Problem& p) const;
	bool operator!=(const Problem& p) const {
		return !(*this == p);
	}

	VPbnEntryI findPbnEntry(const char* tag);
	std::string getPbnTagsString() const;

	VPbnEntryCI pbnEntryBegin() const {
		return m_pbnEntry.begin();
	}

	VPbnEntryI pbnEntryBegin() {
		return m_pbnEntry.begin();
	}

	VPbnEntryCI pbnEntryEnd() const {
		return m_pbnEntry.end();
	}

	VPbnEntryI pbnEntryEnd() {
		return m_pbnEntry.end();
	}

	int pbnEntrySize() const {
		return m_pbnEntry.size();
	}

	void newDeal();
	void randomDeal();

	void setDeal(bool random) {
		if (random) {
			randomDeal();
		}
		else {
			newDeal();
		}
	}

	void changeGameType() {
		m_gameType = isBridge() ? PREFERANS : BRIDGE;
		newDeal();
	}

	void saveState();

	std::string getEnglishTrumpString() const;

	std::string getValidTrumpStringDfPbn() const;
	std::string getValidContractStringDfPbn() const;

	/* name getValidContractTrumpStringDfPbn() not getValidTrumpContractStringDfPbn()
	 * because contract goes first
	 */
	std::string getValidContractTrumpStringDfPbn() const {
		return getValidContractStringDfPbn() + getValidTrumpStringDfPbn();
	}

	inline State const& getState() const {
		assert(m_currentState >= 0 && m_currentState < TOTAL_STATES);
		return m_states[m_currentState];
	}

	inline State& getState() {
		assert(m_currentState >= 0 && m_currentState < TOTAL_STATES);
		return m_states[m_currentState];
	}

	inline State& getNextState() {
		assert(m_currentState + 1 >= 0 && m_currentState + 1 < TOTAL_STATES);
		return m_states[m_currentState + 1];
	}

	inline State& getPreviousState() {
		assert(m_currentState - 1 >= 0 && m_currentState - 1 < TOTAL_STATES);
		return m_states[m_currentState - 1];
	}

	void rotate(bool clockwise, bool likeBridge);
	//index is changed so use reference
	void rotate(CARD_INDEX& index, bool clockwise, bool likeBridge) const;

	inline bool isBridge() const {
		return m_gameType == BRIDGE;
	}

	inline bool isPreferans() const {
		return m_gameType == PREFERANS;
	}

	inline int minContract() const {
		return MIN_CONTRACT[m_gameType];
	}

	inline int maxContract() const {
		return MAX_CONTRACT[m_gameType];
	}

	inline CARD_INDEX getFirstMove() const {
		return m_states[m_currentState].m_firstmove;
	}

	inline void setFirstMove(CARD_INDEX index) {
		m_states[m_currentState].m_firstmove = index;
	}

	inline CARD_INDEX getVeryFirstMove() const {
		return m_states[0].m_firstmove;
	}

	//0-3
	int getDeclarerInt() const {
		return getDeclarer() - CARD_INDEX_NORTH;
	}

	CARD_INDEX getDeclarer() const;

	void setVeryFirstMove(CARD_INDEX index) {
		m_states[0].m_firstmove = index;
	}

	std::string getHTMLContent(int nproblem, int bestMoveIndex, int tricks[2],
			bool forConverter, int totaproblems) const;

	void parse(FILE_TYPE t, const std::string& s, bool useOldBtsParser = false,
			GAME_TYPE gameType = BRIDGE);
	std::string getContent(FILE_TYPE t, int nproblem, int totaproblems);

	std::string getShortFileName() const;

	//for store functions
	static std::string postproceedHTML(const std::string& s, bool images);
	std::string getHTMLRow(int row, CARD_INDEX player, bool cr) const;
	static std::string htmlWrapInner(int columns, const std::string a[],
			const int* index, int size);
	static std::string htmlWrapNOBR(const std::string& s) {
		return "<span style='white-space:nowrap'>" + s + "</span>";
	}
	std::string getHTMLnorthSouth(int columns, bool north, bool tdAtFirst) const;

	//also allow player=CARD_INDEX_ABSENT
	VString getRowVector(int row, CARD_INDEX player) const;
	std::string getRow(int row, CARD_INDEX player) const;
	std::string getHTMLString(int columns, CARD_INDEX player, bool cr) const;
	//END helper functions to store problem

	//dealer 0-north 1-east ...
	std::string getPbnDealString(CARD_INDEX dealer) const {
		return format("%c:", toupper(PLAYER_CHAR[dealer - 1]))
				+ getBtsDealString(dealer);
	}
	std::string getBtsDealString(CARD_INDEX dealer) const;

	CARD_INDEX getLastTrick(int* moves) const;

	//fill all four cards starts from firstmove
	void fillInner4(int* index) const {
		fillInner4(index, getState().m_firstmove);
	}

	//fill all four cards starts from CARD_INDEX_NORTH
	void fillInner4NorthFirst(int* index) const {
		fillInner4(index, CARD_INDEX_NORTH);
	}

	//fill all four cards starts from 'from'
	void fillInner4(int* index, CARD_INDEX from) const;

	/* user can load problem and make some modifications after that
	 * change trump, contract, result, deal etc...
	 */
	void fillAdjustPbnInfo(int nproblem);
	void adjustPbnInfo();

	int fillFirstCard();

	bool checkAuctionTag();

	bool noTrumpOrContract() const {
		return m_trump == NO_TRUMP_SET || m_contract == NO_CONTRACT_SET;
	}

	bool noTrumpAndContract() const {
		return m_trump == NO_TRUMP_SET && m_contract == NO_CONTRACT_SET;
	}

	bool supportFileFormat(FILE_TYPE t) const;

	DEAL_STATE getDealState(bool checkTrump = true) const;

	bool isSolveAllDealsEnable() const;

#ifndef FINAL_RELEASE
	void printcids(int stateNumber);
#endif

	CARD_INDEX getPreviousPlayer(CARD_INDEX player) const {
		return getPlayer(player, false);
	}

	CARD_INDEX getNextPlayer(CARD_INDEX player) const {
		return getPlayer(player, true);
	}

	CARD_INDEX getPlayer(CARD_INDEX player, bool next, int count = 1) const;

	CARD_INDEX getBasePlayer() const;

#ifndef FINAL_RELEASE
	//used in BridgeTest project to get deal
	std::string getForBridgeTestDealClass(int n)const;
#endif

};

typedef std::vector<Problem> VProblem;
typedef VProblem::iterator VProblemI;
typedef VProblem::const_iterator VProblemCI;

#endif /* PROBLEM_H_ */
