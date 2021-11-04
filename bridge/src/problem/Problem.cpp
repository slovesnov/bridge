/*
 * Problem.cpp
 *
 *       Created on: 07.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Problem.h"
#include "../Frame.h"

#define throwOnError(_Expression,error,addon) if(!(_Expression))throw ParseException(#_Expression,error,__FILE__,__LINE__,__func__,m_content,addon);

const char* CHANGE_HTML_SIMPLE[] = {
		"&spades;",
		"<font color='red'>&hearts;</font>",
		"<font color='red'>&diams;</font>",
		"&clubs;",
		"&uarr;",
		"&rarr;",
		"&darr;",
		"&larr;" };

/* M(BEGIN_MARKER)	char DF_BEGIN_MARKER[SIZE(DF_BEGIN_MARKER_L)];
 * const char DF_BEGIN_MARKER[]="Deal:";
 */
#define M(a) static char DF_##a[SIZE(DF_##a##_L)];
M(BEGIN_MARKER)
M(CONTRACT)
M(ONLEAD)
M(LEAD)
M(RESULT)
M(COMMENTARY)
#undef M

void Problem::staticInit() { //called from Config::Config()
#define M(a) *DF_##a=toupper(*DF_##a##_L);strcpy(DF_##a+1,DF_##a##_L+1);
	M(BEGIN_MARKER)
	M(CONTRACT)
	M(ONLEAD)
	M(LEAD)
	M(RESULT)
	M(COMMENTARY)
#undef M
}

Problem::Problem(GAME_TYPE gt, CARD_INDEX a, int type) {
	int i;

	m_currentState = -1; //have to init
	m_maxState = -1; //have to init

	for (i = 0; i < TOTAL_STATES; i++) {
		m_states[i].clearTricks();
	}

	m_trump = 0;

	m_gameType = gt;
	m_contract = minContract();

	m_player = CARD_INDEX_INVALID;
	m_misere = false;
	setAbsent(a);

	//need for parse functions don't remove
	for (i = 0; i < TOTAL_STATES; i++) {
		m_firstCard[i] = CARD_INDEX_INVALID;
	}

	if (type == 1) {
		newDeal();
	}
	else if (type == 2) {
		randomDeal();
	}
}

std::string Problem::getDfContent(int nproblem) const {
	std::string s, s1, s3;
	int i, j;

	assert(isBridge());

	s = DF_CONTRACT;
	s += " " + getValidContractTrumpStringDfPbn() + "-" + LEADER[getDeclarerInt()]
			+ " ";
	s += getDfContentHelper(0);

	//north cards
	j = s.length();
#define s s1
	s = DF_BEGIN_MARKER + format(" %d", nproblem);
	j -= s.length() - 1;
	for (i = 0; i < j; i++) {
		s += ' ';
	}
	s += getDfContentHelper(1);
#undef s

	//south cards
	j = s.length();
#define s s3
	s = DF_ONLEAD + std::string(" ") + LEADER[getVeryFirstMove() - 1];
	j -= s.length() - 1;
	for (i = 0; i < j; i++) {
		s += ' ';
	}
	s += getDfContentHelper(3);
#undef s

	//east cards
	j = s1.length();
	i = s3.length();
	if (j < i)
		j = i;
	j -= s.length() - 1;
	for (i = 0; i < j; i++)
		s += ' ';
	s += getDfContentHelper(2);

	std::string out = s1 + "\n" + s + "\n" + s3;

	s = format("\n%s ", DF_LEAD);

	//j-leave, because can be four cards on table which produce infinite cycle
	if (m_firstCard[0] == CARD_INDEX_INVALID) {
		s += "";
	}
	else {
		i = m_firstCard[0];
		s += toupper(SUITS_CHAR[i / 13]);
		s += toupper(RANK[i % 13]);
	}

	out += s + format("\n%s \n", DF_RESULT);

	if (!m_comment.empty()) {
		out += DF_COMMENTARY + ("\n" + m_comment);
	}

	out += "\n\n";
	return out;
}

std::string Problem::getPbnContent(int nproblem, bool caption) {
	//if not caption new line from previous task
	std::string s;
	if (caption) {
		s = "% PBN 1.0\n% EXPORT\n%";
	}

	fillAdjustPbnInfo(nproblem);

	return s + "\n" + getPbnTagsString();
}

//in case of preferans  EastWestTricks = number_of_tricks_of_player;
std::string Problem::getHTMLContent(int nproblem, int bestMoveIndex,
		int northSouthTricks, int eastWestTricks, bool forConverter, int totaproblems) const {
	int i;
	int a[4];
	std::string s, q,w, inner[9];
	const CARD_INDEX nextmove = getNextMove();

	if(totaproblems>1){
		w=format("%d", nproblem);
	}
	q = getFileInfo(m_filepath, FILEINFO::SHORT_NAME);
	s = "\n<p id='" + q + w
			+ "'>\n<table border=1 cellspacing=0 cellpadding=1>\n<tr><td valign='top'><table border=0 cellspacing=2 cellpadding=1>\n<tr><td><b>"
			+ getString(MENU_PROBLEM) + "</b><td align='right'>" + q
			+ " "+w + "\n";

	s += "<tr><td>";
	s += getString(STRING_DECLARER);
	s += "<td align='right'>";
	s += getPlayerString(isPreferans() ? m_player : getDeclarer());

	s += "<tr><td>";
	s += getString(STRING_CONTRACT);
	s += "<td align='right'>";

	if (isPreferans() && m_misere) {
		s += getString(STRING_MISERE);
	}
	else {
		if (m_contract == NO_CONTRACT_SET) {
			s += NO_CONTRACT_OR_NO_TRUMP_CHAR;
		}
		else {
			s += format("%d", m_contract);
		}

		if (m_trump == NT) {
			s += getNTString();
		}
		else if (m_trump == NO_TRUMP_SET) {
			s += NO_CONTRACT_OR_NO_TRUMP_CHAR;
		}
		else {
			s += "!";
			s += SUIT_ARROW_CHAR[m_trump];
		}
	}

	s += "\n";

	fillInner4NorthFirst(a);

	/* inner[0-3] outer contour,
	 * inner[4-7] inner contour
	 * inner[8]=&nbsp; just for convenience
	 *
	 *                   inner[0]
	 *                   inner[4]
	 * inner[3] inner[7] inner[8] inner[5] inner[1]
	 *                   inner[6]
	 *                   inner[2]
	 */

	for (i = 0; i < 4; i++) {
		if (a[i] != -1) {
			inner[i + 4] = htmlWrapNOBR(
					getCardRankString(a[i]) + "!" + SUIT_ARROW_CHAR[a[i] / 13]);
		}
	}
	i = INDEX_OF(nextmove,PLAYER);
	inner[a[i] ==-1 ? i + 4 : i] = format("!%c", SUIT_ARROW_CHAR[i + 4]);

	inner[8] = "&nbsp;";

	const int columns = inner[1].length() != 0 || inner[3].length() != 0 ? 5 : 3; //inner table columns

	if (!forConverter && gconfig->m_htmlStoreBestMove) {
		s += "<tr><td>";
		s += getString(STRING_BEST_MOVE);
		s += "<td align='right'>";
		s += getCardRankString(bestMoveIndex % 13);
		s += "!";
		s += SUIT_ARROW_CHAR[bestMoveIndex / 13];
		s += "\n";
	}

	if (!forConverter && gconfig->m_htmlStoreNumberOfTricks) {
		s += "<tr><td>";
		s += getString(STRING_NUMBER_OF_TRICKS);
		s += "<td align='right'>";

		if (isPreferans()) {
			//   EastWestTricks			=PrefPosition::number_of_tricks_of_player;
			s += format("<u>%d</u>/%d\n", eastWestTricks, northSouthTricks); //at first player tricks it's east/west tricks
		}
		else {
			s += format("%d/%d\n", northSouthTricks, eastWestTricks);
		}

	}

	//saved by bridge studio
	w=BASE_ADDRESS;
	if(w.back()=='/'){//remove last '/'
		w=w.substr(0,w.length()-1);
	}
	s += "<tr><td colspan='2'><a href='" + w + "'><small>";
	s += getString(STRING_SAVED_BY_BRIDGE_STUDIO);
	s += "</small></a>\n";

	//comment
	s += "<tr><td colspan='2' width='200' align='justify'><small>";
	if (m_pbnEntry.size() == 0) {
		s += m_comment;
	}
	else {
		s += replaceAll(m_comment, "\n", "<br>");
	}
	s += "</small>";

	bool west = isBridge() || (isPreferans() && m_absent != CARD_INDEX_WEST);
	bool east = isBridge() || (isPreferans() && m_absent != CARD_INDEX_EAST);

	s += "</table>\n<td><table border=0 cellspacing=0>\n";
	s += getHTMLnorthSouth(columns, true, west);
	s += "<tr>";

	if (west) {
		s += getHTMLString(1, CARD_INDEX_WEST, false);
	}

	i = 0;
	s += htmlWrapInner(columns, inner, &i, 1);

	if (east) {
		s += getHTMLString(1, CARD_INDEX_EAST, true);
	}

	for (i = 0; i < 4; ++i) {
		s += "<tr>";
		if (west) {
			s += "<td>";
			s += getHTMLRow(i, CARD_INDEX_WEST, false);
		}

		if (i == 1) {
			const int b[] = { 3, 7, 8, 5, 1 };
			s += htmlWrapInner(columns, inner, b + (columns == 3), columns);
		}
		else {
			const int b[] = { 4, -1, 6, 2 }; //second index never used
			s += htmlWrapInner(columns, inner, &b[i], 1);
		}

		if (east) {
			s += "<td>";
			s += getHTMLRow(i, CARD_INDEX_EAST, true);
		}
	}
	s += getHTMLnorthSouth(columns, false, west);
	s += "</table>\n</table>\n";

	return s;

}

std::string Problem::getHTMLnorthSouth(int columns, bool north,
		bool tdAtFirst) const {
	std::string s;
	auto c = north ? CARD_INDEX_NORTH : CARD_INDEX_SOUTH;
	if (isBridge() || (isPreferans() && m_absent != c)) {
		s += "<tr>";
		if (tdAtFirst) {
			s += "<td>";
		}
		s += getHTMLString(columns, c, true);
		for (int i = 0; i < 4; ++i) {
			s += "<tr>";
			if (tdAtFirst) {
				s += "<td>";
			}
			s += format("<td colspan='%d'>", columns) + getHTMLRow(i, c, true);
		}
	}
	return s;
}

std::string Problem::htmlWrapInner(int columns, const std::string a[],
		const int* index, int size) {
	int i;
	std::string s;
	std::string q[2];

	for (i = 0; i < SIZEI(q); i++) {
		//all columns should have same width
		s = format("<td bgcolor='#f0f0f0' width=16");
		if (i == 1) {
			s += " align='center'";
		}
		q[i] = s + ">";
	}

	assert((columns - size) % 2 == 0);
	assert(columns >= size);
	const int j = (columns - size) / 2;

	s = "";
	for (i = 0; i < columns; i++) {
		if (i - j >= 0 && i - j < size) {
			s += q[1] + a[index[i - j]];
		}
		else {
			s += q[0];
		}
	}

	return s;
}

std::string Problem::getHTMLRow(int row, CARD_INDEX player, bool cr) const {
	return htmlWrapNOBR(
			std::string("!") + SUIT_ARROW_CHAR[getSuitsOrder(row)]
					+ getRow(row, player)) + (cr ? "\n" : "");
}

std::string Problem::getHTMLString(int columns, CARD_INDEX player,
		bool cr) const {
	std::string s = gconfig->getPlayerString(player);
	if (isPreferans() && player == m_player) {
		s = "<u>" + s + "</u>";
	}
	s = "<b>" + s + "</b>";

	if (cr) {
		s += "\n";
	}

	if (columns == 1) {
		return "<td>" + s;
	}
	else {
		return format("<td colspan='%d'>", columns) + s;
	}
}

std::string Problem::getRow(int row, CARD_INDEX player) const {
	std::string s;
	int suit = getSuitsOrder(row);
	bool found = false;
	int j, k;
	for (j = 0; j < 13; j++) {
		k = suit * 13 + (gconfig->getAscending() ? 12 - j : j);
		if (m_states[m_currentState].m_cid[k] == player) {
			found = true;
			s += " ";
			s += getCardRankString(k);
		}
	}
	if (!found) {
		s += " -";
	}
	return s;
}

std::string Problem::getDfContentHelper(int n) const {
	int i, j;
	std::string s = "";
	bool b;
	for (i = 0; i < 4; i++) {
		b = false;
		for (j = i * 13; j < (i + 1) * 13; j++)
			if (m_states[0].m_cid[j] % 4 == n) {
				b = true;
				s += toupper(RANK[j % 13]);
			}
		if (!b) {
			s += '-';
		}
		if (i != 3) {
			s += ' ';
		}
	}

	return s;
}

VPbnEntryI Problem::findPbnEntry(const char* tag) {
	VPbnEntryI it;
	for (it = m_pbnEntry.begin(); it != m_pbnEntry.end(); it++) {
		if (strcasecmp(it->tag.c_str(), tag) == 0) {
			break;
		}
	}
	return it;
}

std::string Problem::getPbnTagsString() const {
	/* always store all tags, this can no violate tag order
	 * the second reason is that we can get full pbn from bts with pbninfo
	 */
	std::string s;
	VPbnEntryCI it;
	for (it = m_pbnEntry.begin(); it != m_pbnEntry.end(); it++) {
		s += format("[%s \"%s\"]\n", it->tag.c_str(), it->value.c_str());
		if (it->add.length() > 0) {
			s += it->add;
			s += "\n";
		}
	}

	return s;
}

const char* Problem::parseDfHand(const char*p, CARD_INDEX player) {
	int i, suit;
	const char*pc;
	for (; *p == ' '; p++)
		;
	for (suit = 0; *p != 0 && *p != 0xa && *p != 0xd; p++) {
		if (*p == '-') {
			continue;
		}
		if (*p == ' ') {
			if (suit == 3) {
				if (player == CARD_INDEX_WEST) {
					return p;
				}
				else {
					throwOnError(0, STRING_ERROR_INVALID_DEAL, "");
				}
			}
			suit++;
			continue;
		}
		assert(*p != 0);
		pc = strchr(RANK, *p);
		throwOnError(pc, STRING_ERROR_UNRECOGNIZED_CARD_RANK, "");
		i = 13 * suit + pc - RANK;
		throwOnError(m_states[0].m_cid[i] == CARD_INDEX_ABSENT,
				STRING_ERROR_FOUND_TWO_CARD_OCCURENCE_IN_DEAL, getCardString(i));
		m_states[0].m_cid[i] = player;
	}
	return p;

}

void Problem::parseDf(const std::string& content) {
	const char *p, *pc;
	std::string s;
	char comment[16];
	char lead[16];
	char contract[16];
	char onlead[16];

	sprintf(comment, "\n%s\n", DF_COMMENTARY_L);
	sprintf(lead, "\n%s ", DF_LEAD_L);
	sprintf(contract, "\n%s", DF_CONTRACT_L);
	sprintf(onlead, "\n%s", DF_ONLEAD_L);

	s.resize(content.size());
	std::transform(content.begin(), content.end(), s.begin(), ::tolower);

	p = strstr(s.c_str(), comment);
	if (p == NULL) {
		m_comment = "";
	}
	else {
		m_comment = content.c_str() + (p + strlen(comment) - s.c_str()); //from original case not lower
		m_comment = localeToUtf8(m_comment);
	}

	p = s.c_str();
	throwOnError(strlen(p) >= 20, STRING_UNKNOWN_ERROR, "");
	parseDfHand(p + 20, CARD_INDEX_NORTH);

	p = strstr(s.c_str(), contract);
	throwOnError(p!=NULL, STRING_ERROR_CONTRACT_NOT_FOUND, "");
	throwOnError(strlen(p) >= 12, STRING_UNKNOWN_ERROR, "");
	p += 11;
	m_contract = *p - '0';
	throwOnError(m_contract > 0 && m_contract < 8,
			STRING_ERROR_UNRECOGNIZED_CONTRACT, "");
	p++;
	assert(*p != 0);
	pc = strchr(SUITS_CHAR, *p);
	throwOnError(pc!=NULL, STRING_ERROR_UNRECOGNIZED_TRUMP, "");
	m_trump = pc - SUITS_CHAR;

	for (; *p != ' '; p++)
		;
	//parse west & east cards
	p = parseDfHand(p, CARD_INDEX_WEST);
	parseDfHand(p, CARD_INDEX_EAST);

	p = strstr(s.c_str(), onlead);
	throwOnError(p!=NULL, STRING_ERROR_ONLEAD_NOT_FOUND, "");
	throwOnError(strlen(p) > 21, STRING_UNKNOWN_ERROR, "");
	p += 9;
	setVeryFirstMove(playerFromChar(*p));
	parseDfHand(p + 12, CARD_INDEX_SOUTH);

	p = strstr(s.c_str(), lead);
	throwOnError(p!=NULL, STRING_ERROR_LEAD_NOT_FOUND, "");
	p += strlen(lead);
	/* not 'any' see Richard Pavlicek Deals.txt (deal 50)
	 * *p=='\n' empty string nothing to parse
	 */
	if (!startsWith(p, "any") && *p != '\n') {
		parsePlayString(p);
	}
}

std::string Problem::getContent(FILE_TYPE t, int nproblem, int totaproblems) {
	//should skip preferans problems if store to df or pbn
	if (isPreferans() && (t == FILE_TYPE_DF || t == FILE_TYPE_PBN)) {
		return "";
	}

	fillFirstCard();

	if (t == FILE_TYPE_DF) {
		return getDfContent(nproblem);
	}
	else if (t == FILE_TYPE_HTML) { //can be call from converter
		return getHTMLContent(nproblem, 0, 0, 0, true,totaproblems);
	}
	else if (t == FILE_TYPE_PBN) {
		return getPbnContent(nproblem, nproblem == 1);
	}
	else {
		return getBtsContent(nproblem, nproblem == 1);
	}
}

void Problem::parsePlayString(const char*p) {
	int i = 0;
	const char*q, *w;

	//empty string is possible see Richard Pavlicek Deals.txt Deal48
	if (*p != 0) {
		do {
			assert(*p != 0);
			q = strchr(SUITS_CHAR, *p);
			throwOnError(q, STRING_ERROR_UNRECOGNIZED_SUIT, format("[%c]", *p));
			p++;
			assert(*p != 0);
			w = strchr(RANK, *p);
			throwOnError(w, STRING_ERROR_UNRECOGNIZED_CARD_RANK, format("[%c]", *p));
			throwOnError(i < TOTAL_STATES - 1, STRING_ERROR_TOO_MANY_LEADING_CARDS,
					"");
			m_firstCard[i] = 13 * (q - SUITS_CHAR) + w - RANK;
			i++;

			for (p++; *p == ' ' || *p == '\t'; p++)
				;
			if (*p == '\n' || *p == 0) {
				break;
			}
		} while (1);
	}
	throwOnError(i < TOTAL_STATES, STRING_ERROR_TOO_MANY_LEADING_CARDS, "");
	m_firstCard[i] = CARD_INDEX_INVALID;

}

void Problem::parseDealString(const char* p, CARD_INDEX _player) {
	int i, j;
	const char*q;
	CARD_INDEX player = _player;
	std::string s = p;
	assert(player >= CARD_INDEX_NORTH && player <= CARD_INDEX_WEST);

	i = 0;
	while (*p != 0) {
		if (*p == ' ') { //change player
			i = 0;
			player = getNextPlayer(player);
		}
		else if (*p == '.') { //change suit
			if (i == 3) {
				throwOnError(0, STRING_ERROR_INVALID_DEAL, "");
			}
			i++;
		}
		else {
			assert(*p != 0);
			q = strchr(RANK, *p); //strchr(RANK,'\0')=13!
			throwOnError(q!=NULL, STRING_ERROR_UNRECOGNIZED_CARD_RANK,
					format("%c %d", *p, int(*p)));
			j = i * 13 + q - RANK;

			throwOnError(m_states[0].m_cid[j] == CARD_INDEX_ABSENT,
					STRING_ERROR_FOUND_TWO_CARD_OCCURENCE_IN_DEAL, getCardString(j));
			m_states[0].m_cid[j] = player;
		}
		p++;
	} //while

	throwOnError(getNextPlayer(player) == _player,
			STRING_ERROR_CARDS_SET_NOT_FOR_ALL_PLAYERS, "");
}

std::string Problem::getShortFileName() const {
	return getFileInfo(m_filepath, FILEINFO::NAME);
}

std::string Problem::postproceedHTML(const std::string& s, bool images) {
	std::string r = s;
	char b[] = "!*"; //first symbol '!' as search tag, second is variable char
	unsigned i;
	const char*p = SUIT_ARROW_CHAR;
	for (r = s, i = 0; *p != 0; i++, p++) {
		b[1] = *p;
		r = replaceAll(r, b,
				images ?
						format("<img src='%s/%c.png'>", HTML_IMAGE_DIRECTORY, *p) :
						CHANGE_HTML_SIMPLE[i]);
	}
	return r;
}

bool Problem::operator==(const Problem& p) const {
	int i, j;
	VPbnEntryCI it, it1;

	if (m_trump != p.m_trump || m_contract != p.m_contract
			|| m_comment != p.m_comment || m_maxState != p.m_maxState
			|| m_currentState != p.m_currentState || m_gameType != p.m_gameType
			|| m_pbnEntry.size() != p.m_pbnEntry.size()) {
		return false;
	}

	if (m_gameType == PREFERANS
			&& (m_player != p.m_player || m_misere != p.m_misere
					|| m_absent != p.m_absent)) {
		return false;
	}

	for (i = 0; i <= p.m_maxState; i++) {
		for (j = 0; j < 52; j++) {
			if (m_states[i].m_cid[j] != p.m_states[i].m_cid[j]) {
				return false;
			}
		}
	}

	//assume same order of tags
	for (it = pbnEntryBegin(), it1 = p.pbnEntryBegin(); it != pbnEntryEnd();
			it++, it1++) {
		if ((*it) != (*it1)) {
			return false;
		}
	}

	return true;
}

const Problem& Problem::operator=(const Problem& p) {
	int i;

	m_currentState = p.m_currentState;
	m_maxState = p.m_maxState;
	m_contract = p.m_contract;
	m_trump = p.m_trump;
	m_comment = p.m_comment;
	m_content = p.m_content;
	m_gameType = p.m_gameType;
	m_player = p.m_player;
	m_misere = p.m_misere;
	m_pbnEntry = p.m_pbnEntry;
	setAbsent(p.m_absent);

	for (i = 0; i <= m_maxState; i++) {
		m_states[i] = p.m_states[i];
	}

	return *this;
}

std::string Problem::getBtsContent(int nproblem, bool caption) {
	std::string s;
	int i, j;

	if (caption) {
		s = BTS_SIGNATURE + CURRENT_VERSION_STR + "\n";
	}
	s += BTS_BEGIN_MARKER + format("%d ", nproblem);
	s += getBtsDealString(dealFirst());
	s += "\n";

	s += BTS_CONTRACT;
	s += ' ';
	if (isPreferans() && m_misere) {
		s += BTS_MISERE;
	}
	else {
		if (m_contract == NO_CONTRACT_SET) {
			s += NO_CONTRACT_OR_NO_TRUMP_CHAR;
		}
		else {
			s += format("%d", m_contract);
		}

		if (m_trump == NO_TRUMP_SET) {
			s += NO_CONTRACT_OR_NO_TRUMP_CHAR;
		}
		else {
			s += getEnglishTrumpString();
		}

	}
	s += "\n";

	s += BTS_PLAY;
	for (i = 0; (j = m_firstCard[i]) != CARD_INDEX_INVALID; i++) {
		assert(j >= 0 && j < TOTAL_STATES);
		s += " ";
		s += getCardString(j);
	}
	if (m_firstCard[0] == CARD_INDEX_INVALID) {
		s += " ";
		s += toupper(PLAYER_CHAR[getVeryFirstMove() - 1]);
	}
	s += "\n";

	s += BTS_TURNS;
	s += " ";
	s += format("%d", m_currentState);
	s += "\n";

	if (isPreferans()) {
		s += BTS_PLAYER;
		s += ' ';
		s += toupper(PLAYER_CHAR[m_player - CARD_INDEX_NORTH]);
		s += "\n";

		s += BTS_ABSENT;
		s += ' ';
		s += toupper(PLAYER_CHAR[m_absent - CARD_INDEX_NORTH]);
		s += "\n";
	}

	s += BTS_COMMENT;
	s += ' ';
	s += utf8ToLocale(m_comment);
	s += "\n";

	if (!m_pbnEntry.empty()) {
		s += BTS_PBNINFO;
		s += "\n";
		adjustPbnInfo();
		s += getPbnTagsString();
		if (s[s.length() - 1] != '\n') { //sometimes last tag is AUCTION so it's already '\n'
			s += "\n";
		}
	}

	return s + "\n";
}

void Problem::parsePbn(const std::string& s) {
	const char *e, *q;
	std::string tag, value, add;
	char playValue = 0;
	CARD_INDEX player;
	int i, j, k, l;
	int a[4];
	Problem problem;
	problem.m_gameType = BRIDGE;
	State& state = problem.m_states[0];
	VPbnEntryCI it;

	//m_comment=s; user can change for example declarer, so should change many tags
	bool declarerError = false;

	fillPbnTags(s);

	for (it = pbnEntryBegin(); it != pbnEntryEnd(); it++) {
		tag = it->tag;
		value = it->value;
		add = it->add;
		std::transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		std::transform(add.begin(), add.end(), add.begin(), ::tolower);

		if (cmpnocase(tag, PBN_CONTRACT_TAG)) {
			if (cmpnocase(value, "pass")) {
				m_contract = NO_CONTRACT_SET;
				m_trump = NO_TRUMP_SET;
			}
			else {
				throwOnError(value.length() > 1, STRING_ERROR_INVALID_TAG_STRING, tag);
				m_contract = value[0] - '0';
				throwOnError(m_contract >= 1 && m_contract <= 7,
						STRING_ERROR_UNRECOGNIZED_CONTRACT, "");

				assert(value[1] != 0);
				e = strchr(SUITS_CHAR, value[1]);
				throwOnError(e!=NULL, STRING_ERROR_UNRECOGNIZED_TRUMP, "");
				m_trump = e - SUITS_CHAR;
			}
		}
		else if (cmpnocase(tag, PBN_DECLARER_TAG)) {
			if (value.length() == 0) {
				setVeryFirstMove(CARD_INDEX_NORTH);
			}
			else {
				player = playerFromChar(value[0]);
				if (player == CARD_INDEX_INVALID && value[0] == '^') { //available "^s"
					player = playerFromChar(value[1]);
					if (player == CARD_INDEX_INVALID) {
						declarerError = true;
					}
				}
				if (!declarerError) {
					assert(player >= CARD_INDEX_NORTH && player <= CARD_INDEX_WEST);
					setVeryFirstMove(getNextPlayer(player));
				}
			}
		}
		else if (cmpnocase(tag, PBN_PLAY_TAG)) {
			//I saw play tags like this "HK SJ H2 H9\n - CK - -"
			throwOnError(value.length() == 1, STRING_ERROR_INVALID_TAG_STRING, tag);
			playValue = value[0];
			k = 0;
			for (q = add.c_str(); *q != 0 && q[1] != 0;) {
				if (*q == '-') {
					k++; //may be can read other cards
				}
				else {
					assert(*q != 0);
					e = strchr(SUITS_CHAR, *q);
					if (!e) {
						throwOnError(*q == '\n' || *q == '^',
								STRING_ERROR_UNRECOGNIZED_SUIT, "");
						break;
					}
					j = 13 * (e - SUITS_CHAR);

					q++;

					e = strchr(RANK, *q);
					throwOnError(e!=NULL, STRING_ERROR_UNRECOGNIZED_CARD_RANK, "");
					assert(*e != 0); //after e!=NULL

					j += (e - RANK);
					//j automatically >=0 & <52
					throwOnError(k < 52, STRING_ERROR_TOO_MANY_LEADING_CARDS, "");
					m_firstCard[k++] = j;
				}

				//in pbn from Jack Saved Deal "HQ HK H4 H2 \nD5 DK D8 D2"
				for (q++; *q != 0 && strchr("\n \t", *q) != NULL; q++)
					;

			}
			/* reorder m_firstCard[] later, because trump should be set
			 * [Play "E"]
			 * HQ HK H4 H2
			 * D5 DK D8 D2 -> here D5 means East card, but m_firstCard[4] it should be a taker card
			 */
		}
		else if (cmpnocase(tag, PBN_DEAL_TAG)) {
			throwOnError(value.length() > 2, STRING_ERROR_INVALID_TAG_STRING, tag);

			player = playerFromChar(value[0]);
			throwOnError(player != CARD_INDEX_INVALID,
					STRING_ERROR_UNRECOGNIZED_PLAYER, "");
			//value in lower case
			parseDealString(value.c_str() + 2, player);
		}
	}

	//because call getNextMove() later first move should be set
	if (m_contract != NO_CONTRACT_SET && declarerError) {
		throwOnError(0, STRING_ERROR_DECLARER_TAG_NOT_FOUND, "");
	}

	//reorder m_firstCard[] see read "play" section
	for (k = i = 0; i < 52; i += 4) {
		//should read packs (with four cards) for strings like this "- CK - -"
		for (l = j = 0; j < 4; j++) {
			assert(i + j < 52);
			if (m_firstCard[i + j] != CARD_INDEX_INVALID) {
				l++;
				k++;
			}
		}
		if (l < 4) {
			break;
		}
	}

	//adjust k should be k%4=0
	if (k % 4 != 0) {
		k = 4 * (k / 4 + 1);
	}

	if (k > 4) {
		for (i = 0; i < 52; i++) {
			state.m_cid[i] = m_states[0].m_cid[i];
		}
		problem.m_trump = m_trump;

		assert(playValue != 0);
		e = strchr(PLAYER_CHAR, playValue);
		throwOnError(e!=NULL, STRING_ERROR_UNRECOGNIZED_PLAYER, "");

		state.m_firstmove = getVeryFirstMove();
		for (l = 0; l + 4 < k; l += 4) { //for l reorder l+4 - l+7
			for (i = 0; i < 4; i++) {
				assert(i + l < 52);
				j = m_firstCard[i + l];
				assert(j >= 0 && j < 52);
				state.m_cid[j] = getInner(m_states[0].m_cid[j]);
			}
			state.m_firstmove = problem.getNextMove(0);
			j = (state.m_firstmove - CARD_INDEX_NORTH) + 4 - (e - PLAYER_CHAR);

			for (i = 0; i < 4; i++) {
				assert(l + 4 + (i + j) % 4 < 52);
				a[i] = m_firstCard[l + 4 + (i + j) % 4];
			}
			for (i = 0; i < 4; i++) {
				assert(l + 4 + i < 52);
				m_firstCard[l + 4 + i] = a[i];
			}
			state.clearInner();
		}
	}
}

void Problem::parseBts(const std::string& content) {
	const char*f;
	const char*p, *p1;
	char b[32];
	char c;
	char*q;
	CARD_INDEX ci;
	std::string s;
	int i;

	s.resize(content.size());
	std::transform(content.begin(), content.end(), s.begin(), ::tolower);
	f = s.c_str();

	sprintf(b, "\n%s ", BTS_ABSENT);
	p = strstr(f, b);
	if (p == NULL) {
		ci = CARD_INDEX_SOUTH; //version below 5.1
	}
	else {
		p += strlen(b);
		ci = playerFromChar(*p);
		throwOnError(ci != CARD_INDEX_INVALID, STRING_ERROR_UNRECOGNIZED_ABSENT, "");
	}
	setAbsent(ci);

	sprintf(b, "\n%s ", BTS_PLAY);
	p = strstr(f, b);
	/* p=NULL means pass all contract because in file "\nplay\n" and we want
	 * to find "\nplay " but anyway nothing to parse
	 */
	if (p != NULL) {
		p += strlen(b);
		while (*p == ' ' || *p == '\t') {
			p++;
		}

		if (*p != 0 && p[1] == '\n') { //one char
			assert(*p != 0);
			p = strchr(PLAYER_CHAR, *p);
			throwOnError(p, STRING_ERROR_UNRECOGNIZED_PLAYER, "");
			setVeryFirstMove(PLAYER[p - PLAYER_CHAR]);
		}
		else if (*p != '\n') {
			parsePlayString(p);
		}
	}

	sprintf(b, "\n%s ", BTS_TURNS);
	p = strstr(f, b);
	throwOnError(p, STRING_ERROR_TURNS_NOT_FOUND, "");
	p += strlen(b);
	m_currentState = strtol(p, &q, 10);
	throwOnError(*q == '\n', STRING_ERROR_INVALID_TAG_STRING, "");

	sprintf(b, "\n%s ", BTS_CONTRACT);
	p = strstr(f, b);
	throwOnError(p!=NULL, STRING_ERROR_CONTRACT_NOT_FOUND, "");
	p += strlen(b);
	if (*p == NO_CONTRACT_OR_NO_TRUMP_CHAR) {
		m_contract = NO_CONTRACT_SET;
		p++;
	}
	else {
		m_contract = strtol(p, &q, 10); //invalid contract checked later when game type will be set
		assert(*q != 0);
		p = q;
	}
	if (*p == NO_CONTRACT_OR_NO_TRUMP_CHAR) {
		m_trump = NO_TRUMP_SET;
	}
	else {
		c = *p;
		p = strchr(SUITS_CHAR, c);
		if (p == NULL) {
			if (startsWith(q, BTS_MISERE) || startsWith(q, BTS_MISERE_OLD) ) {
				//set misere
				m_misere = true;
				m_trump = NT;
			}
			else {
				throwOnError(0, STRING_ERROR_UNRECOGNIZED_TRUMP, format("[%c]", c));
			}
		}
		else {
			m_trump = p - SUITS_CHAR;
		}
	}

	//for preferans
	sprintf(b, "\n%s ", BTS_PLAYER);
	p = strstr(f, b);
	if (p == NULL) { //can be empty
		m_gameType = BRIDGE;
	}
	else {
		m_gameType = PREFERANS;
		throwOnError(strlen(p) > strlen(b), STRING_UNKNOWN_ERROR, "")
		p += strlen(b);
		m_player = playerFromChar(*p);
		throwOnError(m_player != CARD_INDEX_INVALID,
				STRING_ERROR_UNRECOGNIZED_PLAYER, "");
	}

	//not assertAlways split function already checks it
	assert(startsWith(f, BTS_BEGIN_MARKER));
	//Note after m_gametype already set
	strtol(f + strlen(BTS_BEGIN_MARKER), &q, 10);
	throwOnError(*q == ' ', STRING_UNKNOWN_ERROR, "");
	q++;
	p = strchr(q, '\n');
	assert(p);
	parseDealString(std::string(q, p - q).c_str(), dealFirst());

	//now we can set very first move
	if (m_firstCard[0] == CARD_INDEX_INVALID) {
		if (m_trump != NO_TRUMP_SET || m_contract != NO_CONTRACT_SET) { //not pass contract
			throwOnError(getVeryFirstMove() != CARD_INDEX_INVALID,
					STRING_ERROR_PLAY_NOT_FOUND, "");
		}
		else {
			//first move not set in 'play' section 'pass contract'
			if (getVeryFirstMove() == CARD_INDEX_INVALID) {
				setVeryFirstMove(CARD_INDEX_NORTH); //should be set anyway
			}
		}
	}
	else {
		ci = m_states[0].m_cid[m_firstCard[0]];
		if (isInner(ci)) {
			ci = getOuter(ci);
		}
		setVeryFirstMove(ci);
	}

	sprintf(b, "\n%s ", BTS_COMMENT);
	p = strstr(f, b);
	throwOnError(p!=NULL, STRING_ERROR_COMMENT_NOT_FOUND, "");
	p += strlen(b);
	p1 = content.c_str() + (p - f); //from original case not lower

	sprintf(b, "\n%s\n", BTS_PBNINFO); //no space after %s!
	q = strstr(f, b);

	if (q == NULL) {
		m_comment = p1;
	}
	else {
		assert(q >= p);
		m_comment = std::string(p1, q - p);
		fillPbnTags(content.c_str() + (q - f) + strlen(b)); //from original case not lower
	}
	m_comment = localeToUtf8(m_comment);

	for (i = 0; i < 52; i++) {
		if (m_firstCard[i] == CARD_INDEX_INVALID) {
			break;
		}
	}
	throwOnError(m_currentState <= i,
			STRING_ERROR_NUMBER_OF_TURNS_EXCEEDS_NUMBER_OF_FIRST_MOVES, "");

	if (isBridge() || (isPreferans() && !m_misere)) {
		throwOnError(
				(m_contract >= minContract() && m_contract <= maxContract())
						|| m_contract == NO_CONTRACT_SET,
				STRING_ERROR_UNRECOGNIZED_CONTRACT, "");
	}

}

void Problem::parseBtsOldFormat(const std::string& content,
		GAME_TYPE gameType) {
	int i, j, k;
	Stream stream;
	CARD_INDEX ci;

	stream.init(content); //set content pointer
	m_gameType = gameType;
	m_absent = CARD_INDEX_SOUTH; //for old versions

	//no info about contract in file
	m_contract = NO_CONTRACT_SET; //

	for (i = 0; i < 52; ++i) {
		j = stream.readInteger();
		throwOnError(j >= -1 && j <= 8, STRING_ERROR_INVALID_CARD, "");
		m_states[0].m_cid[i] = static_cast<CARD_INDEX>(j);
	}

	//if problem have many states we'll load very first move from very first state
	//but we still need to load m_firstmove here for old files format
	j = stream.readInteger();
	const CARD_INDEX veryFirstMove = PLAYER[j];
	setVeryFirstMove(veryFirstMove);

	j = stream.readInteger();
	m_trump = j;

	if (isPreferans()) {
		//player
		j = stream.readInteger();
		m_player = m_preferansPlayer[j];

		//misere
		j = stream.readInteger();
		m_misere = j == 1;

		//in case of PREFERANS description string is placed before tricks of every player
		m_comment = stream.readString();
	}
	//if file in old format then number of tricks=0 (set in Problem constructor)
	if (stream.integersLeft() < 4) {
		goto end;
	}
	else {
		stream.readDummyIntegers(4);	//tricks
	}

	if (isBridge()) {	//in case of bridge game description will go after common tricks
		if (stream.end()) {
			goto end;
		}
		m_comment = stream.readString();
	}

	/*Note Not remove this comment
	 //read 53 states{
	 M(cid,52); 52 - integer cid's
	 M(n_tricks,5); 5 - integer tricks
	 M(LastTrick,4); 4 - integer Last trick
	 M0(nextmove); - integer next move
	 M0(firstmove); - integer first move
	 M0(prevmove); - integer previous move
	 M0(notUsed); - integer not used
	 M(estimate,52); 52 - integer estimate
	 M(oldTricks,2); 2 - integer old tricks
	 //total 52+5+4+4+52+2=119 integers for one state
	 }
	 +StoreActualPos+StoreMaxIndex
	 altogether 53*119+2=6309 integers or 25236 bytes
	 */

	if (stream.integersLeft() < 6309) {
		goto end;
	}

	//better use bytesLeft() then integersLeft() because integer has divisor 4, so for 0,1,2,3 ->integersLeft()=0 the same result

	/*
	 * assertAlways(stream.bytesLeft()==6309*4);
	 * http://www.doubledummy.net/Problem0004.html
	 */
	throwOnError(stream.bytesLeft() == 6309 * 4, STRING_UNKNOWN_ERROR, "");

	/* Note We should load all of first moves because user may be want to store problem to html file
	 * immediately after loading, so we need to have valid firstmoves
	 */
	stream.getLastTwoIntegers(m_currentState, m_maxState);
	throwOnError(m_maxState < TOTAL_STATES, STRING_UNKNOWN_ERROR, "");

	for (i = 0; i <= m_maxState; i++) {	// i<=maxindex !!!
		//total (52+13+52+2)=119 integers for each state
		for (j = 0; j < 52; j++) {
			m_states[i].m_cid[j] = CARD_INDEX(stream.readInteger());
		}
		stream.readDummyIntegers(10);

		j = stream.readInteger();
		throwOnError(j>=0 && j<SIZEI(PLAYER), STRING_ERROR_UNRECOGNIZED_PLAYER, "");
		m_states[i].m_firstmove = PLAYER[j];

		stream.readDummyIntegers(2);

		for (j = 0; j < 52; j++) {
			m_states[i].m_estimate[j] = stream.readInteger();
		}
		stream.readDummyIntegers(2);
	}

//	stream.readDummyIntegers((TOTAL_STATES-maxindex-1)*119 );
//	problem.m_currentState=stream.readInteger();
//	maxindex=stream.readInteger();//StoreMaxIndex;

	if (m_maxState <= 0) {		//restore veryFirstMove check on 7.pts (maxindex=0)
		setVeryFirstMove(veryFirstMove);
	}

	end: checkFirstCardsRepeat();

	//first state has inner cards
	i = m_states[0].countInnerCards();
	ci = getVeryFirstMove();
	for (j = 0; j < i; j++) {
		for (k = 0; k < 52; k++) {
			if (m_states[0].m_cid[k] == getInner(ci)) {
				break;
			}
		}
		assert(k < 52);
		m_firstCard[j] = k;
		ci = getNextPlayer(ci);
	}
	m_firstCard[j] = -1;

	for (k = 0; k < 52; k++) {
		ci = m_states[0].m_cid[k];
		if (isInner(ci)) {
			m_states[0].m_cid[k] = getOuter(ci);
		}
	}

}

void Problem::parse(FILE_TYPE t, const std::string& s, bool useOldBtsParser,
		GAME_TYPE gameType) {
	assert(
			t == FILE_TYPE_DF || t == FILE_TYPE_PBN || t == FILE_TYPE_BRIDGE
					|| t == FILE_TYPE_PREFERANS);

	int i, j;
	m_gameType = BRIDGE;
	/* at start make all cards are absent
	 * if user load problem where every player has less than 13 card
	 * and then select 'edit problem' all no set cards should be absent
	 */
	for (i = 0; i < 52; ++i) {
		m_states[0].m_cid[i] = CARD_INDEX_ABSENT;
	}

	m_contract = INVALID_CONTRACT;
	m_trump = INVALID_TRUMP;
	setVeryFirstMove(CARD_INDEX_INVALID);
	m_content = s;

	if (t == FILE_TYPE_DF) {
		parseDf(s);
	}
	else if (t == FILE_TYPE_PBN) {
		parsePbn(s);
	}
	else {
		if (useOldBtsParser) {
			parseBtsOldFormat(s, gameType);
		}
		else {
			parseBts(s);
		}
	}

	assert(getVeryFirstMove() != CARD_INDEX_INVALID);

#ifndef NDEBUG
	if (isBridge() || (isPreferans() && !m_misere)) {
		assert(
				(m_contract >= minContract() && m_contract <= maxContract())
						|| m_contract == NO_CONTRACT_SET);
	}
#endif
	assert((m_trump >= 0 && m_trump <= NT) || m_trump == NO_TRUMP_SET);

	if (!useOldBtsParser) {
		checkFirstCardsRepeat();
	}

	if (isPreferans()) {
		m_states[0].setInvalidCardsForPreferans();
	}

	//4nov2021 allow all deal states, because user can store error deals getDealState(false) != DEAL_STATE_ERROR

	if (m_maxState == -1) {
		for (i = 0; i < TOTAL_STATES; ++i) {
			m_states[i].clearEstimates();
		}
		for (i = 0; i < 52 && (j = m_firstCard[i]) != -1; i++) {
			throwOnError(isValidTurn(j, i), STRING_ERROR_INVALID_MOVE_FOUND,
					getCardString(j));
			makeMove(j, i);
		}
		m_maxState = i;

		//m_currentState is set in parseBts(), parseBtsOld() define it for other formats
		if (m_currentState == -1) {
			m_currentState = m_maxState < 1 ? 0 : 1;
		}
	}

}

void Problem::fillPbnTags(const std::string& s) {
	SString set;
	const char*p, *e;
	std::string tag, value, add;
	p = s.c_str();
	throwOnError(*p == '[', STRING_UNKNOWN_ERROR, "");
	do {
		p++;
		assert(*p != 0);
		e = strchr(p, ' ');
		throwOnError(e, STRING_ERROR_INVALID_TAG_STRING, "");
		tag = std::string(p, e);
		p = strchr(e, '\"');
		throwOnError(p, STRING_ERROR_INVALID_TAG_STRING, tag);
		p++;
		e = strchr(p, '\"');
		value = std::string(p, e);
		e++;
		throwOnError(*e == ']', STRING_ERROR_INVALID_TAG_STRING, tag);
		e++;

		if (*e == 0) {
			add = "";
			p = NULL;
		}
		else {
			throwOnError(*e == '\n', STRING_ERROR_INVALID_TAG_STRING, tag);
			p = strchr(e, '[');

			if (p == NULL) {
				add = std::string(e + 1);
			}
			else {
				if (p - 1 > e) {
					e++;
				}
				add = std::string(e, p - 1);
			}
		}
		m_pbnEntry.push_back(PbnEntry(tag, value, add));

		throwOnError(set.find(tag) == set.end(), STRING_ERROR_DUPLICATE_TAG_FOUND,
				tag);
		set.insert(tag);
	} while (p);

}

void Problem::checkFirstCardsRepeat() {
	SInt set;
	int i, j;
	for (i = 0; i < 52 && (j = m_firstCard[i]) != CARD_INDEX_INVALID; i++) {
		throwOnError(set.find(j) == set.end(),
				STRING_ERROR_CARD_IN_LIST_OF_MOVES_IS_FOUND_MORE_THAN_ONE_TIME,
				getCardString(j));
		set.insert(j);
	}

}

void Problem::saveState() {
	m_maxState = ++m_currentState;
	assert(m_currentState >= 1 && m_currentState < TOTAL_STATES);
	m_states[m_currentState] = m_states[m_currentState - 1];
}

std::string Problem::getEnglishTrumpString() const {
	return ::getEnglishTrumpString(m_trump);
}

void Problem::newDeal() {
	m_maxState = m_currentState = 0;
	m_states[0].newDeal(*this);
	if (isPreferans()) {
		m_player = getBridgePartner(m_absent);
		m_misere = false;
	}
	m_comment = "";
	m_contract = minContract();
}

void Problem::randomDeal() {
	int i;

	for (i = 0; i < 52; i++) {
		m_states[i].clearEstimates();
	}

	m_trump = rand() % 5;

	if (isPreferans()) {
		m_misere = rand() % 6 == 0;
		m_player = m_preferansPlayer[rand() % 3];
	}
	i = minContract();
	m_contract = i + rand() % (maxContract() - i + 1);

	m_currentState = 0;
	i = getState().randomDeal(*this);

	saveState();

	m_states[1].m_cid[i] = getInner(m_states[0].m_firstmove);

	m_comment = "";
}

CARD_INDEX Problem::getLastTrick(int* moves) const {
	int i;
	int*p;
	CARD_INDEX result, c;
	const int max = maxTableCards();
	State const& state = getState();
	i = state.countInnerCards();
	if (i == max) {
		i = m_currentState;
		result = state.m_firstmove;
	}
	else {
		i = m_currentState - i;
		//bridge state index should be at least 3=max-1
		//preferans state index should be at least 2=max-1
		result = i < max - 1 ? CARD_INDEX_INVALID : m_states[i].m_firstmove;
	}

	//fill moves
	if (result != CARD_INDEX_INVALID && moves != NULL) {
		State const& state = m_states[i];
		c = result;
		p = moves;
		do {
			*p++ = state.findInner(c);
			assert(p[-1] < 52);
		} while ((c = getNextPlayer(c)) != result);
	}

	return result;
}

int Problem::fillFirstCard() {
	int i;
	for (i = 0; i < m_maxState; i++) {
		m_firstCard[i] = m_states[i].getDifference(m_states[i + 1]);
	}
	assert(i < TOTAL_STATES);
	m_firstCard[i] = CARD_INDEX_INVALID;
	return i;
}

bool Problem::checkAuctionTag() {
	VPbnEntryCI it;
	it = findPbnEntry(PBN_AUCTION_TAG);
	assert(it != pbnEntryEnd());
	std::string s;
	AuctionTagParseResult r = parseAuctionTag(it->value, it->add);
	if (r.id != STRING_OK) {
		s = getString(r.id);
		if (r.s.length() != 0) {
			s += " ";
			s += r.s;
		}
		message(MESSAGE_ICON_ERROR, s);
		return false;
	}

	it = findPbnEntry(PBN_CONTRACT_TAG);
	assert(it != pbnEntryEnd());
	if (r.s != it->value) {
		message(MESSAGE_ICON_ERROR,
				STRING_CONTRACT_FROM_AUCTION_TAG_SHOULD_MATCH_WITH_CONTRACT_TAG);
		return false;
	}

	if (!cmpnocase(it->value, "pass")) {
		it = findPbnEntry(PBN_DECLARER_TAG);
		assert(it != pbnEntryEnd());
		assert(it->value.length() == 1);
		if (r.declarer != it->value[0]) {
			message(MESSAGE_ICON_ERROR,
					STRING_DECLARER_FROM_AUCTION_TAG_SHOULD_MATCH_WITH_DECLARER_TAG);
			return false;
		}
	}

	return true;
}

DEAL_STATE Problem::getDealState(bool checkTrump) const {
	int i, j, k;
	int ncards[] = { 0, 0, 0, 0 };
	bool empty = true;
	CARD_INDEX player;

	if (checkTrump && m_trump == NO_TRUMP_SET) {//no trump is set, for example for pass contract
		return DEAL_STATE_ERROR;
	}

	for (i = 0; i < 52; ++i) {
		if (getState().m_cid[i] == CARD_INDEX_ABSENT
				|| getState().m_cid[i] == CARD_INDEX_INVALID) {	//invalid 2-6 for preferans
			continue;
		}
		else if (isOuter(getState().m_cid[i])) {
			player = getState().m_cid[i];
			empty = false;
		}
		else {
			player = getOuter(getState().m_cid[i]);
		}
		ncards[indexOfPlayer(player)]++;
	}

	auto n = isBridge() ? CARD_INDEX_NORTH : getNextBridgePlayer(m_absent);
	j = ncards[indexOfPlayer(n)];
	k = 2 + isBridge();
	for (i = 0; i < k; i++) {
		n = getNextPlayer(n);
		if (ncards[indexOfPlayer(n)] != j) {
			break;
		}
	}
	if (i == k) {
		return empty ? DEAL_STATE_EMPTY : DEAL_STATE_VALID;
	}
	else {
		return DEAL_STATE_ERROR;
	}
}

bool Problem::supportFileFormat(FILE_TYPE t) const {
	assert(t == FILE_TYPE_DF || t == FILE_TYPE_PBN);
	if (t == FILE_TYPE_DF) {
		return !noTrumpOrContract();
	}
	else {
		if (noTrumpAndContract()) {
			return true;
		}
		else {
			return !noTrumpOrContract();
		}
	}
}

std::string Problem::getValidTrumpStringDfPbn() const {
	if (m_trump == NO_TRUMP_SET) {
		return ::getEnglishTrumpString(0);
	}
	else {
		return getEnglishTrumpString();
	}
}

std::string Problem::getValidContractStringDfPbn() const {
	return format("%d",
			m_contract == NO_CONTRACT_SET ? MIN_CONTRACT[m_gameType] : m_contract);
}

#ifndef FINAL_RELEASE
void Problem::printcids(int problemIndex) {
	assert(problemIndex >= 0 && problemIndex < TOTAL_STATES);
	m_states[problemIndex].printcids();
}
#endif

std::string Problem::getBtsDealString(CARD_INDEX dealer) const {
	int i, j;
	std::string s;
	CARD_INDEX ci = dealer;

	const CARD_INDEX* cid = m_states[0].m_cid;

	//i-who j-suit k-index
	do {
		for (i = 0; i < 4; ++i) {
			for (j = 0; j < 13; ++j) {
				if (cid[i * 13 + j] == ci || cid[i * 13 + j] == ci + 4) {	//sometimes can be ci+4 on very first state
					s += toupper(RANK[j]);
				}
			}
			if (i != 3) {
				s += '.';
			}
		}

		ci = getNextPlayer(ci);
		if (ci == dealer) {		//3 times for preferans, 4 for bridge
			break;
		}
		s += ' ';

	} while (1);

	return s;
}

CARD_INDEX Problem::getNextMove(int stateIndex) const {
	assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
	const State & s = m_states[stateIndex];

	int inner = s.countInnerCards();

	if (inner == 0) {
		return s.m_firstmove;
	}
	else if (inner == maxTableCards()) {
		return getTaker(inner, stateIndex);
	}
	else {
		return getPlayer(s.m_firstmove, true, inner);
	}
}

void Problem::makeMove(int index, int stateIndex) {
	int i;
	assert(stateIndex + 1 < TOTAL_STATES);
	assert(stateIndex >= 0);
	State& prev = m_states[stateIndex];
	State& state = m_states[stateIndex + 1];
	for (i = 0; i < 52; i++) {
		state.m_cid[i] = prev.m_cid[i];
	}
	state.copyTricks(prev);

	if (isTableFull(stateIndex)) {
		state.clearInner();
	}

	if (isTableFull(stateIndex)) {
		state.m_firstmove = getNextMove(stateIndex);
	}
	else {
		state.m_firstmove = prev.m_firstmove;
	}

	state.m_cid[index] = getInner(prev.m_cid[index]);

	if (isTableFull(stateIndex + 1)) {
		state.incrementTricks(getNextMove(stateIndex));
	}

}

bool Problem::isValidTurn(int index, int stateIndex, bool drawarea) {
	assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
	const State & s = m_states[stateIndex];

	CARD_INDEX player = s.m_cid[index];

	if (!drawarea) {
		if (getNextMove(stateIndex) != player) {
			return false;
		}
	}

	if (isTableFullOrEmpty(stateIndex)) {		// first turn every card is valid
		return true;
	}
	else if (hasLeadingSuit(player, stateIndex)) {//player has leading suit, valid is suit of card is the same with leading suit
		return index / 13 == getLeadingSuit(stateIndex);
	}
	else {		//player has not leading suit
		if (m_gameType == BRIDGE) {
			return true;		//for bridge is always valid
		}
		else {		//for preferans
			if (m_trump == NT || !s.hasSuit(player, m_trump)) {
				return true;
			}
			else {
				return index / 13 == m_trump;		//trump suit
			}
		}
	}

}

int Problem::getLeadingSuit(int stateIndex) const {
	assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
	const State & s = m_states[stateIndex];

	if (isTableFullOrEmpty(stateIndex)) {
		return NT;
	}
	int i;
	for (i = 0; i < 52; ++i) {
		if (s.m_cid[i] == getInner(s.m_firstmove)) {
			break;
		}
	}
	return i / 13;
}

CARD_INDEX Problem::getTaker(const int inner, int stateIndex) const {
	int i, j;
	assert(stateIndex >= 0 && stateIndex < TOTAL_STATES);
	const State & s = m_states[stateIndex];

	CARD_INDEX player[4];
	player[0] = s.m_firstmove;

	for (i = 1; i < 4; i++) {
		player[i] = getNextPlayer(player[i - 1]);
	}

	int index[4];
	for (i = 0; i < 52; ++i) {
		for (j = 0; j < inner; j++) {
			if (s.m_cid[i] == getInner(player[j])) {
				index[j] = i;
				break;
			}
		}
	}

	j = 0;		//taker
	for (i = 1; i < inner; i++) {
		if (!compare(index[j], index[i])) {
			j = i;
		}
	}
	return player[j];

}

bool Problem::compare(int index1, int index2) const {
	int suit1 = index1 / 13;
	int card1 = 12 - index1 % 13;
	int suit2 = index2 / 13;
	int card2 = 12 - index2 % 13;
	return suit1 == suit2 ? card1 > card2 : suit2 != m_trump;
}

void Problem::rotate(bool clockwise, bool likeBridge) {
	int i;

	//before rotate states set m_absent
	if (isPreferans()) {
		rotate(m_player, clockwise, likeBridge);
		if (likeBridge) {
			rotate(m_absent, clockwise, likeBridge);
			setAbsent(m_absent);		//also fill preferansPlayer
		}
	}

	for (i = 0; i <= m_maxState; i++) {
		m_states[i].rotate(*this, clockwise, likeBridge);
	}

}

void Problem::rotate(CARD_INDEX& index, bool clockwise, bool likeBridge) const {
	if (index != CARD_INDEX_INVALID && index != CARD_INDEX_ABSENT) {
		if (likeBridge) {
			index = getBridgePlayer(index, clockwise);
		}
		else {
			index = getPlayer(index, clockwise);
		}
	}
}

void Problem::fillInner4(int* index, CARD_INDEX from) const {
	State const& s = getState();
	int*p = index;
	CARD_INDEX c = from;
	do {
		*p++ = s.findInner(c);
	} while ((c = getNextBridgePlayer(c)) != from);
}

void Problem::fillAdjustPbnInfo(int nproblem) {
	int i;
	std::string value, add;
	char buffer[80];
	VPbnEntryI it;

	//no dealer info assume 'N'
	const CARD_INDEX dealer = CARD_INDEX_NORTH;

	const char *tag[] = {
			PBN_EVENT_TAG,
			PBN_SITE_TAG,
			PBN_DATE_TAG,
			PBN_BOARD_TAG,
			PBN_WEST_TAG,
			PBN_NORTH_TAG,
			PBN_EAST_TAG,
			PBN_SOUTH_TAG,
			PBN_DEALER_TAG,
			PBN_VULNERABLE_TAG,
			PBN_DEAL_TAG,
			PBN_DECLARER_TAG,
			PBN_CONTRACT_TAG,
			PBN_RESULT_TAG,
			PBN_HOMETEAM_TAG,
			PBN_ROOM_TAG,
			PBN_ROUND_TAG,
			PBN_SCORE_TAG,
			PBN_SECTION_TAG,
			PBN_TABLE_TAG,
			PBN_VISITTEAM_TAG,
			PBN_AUCTION_TAG,
			PBN_PLAY_TAG };

	if (m_pbnEntry.empty()) {
		for (i = 0; i < SIZEI(tag); i++) {
			if (cmpnocase(tag[i], PBN_DATE_TAG)) {
				time_t rawtime;
				time(&rawtime);

				//date 1997.06.27
				strftime(buffer, 80, "%Y.%m.%d", localtime(&rawtime));
				value = buffer;
			}
			else if (cmpnocase(tag[i], PBN_DEALER_TAG)) {
				value = DEALER[dealer - CARD_INDEX_NORTH];
			}
			else if (cmpnocase(tag[i], PBN_DEAL_TAG)) {
				value = getPbnDealString(dealer);
			}
			else if (cmpnocase(tag[i], PBN_TABLE_TAG)) {
				value = format("%d", nproblem);
			}
			else if (cmpnocase(tag[i], PBN_VULNERABLE_TAG)) {
				value = VULNERABLE[0];
			}
			else if (cmpnocase(tag[i], PBN_ROOM_TAG)) {
				value = ROOM[0];
			}
			else if (cmpnocase(tag[i], PBN_RESULT_TAG)) {
				value = "0";
			}
			else if (cmpnocase(tag[i], PBN_SCORE_TAG)) {
				value = "NS 0";
			}
			else if (cmpnocase(tag[i], PBN_AUCTION_TAG)) {
				value = DEALER[dealer - CARD_INDEX_NORTH];
			}
			else {		//some tags will be set later usually empty tag has value="#"
				value = "#";
			}

			add = "";
			m_pbnEntry.push_back(PbnEntry(tag[i], value, add));
		}
	}

	adjustPbnInfo();
}

void Problem::adjustPbnInfo() {
	int i, j, k, l, result;
	std::string value, add;
	VPbnEntryI it;
	bool br;
	char*p;

	//correct some tags
	it = findPbnEntry(PBN_CONTRACT_TAG);
	assert(it != m_pbnEntry.end());
	j = countEndX(it->value);
	assert(j <= 2);
	it->value =
			noTrumpAndContract() ?
					"Pass" : getValidContractTrumpStringDfPbn() + DOUBLE_REDOUBLE[j];

	it = findPbnEntry(PBN_DECLARER_TAG);
	assert(it != m_pbnEntry.end());
	it->value = noTrumpAndContract() ? "" : DEALER[getDeclarerInt()];

	it = findPbnEntry(PBN_PLAY_TAG);
	if (it != m_pbnEntry.end()) {
		it->value = DEALER[getVeryFirstMove() - CARD_INDEX_NORTH];
		add = "";
		CARD_INDEX fci[4] = { getVeryFirstMove() };
		assert(
				getVeryFirstMove() >= CARD_INDEX_NORTH
						&& getVeryFirstMove() <= CARD_INDEX_WEST);
		for (j = 1; j < 4; j++) {
			fci[j] = getNextPlayer(fci[j - 1]);
		}

		i = fillFirstCard();
		//fill up to %4=0, if m_firstCard[0..4] 5 cards fill m_firstCard[5,6,7]=-1
		//if fill first 4 cards m_firstCard[0..3] fill m_firstCard[4,5,6,7]=-1
		for (j = i + 1; j % 4 != 0; j++) {
			m_firstCard[j] = CARD_INDEX_INVALID;
		}

		for (i = 0; i < 52; i += 4) {
			for (j = 0; j < 4; j++) {
				assert(i + j < 52);
				if (m_firstCard[i + j] != CARD_INDEX_INVALID) {
					break;
				}
			}
			if (j == 4) {
				break;
			}

			br = false;
			for (j = 0; j < 4; j++) {
				for (k = 0; k < 4; k++) {
					l = m_firstCard[i + k];
					if (l != -1 && m_states[0].m_cid[l] == fci[j]) {
						break;
					}
				}
				if (k == 4) {
					br = true;
					k = -1;
				}
				else {
					k = m_firstCard[i + k];
				}
				add += k == -1 ? "-" : getCardString(k);
				add += j == 3 ? "\n" : " ";
			}
			if (br) {
				break;
			}
		}

		/* http://www.tistis.nl/pbn/
		 * The play section ends after 13 tricks, or with "*" .  The "*" character
		 * indicates that no further cards will or can be given.  In this case, the
		 * result of the play can be given in the tags 'Result'.
		 */
		if (m_firstCard[51] == CARD_INDEX_INVALID) {
			add += "*";
		}
		it->add = add;
	}

	//correct DEAL tag
	it = findPbnEntry(PBN_DEALER_TAG);
	assert(it != m_pbnEntry.end());
	assert(it->value.length() == 1);
	const CARD_INDEX dealer = playerFromChar(tolower(it->value[0]));//DEALER already used, dealer used later
	assert(dealer != CARD_INDEX_INVALID);
	it = findPbnEntry(PBN_DEAL_TAG);
	assert(it != m_pbnEntry.end());
	it->value = getPbnDealString(dealer);

	//TODO correct RESULT tag
	it = findPbnEntry(PBN_RESULT_TAG);
	assert(it != m_pbnEntry.end());
	l = strtol(it->value.c_str(), &p, 10);
	result = l;		//used for score tag
	assert(*p == 0);
	assert(l >= 0 && l <= 13);

	//BEGIN correct SCORE tag (after all tags)
	it = findPbnEntry(PBN_CONTRACT_TAG);
	assert(it != m_pbnEntry.end());
	i = countEndX(it->value);
	const int doubleRedouble = i;		//user later

	if (noTrumpOrContract()) {
		j = 0;
	}
	else {
		it = findPbnEntry(PBN_VULNERABLE_TAG);
		assert(it != m_pbnEntry.end());
		k = INDEX_OF_NO_CASE(it->value,VULNERABLE);

		j = countBridgeScore(m_contract, m_trump, result, i, getDeclarerInt(), k);
	}

	it = findPbnEntry(PBN_SCORE_TAG);		//sometimes not found
	if (it != m_pbnEntry.end()) {
		i = INDEX_OF_NO_CASE(it->value.substr(0, 2),SCORE );
		assert(i>=0 && i<SIZEI(SCORE));
		if (getDeclarerInt() % 2 != i) {
			j = -j;
		}
		it->value = format("%s %d", SCORE[i], j);
	}
	//END correct SCORE tag (after all tags)

	//correct AUCTION tag
	it = findPbnEntry(PBN_AUCTION_TAG);
	if (it != m_pbnEntry.end()) {
		if (it->add.length() == 0) {		//only if empty
			i = dealer - CARD_INDEX_NORTH;
			it->value = DEALER[i];
			add = "";
			for (k = 0; i != getDeclarerInt(); i = (i + 1) % 4, k++) {
				add += "Pass ";
			}
			if (doubleRedouble == 1) {
				add += "X ";
			}
			else if (doubleRedouble == 2) {
				add += "XX ";
			}

			add += getValidContractTrumpStringDfPbn();
			k++;		//max k=4

			for (i = 0; i < 3; i++, k++) {
				add += k == 4 ? "\n" : " ";
				add += "Pass";
			}
			it->add = add;
		}
	}

}

bool Problem::isSolveAllDealsEnable() const {
	if(think()){
		return false;
	}

	CARD_INDEX c[2];
	if(isBridge()){
		c[0]=isBridgeSolveAllDealsAbsentNS() ? CARD_INDEX_NORTH:CARD_INDEX_EAST;
		c[1]=getBridgePartner(c[0]);
	}
	else{
		c[0]=getNextPlayer(m_player);
		c[1]=getNextPlayer(c[0]);
	}
	//valid deal and players with unknown cards have at least one card
	auto&s=getState();
	return (s.countCards(c[0])>0 || s.countCards(c[1])>0) && getDealState()==DEAL_STATE_VALID;
}

void Problem::setAbsent(CARD_INDEX absent) {
	auto p = absent;
	m_absent = p;
	for (int i = 0; i < 3; i++) {
		/* cann't call getNextPlayer(p,PREFERANS) because this function uses preferansPlayer array
		 * so use getNextPlayer(p,BRIDGE)
		 */
		p = getNextBridgePlayer(p);
		m_preferansPlayer[i] = p;
	}
}

CARD_INDEX Problem::getPlayer(CARD_INDEX player, bool next, int count) const {
	if (m_gameType == BRIDGE) {
		return getBridgePlayer(player, next, count);
	}
	else {
		return getPlayerForArray(player, m_preferansPlayer, 3, next, count);
	}
}

CARD_INDEX Problem::getBasePlayer() const {
	return
			isPreferans() && m_absent == CARD_INDEX_WEST ?
					CARD_INDEX_EAST : CARD_INDEX_WEST;
}

void Problem::getClearCid(CARD_INDEX*cid) const {
	State const& state = getState();
	for (int i = 0; i < 52; i++) {
		cid[i] =
				isTableFull() && isInner(state.m_cid[i]) ?
						CARD_INDEX_ABSENT : state.m_cid[i];
	}
}

CARD_INDEX Problem::getDeclarer() const {
	if(m_gameType == BRIDGE){
		return getPreviousPlayer(getVeryFirstMove());
	}
	else{
		return m_player;
	}
}

#ifndef FINAL_RELEASE
std::string Problem::getForBridgeTestDealClass(int n)const {
	int i,j,k;
	std::string s="{\"";
	State const& state = getState();
	auto c=state.m_cid;
	for(i=0;i<4;i++){
		for(j=0;j<8;j++){
			k=c[i*13+j];
			if(k==CARD_INDEX_ABSENT || k==getInner(m_player) || k==m_player ){
				if(k==CARD_INDEX_ABSENT){
					s+='*';
				}
				else if(k==getInner(m_player)){
					s+='#';
				}
				s+=toupper(RANK[j]);
			}
		}
		s+=i==3?'"':'.';
	}

	s+=", "+std::to_string(m_misere?NT+1:m_trump);

	CARD_INDEX ci[]={CARD_INDEX_NORTH,CARD_INDEX_EAST,CARD_INDEX_WEST};
	i=INDEX_OF(state.m_firstmove,ci);
	s+=", "+std::to_string(i);

	s+=", \""+getFileInfo(m_filepath, FILEINFO::SHORT_NAME)+std::to_string(n)+"\"";

	s+="},\n";
	return s;
}
#endif
