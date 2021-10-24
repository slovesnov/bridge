/*
 * PbnEditorDialog.cpp
 *
 *       Created on: 22.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "PbnEditorDialog.h"
#include "../Frame.h"

/*
 * SCORE tag not always = DECLARER
 * AUCTION tag not always present even if not Pass game
 * PLAY tag not always present even if not Pass game
 * DEAL starts always from DEALER tag, if AUCTION tag present then it starts from DEALER
 * PLAY starts from next after dealer
 * min tags found=21, max tags found=23 (for problem from Poland tags=19 so table have (19-3)/2=8 rows only -3 because DEAL,AUCTION,PLAY)
 */

const char* CM[] = {
		PBN_DEALER_TAG,
		PBN_RESULT_TAG,
		PBN_ROOM_TAG,
		PBN_VULNERABLE_TAG };
const char* DAP[] = { PBN_DEAL_TAG, PBN_AUCTION_TAG, PBN_PLAY_TAG };

static PbnEditorDialog*dialog;

static void combo_changed(GtkWidget *widget, int i) {
	dialog->comboChanged(i);
}

static gboolean mouse_press_event(GtkWidget *widget, GdkEventButton *event,
		int i) {
	if (event->button == 1) {
		dialog->clickButton(i); //click(int) used for ButtonDialog
	}
	return TRUE;
}

PbnEditorDialog::PbnEditorDialog() :
		ButtonsDialog(MENU_PBN_EDITOR), ProblemVectorModified(getProblemSelector()) {
	int i;
	GtkWidget *w, *w1;

	dialog = this;
	prepareBridgeOnly();

	m_grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(m_grid), 3);
	gtk_grid_set_row_spacing(GTK_GRID(m_grid), 2);
	gtk_box_pack_start(GTK_BOX(getContentArea()), m_grid, TRUE, TRUE, 2);

	m_labelProblem = gtk_label_new("");
	w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	for (i = 0; i < 4; i++) {
		m_button[i] = gtk_image_new();
		gtk_widget_set_hexpand(m_button[i], TRUE);
		w1 = gtk_event_box_new();
		gtk_container_add(GTK_CONTAINER(w1), m_button[i]);
		gtk_widget_add_events(w1, GDK_BUTTON_PRESS_MASK);
		g_signal_connect(w1, "button_press_event", G_CALLBACK(mouse_press_event),
				GP(i));
		gtk_container_add(GTK_CONTAINER(w), w1);
		if (i == 1) {
			gtk_container_add(GTK_CONTAINER(w), m_labelProblem);
		}
	}

	redrawProblem();
	show(w, true);
}

PbnEditorDialog::~PbnEditorDialog() {
}

void PbnEditorDialog::updateScore() {
	int i, j;

	if (getProblem().noTrumpOrContract()) {
		j = 0;
	}
	else {
		i = getProblem().getDeclarerInt(); //north east south west
		j = countBridgeScore(getProblem().m_contract, getProblem().m_trump,
				getComboPosition(m_combo[PBN_EDITOR_COMBO_RESULT]),
				getComboPosition(m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE]), i,
				getComboPosition(m_combo[PBN_EDITOR_COMBO_VULNERABLE]));

		if (i % 2 != getComboPosition(m_combo[PBN_EDITOR_COMBO_SCORE])) {
			j = -j;
		}
	}
	gtk_label_set_text(GTK_LABEL(m_label[PBN_EDITOR_LABEL_SCORE]),
			format("%d", j).c_str());
}

void PbnEditorDialog::redrawProblem() {
	int i, j, k, l;
	char*p;
	VPbnEntryCI it;
	GList *children, *iter;
	GtkWidget *w, *w1;
	std::string s;
	GtkTextBuffer *buffer;
	VPbnEntryCI pbnDapEntry[SIZE(DAP)];
	const char* S_MEASURE[] = { "Pass Pass Pass Pass ", "DH DH DH DH " };
	const int ENTRY_SIZE = 230;
	const int COLUMNS = 2; //columns with simple tags

	assert(getProblem().isBridge());

	/* two combos m_combo[PBN_EDITOR_COMBO_SCORE] & m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE] not in m_grid and
	 * if user change one of them then got next message
	 * (bridge.exe:4904): Gdk-WARNING **: gdkwindow-win32.c:1833: SetWindowLongPtr failed: Invalid window handle.
	 * the solution is remove m_combo[..] from it's parent, all other combos are directly in m_grid, so they will
	 * be removed automatically
	 */
	children = gtk_container_get_children(GTK_CONTAINER(m_grid));
	if (g_list_length(children) > 0) {
		w = m_combo[PBN_EDITOR_COMBO_SCORE];
		w1 = gtk_widget_get_parent(w);
		if (w1 != NULL) { //remove only if have score tag problem send jvszwuwo@wp.pl no score tag
			gtk_container_remove(GTK_CONTAINER(w1), w);
		}

		w = m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE];
		w1 = gtk_widget_get_parent(w); //no parent in pass contract
		if (w1) {
			gtk_container_remove(GTK_CONTAINER(w1), w);
		}

		//clear m_grid
		for (iter = children; iter != NULL; iter = g_list_next(iter)) {
			gtk_container_remove(GTK_CONTAINER(m_grid), GTK_WIDGET(iter->data));
		}
	}
	g_list_free(children);

	gtk_label_set_text(GTK_LABEL(m_labelProblem),
			format("# %d / %d", m_current + 1, size()).c_str());

	for (i = 0; i < 4; i++) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(m_button[i]),
				getToolbarPixbuf(TOOLBAR_BUTTON_ARRAY[i + 1], false,
						boolToButtonState(isMovePossible(i))));
	}

	getProblem().fillAdjustPbnInfo(getProblemSelector().m_current + 1);

	m_entry.clear();

#define CREATE_COMBO(id,a) m_combo[id]=createTextCombobox(a,SIZE(a));
	CREATE_COMBO(PBN_EDITOR_COMBO_DEALER, DEALER);
	CREATE_COMBO(PBN_EDITOR_COMBO_ROOM, ROOM)
	CREATE_COMBO(PBN_EDITOR_COMBO_VULNERABLE, VULNERABLE)
	CREATE_COMBO(PBN_EDITOR_COMBO_DOUBLE_REDOUBLE, DOUBLE_REDOUBLE)
	CREATE_COMBO(PBN_EDITOR_COMBO_SCORE, SCORE);
#undef CREATE_COMBO
	m_combo[PBN_EDITOR_COMBO_RESULT] =
			getProblem().noTrumpOrContract() ? NULL : createTextCombobox(0, 13);

	for (i = 0; i < PBN_EDITOR_COMBO_SIZE; i++) {
		if (m_combo[i] != NULL) {
			g_signal_connect(m_combo[i], "changed", G_CALLBACK(combo_changed),
					GP(i));
		}
	}

	for (i = 0; i < PBN_EDITOR_LABEL_SIZE; i++) {
		m_label[i] = gtk_label_new("");
	}

	for (j = 0, i = 0; i < SIZEI(pbnDapEntry); i++) {
		pbnDapEntry[i] = getProblem().findPbnEntry(DAP[i]);
		if (pbnDapEntry[i] != getProblem().pbnEntryEnd()) {
			j++;
		}
	}

	i = getProblem().pbnEntrySize() - j;
	const int rows0 = i / COLUMNS + i % COLUMNS;

	i = 0;
	k = 0;
	for (it = getProblem().pbnEntryBegin(); it != getProblem().pbnEntryEnd();
			it++) {
		if ( INDEX_OF_NO_CASE(it->tag,DAP) != -1) {
			continue;
		}

		w = gtk_label_new(it->tag.c_str());
		gtk_label_set_xalign(GTK_LABEL(w), 0);
		gtk_label_set_yalign(GTK_LABEL(w), 0.5);

		gtk_grid_attach(GTK_GRID(m_grid), w, k, i, 1, 1);

		if ((j = INDEX_OF_NO_CASE(it->tag,CM)) != -1) {
			if (j == 1) {
				if (!getProblem().noTrumpOrContract()) {
					l = strtol(it->value.c_str(), &p, 10);
					assert(*p == 0);
					assert(l >= 0 && l <= 13);
					setComboPosition(m_combo[j], l);
					gtk_grid_attach(GTK_GRID(m_grid), m_combo[j], k + 1, i, 1, 1);
				}
			}
			else {
				if (j == 0) {
					l = INDEX_OF_NO_CASE(it->value,DEALER);
				}
				else if (j == 2) {
					l = INDEX_OF_NO_CASE(it->value,ROOM);
				}
				else {
					l = INDEX_OF_NO_CASE(it->value,VULNERABLE);
				}
				//doesn't work l=INDEX_OF_NO_CASE(j==0 ? DEALER( j==2 ? S_ROOM :S_VULNERABLE),it->value);
				assert(l != -1);
				setComboPosition(m_combo[j], l);
				gtk_grid_attach(GTK_GRID(m_grid), m_combo[j], k + 1, i, 1, 1);
			}
		}
		else if (cmpnocase(it->tag, PBN_DECLARER_TAG)) {
			w = gtk_label_new(it->value.c_str());
			gtk_label_set_xalign(GTK_LABEL(w), 0);
			gtk_label_set_yalign(GTK_LABEL(w), 0.5);
			gtk_grid_attach(GTK_GRID(m_grid), w, k + 1, i, 1, 1);
		}
		else if (cmpnocase(it->tag, PBN_DATE_TAG)) {
			w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

			w = gtk_entry_new();
			m_entry.push_back(w);
			gtk_entry_set_text(GTK_ENTRY(w), it->value.c_str());

			gtk_container_add(GTK_CONTAINER(w1), w);
			gtk_container_add(GTK_CONTAINER(w1), gtk_label_new("YYYY.MM.DD"));

			gtk_grid_attach(GTK_GRID(m_grid), w1, k + 1, i, 1, 1);
		}
		else if (cmpnocase(it->tag, PBN_SCORE_TAG)) {
			w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			gtk_container_add(GTK_CONTAINER(w1), m_combo[PBN_EDITOR_COMBO_SCORE]);
			gtk_label_set_width_chars(GTK_LABEL(m_label[PBN_EDITOR_LABEL_SCORE]), 5);
			gtk_box_pack_start(GTK_BOX(w1), m_label[PBN_EDITOR_LABEL_SCORE], TRUE,
			TRUE, 2);

			l = INDEX_OF_NO_CASE(it->value.substr(0, 2),SCORE);
			assert(l != -1);
			setComboPosition(m_combo[PBN_EDITOR_COMBO_SCORE], l);

			gtk_grid_attach(GTK_GRID(m_grid), w1, k + 1, i, 1, 1);

		}
		else if (cmpnocase(it->tag, PBN_CONTRACT_TAG)) {
			w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
			if (getProblem().noTrumpOrContract()) {
				gtk_box_pack_start(GTK_BOX(w1), gtk_label_new("Pass"), TRUE, TRUE, 2);
			}
			else {
				s = format("%d", getProblem().m_contract);
				gtk_container_add(GTK_CONTAINER(w1), gtk_label_new(s.c_str()));
				if (getProblem().m_trump == NT) {
					w = gtk_label_new(getProblem().getTrumpString().c_str()); //language independent
				}
				else {
					w = gtk_image_new_from_pixbuf(
							getSuitPixbuf(getProblem().m_trump, getFontHeight()));
				}

				gtk_container_add(GTK_CONTAINER(w1), w);

				gtk_box_pack_start(GTK_BOX(w1),
						m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE], TRUE, TRUE, 2);

				setComboPosition(m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE],
						countEndX(it->value));

			}
			gtk_grid_attach(GTK_GRID(m_grid), w1, k + 1, i, 1, 1);
		}
		else {
			w = gtk_entry_new();
			m_entry.push_back(w);
			gtk_widget_set_size_request(w, ENTRY_SIZE, 0);
			gtk_entry_set_text(GTK_ENTRY(w), it->value.c_str());
			gtk_grid_attach(GTK_GRID(m_grid), w, k + 1, i, 1, 1);
		}

		if (i == rows0 - 1) {
			i = 0;
			k += 2;
		}
		else {
			i++;
		}
	}

	for (k = 2 * COLUMNS, i = 1; i < SIZEI(pbnDapEntry); i++) {
		it = pbnDapEntry[i];
		if (it == getProblem().pbnEntryEnd()) {
			continue;
		}

		w = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

		w1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
		gtk_container_add(GTK_CONTAINER(w1), gtk_label_new(it->tag.c_str()));

		gtk_container_add(GTK_CONTAINER(w1),
				i == 1 ?
						m_label[PBN_EDITOR_LABEL_AUCTION] :
						gtk_label_new(it->value.c_str()));

		gtk_container_add(GTK_CONTAINER(w), w1);

		w1 = gtk_text_view_new();
		if (i == 1) {
			m_auction = w1;
			addClass(w1, "whitebackground");
		}
		else {
			gtk_text_view_set_editable(GTK_TEXT_VIEW(w1), FALSE);
			gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(w1), FALSE);
		}

		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(w1));

		/* fillAdjustPbnEntry() adds '\n' at the end of it->add, but if pbn has not many tags, play section
		 * stretches m_grid vertically but it's not good view. This effect appear in problem send to me from Poland.
		 * So remove last symbol
		 */
		s = it->add;
		if (s[s.length() - 1] == '\n') {
			s = s.substr(0, s.length() - 1);
		}
		gtk_text_buffer_set_text(buffer, s.c_str(), -1);

		j = getArea().getTextExtents(TextWithAttributes(S_MEASURE[i - 1])).cx;
		gtk_widget_set_size_request(w1, j, 0);
		gtk_box_pack_start(GTK_BOX(w), w1, TRUE, TRUE, 2);

		gtk_grid_attach(GTK_GRID(m_grid), w, k++, 0, 1, rows0);
	}

	//k use later
	assert(pbnDapEntry[0] != getProblem().pbnEntryEnd());
	w = gtk_label_new(pbnDapEntry[0]->tag.c_str());
	gtk_label_set_xalign(GTK_LABEL(w), 0);
	gtk_label_set_yalign(GTK_LABEL(w), 0.5);
	gtk_grid_attach(GTK_GRID(m_grid), w, 0, rows0, 1, 1);

	w = m_label[PBN_EDITOR_LABEL_DEAL];
	gtk_label_set_text(GTK_LABEL(w), pbnDapEntry[0]->value.c_str());
	gtk_label_set_xalign(GTK_LABEL(w), 0);
	gtk_label_set_yalign(GTK_LABEL(w), 0.5);
	gtk_grid_attach(GTK_GRID(m_grid), w, 1, rows0, k - 1, 1);

	showAll(); //redraw m_grid

	updateAuctionLabel();
	updateDealLabel();
	updateScore();
}

bool PbnEditorDialog::storeProblem() {
	int i, j, k;
	std::string s;
	VPbnEntryI it;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buffer;

	k = 0;
	for (it = getProblem().pbnEntryBegin(); it != getProblem().pbnEntryEnd();
			it++) {
		if ((j = INDEX_OF_NO_CASE(it->tag,DAP)) != -1) {
			if (j == 0) {
				it->value = gtk_label_get_text(
						GTK_LABEL(m_label[PBN_EDITOR_LABEL_DEAL]));
			}
			else if (j == 1) {
				buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(m_auction));
				gtk_text_buffer_get_start_iter(buffer, &start);
				gtk_text_buffer_get_end_iter(buffer, &end);
				it->add = gtk_text_buffer_get_text(buffer, &start, &end, true);
			}
		}
		else if ((j = INDEX_OF_NO_CASE(it->tag,CM)) != -1) {
			if (j == 1 && getProblem().noTrumpOrContract()) {
				it->value = "";
			}
			else {
				i = getComboPosition(m_combo[j]);
				if (j == 0) {
					it->value = DEALER[i];
				}
				else if (j == 1) {
					it->value = format("%d", i);
				}
				else if (j == 2) {
					it->value = ROOM[i];
				}
				else {
					it->value = VULNERABLE[i];
				}
			}
		}
		else if (cmpnocase(it->tag, PBN_DECLARER_TAG)) {
			//skip
		}
		else if (cmpnocase(it->tag, PBN_SCORE_TAG)) {
			i = getComboPosition(m_combo[PBN_EDITOR_COMBO_SCORE]);
			it->value = SCORE[i];
			it->value += ' ';
			it->value += gtk_label_get_text(
					GTK_LABEL(m_label[PBN_EDITOR_LABEL_SCORE]));

		}
		else if (cmpnocase(it->tag, PBN_CONTRACT_TAG)) {
			if (getProblem().m_trump == NO_TRUMP_SET
					|| getProblem().m_contract == NO_CONTRACT_SET) {
				//skip
			}
			else {
				i = getComboPosition(m_combo[PBN_EDITOR_COMBO_DOUBLE_REDOUBLE]);
				it->value = format("%d", getProblem().m_contract)
						+ getProblem().getTrumpString() + DOUBLE_REDOUBLE[i];
			}
		}
		else {
			it->value = gtk_entry_get_text(GTK_ENTRY(m_entry[k++]));
		}
	}

	assert(k == int(m_entry.size()));

	//check date
	it = getProblem().findPbnEntry(PBN_DATE_TAG);
	if (it != getProblem().pbnEntryEnd() && (s = it->value) != "#") { //"#" - allowed
		if (s.length() != 10 || s[4] != '.' || s[7] != '.') {
			message(MESSAGE_ICON_ERROR, STRING_INVALID_DATE);
			return false;
		}
		for (i = 0; i < 10; ++i) {
			if (!isdigit(s[i]) && i != 4 && i != 7) {
				message(MESSAGE_ICON_ERROR, STRING_INVALID_DATE);
				return false;
			}
		}

		i = atoi(s.c_str()); //year
		j = atoi(s.c_str() + 5); //month
		k = atoi(s.c_str() + 8); //day

		if (j < 1 || j > 12) {
			message(MESSAGE_ICON_ERROR, STRING_INVALID_DATE);
			return false;
		}

		int days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; //leap year
		if ((i % 4 == 0) && ((i % 100 != 0) || (i % 400 == 0))) {
			days[1] = 29;
		}

		if (k > days[j - 1] || k <= 0) {
			message(MESSAGE_ICON_ERROR, STRING_INVALID_DATE);
			return false;
		}
	}

	if (getProblem().m_trump != NO_TRUMP_SET
			&& getProblem().m_contract != NO_CONTRACT_SET) {
		if (!getProblem().checkAuctionTag()) {
			return false;
		}
	}
	return true;
}

void PbnEditorDialog::comboChanged(int i) {
	if (i == PBN_EDITOR_COMBO_DEALER) {
		updateAuctionLabel();
		updateDealLabel();
	}
	else if (i != PBN_EDITOR_COMBO_ROOM) {
		updateScore();
	}
}

bool PbnEditorDialog::click(int index) {
	if (index == 0) {
		if (!storeProblem()) {
			return false;
		}
		countModified();
	}
	return true;
}

void PbnEditorDialog::clickButton(int i) {
	if (isMovePossible(i, true)) {
		if (!storeProblem()) {
			return;
		}
		move(i, true);
		redrawProblem();
	}
}

void PbnEditorDialog::updateAuctionLabel() {
	gtk_label_set_text(GTK_LABEL(m_label[PBN_EDITOR_LABEL_AUCTION]),
			DEALER[getComboPosition(m_combo[PBN_EDITOR_COMBO_DEALER])]);
}

void PbnEditorDialog::updateDealLabel() {
	int i = getComboPosition(m_combo[PBN_EDITOR_COMBO_DEALER]);
	gtk_label_set_text(GTK_LABEL(m_label[PBN_EDITOR_LABEL_DEAL]),
			getProblem().getPbnDealString(PLAYER[i]).c_str());
}
