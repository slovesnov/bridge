/*
 * CalculatorDialog.cpp
 *
 *       Created on: 27.12.2016
 *           Author: alexey slovesnov
 * copyright(c/c++): 2016-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "CalculatorDialog.h"

const int areaWidth = 540;
const int areaHeight = 3 * areaWidth / 4;

enum {
	BRIDGE_CALCULATOR_DIALOG_CONTRACT,
	BRIDGE_CALCULATOR_DIALOG_TRUMP,
	BRIDGE_CALCULATOR_DIALOG_TRICKS,
	BRIDGE_CALCULATOR_DIALOG_VULNERABLE,
	BRIDGE_CALCULATOR_DIALOG_DOUBLE_REDOUBLE,

//	BRIDGE_CALCULATOR_DIALOG_SIZE
};

enum {
	PREFERANS_CALCULATOR_DIALOG_CONTRACT,
	PREFERANS_CALCULATOR_DIALOG_TRICKS,
	PREFERANS_CALCULATOR_DIALOG_PLAYERS,
	PREFERANS_CALCULATOR_DIALOG_WHIST_OPTION,

//	PREFERANS_CALCULATOR_DIALOG_SIZE
};

#define BC(a) m_combo[BRIDGE_CALCULATOR_DIALOG_##a]
#define CB(a) getComboPosition(BC(a))

#define PC(a) m_combo[PREFERANS_CALCULATOR_DIALOG_##a]
#define CP(a) getComboPosition(PC(a))

CalculatorDialog *dialog;

static gboolean combo_changed(GtkWidget *w, gpointer) {
	dialog->comboChanged(w);
	return TRUE;
}

static gboolean draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data) {
	dialog->drawArea(cr);
	return FALSE;
}

CalculatorDialog::CalculatorDialog() :
		BaseDialog(MENU_CALCULATOR) {
	int i;
	GtkWidget *w, *w1, *w2;
	std::string s;
	VString v;
	VStringID l;
	VGtkWidgetPtr wv;
	const bool bridge = isBridge();
	const int margin = 4;
	const int hmargin = 11;
	const int topmargin = 10;

	dialog = this;

	for (auto &a : m_score) {
		a = gtk_label_new("");
		gtk_widget_set_margin_top(a, topmargin);
		gtk_widget_set_halign(a, GTK_ALIGN_START);
		gtk_widget_set_margin_start(a, 20); //horizontal space
	}

	if (bridge) {
		//create comboboxes
		m_combo = { createTextCombobox(1, 7), createImageCombobox(),
				createTextCombobox(0, 13), createTextCombobox(STRING_NO,
						STRING_YES), createTextCombobox(DOUBLE_REDOUBLE,
				SIZE(DOUBLE_REDOUBLE)) };

		gtk_combo_box_set_model(GTK_COMBO_BOX(BC(TRUMP)),
				createTrumpModel(false, true, BRIDGE));
		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRUMP)), 0);

		gtk_combo_box_set_active(GTK_COMBO_BOX(BC(TRICKS)), 7);

		l = { STRING_CONTRACT, STRING_NUMBER_OF_TRICKS, STRING_VULNERABLE };
		w = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, margin);
		gtk_container_add(GTK_CONTAINER(w), BC(CONTRACT));
		gtk_container_add(GTK_CONTAINER(w), BC(TRUMP));
		gtk_container_add(GTK_CONTAINER(w), BC(DOUBLE_REDOUBLE));

		wv = { w, BC(TRICKS), BC(VULNERABLE) };

	} else {
		m_combo = { createTextCombobox(6, 10, 1, getString(STRING_MISERE)),
				createTextCombobox(0, 10), createTextCombobox(3, 4),
				createTextCombobox(STRING_WHIST, 3) };

		l = { STRING_CONTRACT, STRING_TRICKS, STRING_PLAYERS,
				STRING_WHIST_OPTION };
		setComboPosition(PC(TRICKS), 6);
	}

	w2 = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(w2), margin);
	gtk_grid_set_row_spacing(GTK_GRID(w2), margin);
	gtk_widget_set_margin_start(w2, hmargin);
	gtk_widget_set_margin_end(w2, hmargin);
	gtk_widget_set_margin_top(w2, 5);
	gtk_widget_set_margin_bottom(w2, 5);
	gtk_widget_set_halign(w2, GTK_ALIGN_CENTER);

	auto &r = bridge ? wv : m_combo;
	assert(l.size() == r.size());
	i = 0;
	for (auto a : l) {
		w = gtk_label_new(getString(a));
		if (isPreferans()) {
			m_label[i] = w;
		}
		gtk_grid_attach(GTK_GRID(w2), w, 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(w2), r[i], 1, i, 1, 1);
		i++;
	}

	w = gtk_grid_new();
	gtk_grid_attach(GTK_GRID(w), w2, 0, 0, 1, 1);
	gtk_grid_set_column_spacing(GTK_GRID(w), 7);

	w1 = gtk_label_new(""); //use label instead of frame to make upper/left corner rounded
	addClass(w1, ROUND_CORNER_CLASS);

	gtk_grid_attach(GTK_GRID(w), w1, 0, 0, 1, 1);

	w1 = gtk_grid_new();

	for (i = 0; i < (isBridge() ? 2 : 4); i++) {
		if (bridge) {
			s = getString(i == 0 ? STRING_DECLARING_SIDE : STRING_DEFENDERS);
		} else {
			s = "";
		}
		w2 = m_label[i + 4] = label(s);
		gtk_widget_set_margin_top(w2, topmargin);
		gtk_widget_set_halign(w2, GTK_ALIGN_END);
		gtk_grid_attach(GTK_GRID(w1), w2, 0, i, 1, 1);
		gtk_grid_attach(GTK_GRID(w1), m_score[i], 1, i, 1, 1);
	}

	if (!bridge) {
		setPreferansLabels();
	}

	gtk_grid_attach(GTK_GRID(w), w1, 0, 1, 1, 1);

	if (isPreferans()) {
		m_area = gtk_drawing_area_new();
		gtk_widget_set_size_request(m_area, areaWidth, areaHeight);
		gtk_grid_attach(GTK_GRID(w), m_area, 1, 0, 1, 2);
		g_signal_connect(G_OBJECT (m_area), "draw", G_CALLBACK (draw_callback),
				NULL);
	}

	gtk_container_add(GTK_CONTAINER(getContentArea()), w);

	for (auto a : m_combo) {
		g_signal_connect(a, "changed", G_CALLBACK(combo_changed), 0);
	}

	updateScore();
	VGtkWidgetPtr ve = { m_label[7], m_score[3] };
	showExclude(ve);
}

void CalculatorDialog::updateScore() {
	int i;
	std::string v[4];

	if (isBridge()) {
		int r = countBridgeScore(CB(CONTRACT) + 1, CB(TRUMP), CB(TRICKS),
				CB(DOUBLE_REDOUBLE), CB(VULNERABLE));
		for (i = 0; i < 2; i++) {
			v[i] = std::to_string(i == 0 ? r : -r);
		}
	} else {
		const int players = getPreferansPlayers();
		const int contract = getPreferansContract();
		const int tricks = CP(TRICKS);
		WHIST_OPTION wo = WHIST_OPTION(CP(WHIST_OPTION));
		if (isMisere() || wo == WHIST_OPTION_WHIST) {
			m_pscore.setGame(players, contract, tricks);
		} else {
			m_pscore.setNonPlayingGame(players, contract,
					wo == WHIST_OPTION_HALFWHIST);
		}
		for (i = 0; i < 4; i++) {
			v[i] = preferansScoreToString(i);
		}
	}

	i = 0;
	for (auto a : m_score) {
		gtk_label_set_text(GTK_LABEL(a), v[i].c_str());
		i++;
	}
}

void CalculatorDialog::setPreferansLabels() {
	int i;
	std::string s;
	gtk_label_set_text(GTK_LABEL(m_label[4]),
			getString(isMisere() ? STRING_MISERE_PLAYER : STRING_DECLARER));
	for (i = 0; i < 3; i++) {
		s = getString(isMisere() ? STRING_CATCHER : STRING_WHISTER)
				+ std::to_string(i + 1);
		gtk_label_set_text(GTK_LABEL(m_label[i + 5]), s.c_str());
	}
}

int CalculatorDialog::getPreferansContract() {
	const int c = CP(CONTRACT) + 6;
	return c == 11 ? 0 : c;
}

int CalculatorDialog::getPreferansPlayers() {
	return CP(PLAYERS) + 3;
}

bool CalculatorDialog::isMisere() {
	return getPreferansContract() == 0;
}

void CalculatorDialog::showHideRow(int row, bool show) {
	showHideWidget(m_label[row], show);
	showHideWidget(m_combo[row], show);
}

void CalculatorDialog::comboChanged(GtkWidget *w) {
	updateScore();
	if (isPreferans()) {
		setPreferansLabels();
		showHideRow(1, isMisere() || CP(WHIST_OPTION) == WHIST_OPTION_WHIST); //tricks
		showHideRow(3, !isMisere()); //whist option
		if (w == m_combo[PREFERANS_CALCULATOR_DIALOG_PLAYERS]) {
			bool players4 = getPreferansPlayers() == 4;
			showHideWidget(m_score[3], players4);
			showHideWidget(m_label[3 + 4], players4);
		}
		gtk_widget_queue_draw(m_area);
	}
}

std::string CalculatorDialog::preferansScoreToString(int i) {
	auto score = m_pscore.score(i);
	auto s = toString(score);
	std::size_t p;
	if (getPreferansPlayers() == 3 && (p = s.find('.')) != std::string::npos) {
		p++;
		s = s.substr(0, p) + '(' + s[p] + ')';
	}
	if (score > 0) {
		s = '+' + s;
	}
	return s;
}

void CalculatorDialog::drawArea(cairo_t *cr) {
	const GdkRGBA bg = { 0xF6 / 255., 0xF4 / 255., 0xBA / 255., 1 };
	const GdkRGBA linecolor = { 0, 0, 0, 1 };
	const int textmargin = areaWidth * .01;
	const double fontsize = areaWidth / 18;
	const double r = std::min(areaWidth, areaHeight) / 15;
	const double whistAreaPercent = 20;
	const double pw = areaWidth * whistAreaPercent / 200;
	const double ph = areaHeight * whistAreaPercent / 200;
	const int players = getPreferansPlayers();

	int i, j, k;
	double x, y, v;
	std::string s;
	DRAW_TEXT xo, yo;

	setFont(cr, fontsize);

	cairo_set_line_width(cr, 1);

	cairo_rectangle(cr, 0, 0, areaWidth, areaHeight);
	gdk_cairo_set_source_rgba(cr, &bg);
	cairo_stroke_preserve(cr);
	cairo_fill(cr);

	gdk_cairo_set_source_rgba(cr, &linecolor);

	//whist lines
	auto hwhist = [players](int i) {
		return areaWidth / (players - 1) * i;
	};
	auto vwhist = [players](int i) {
		return areaHeight / (players - 1) * i;
	};
	auto player = [players](int i) {
		return (i + players - 2) % players;
	};

	for (j = 0; j < 5; j++) {
		for (i = players == 3; i < 4; i++) {
			k = player(i);
			if (j == 1) {
				k = m_pscore.pool(k);
			} else if (j == 2) {
				k = m_pscore.dump(k);
			} else {
				k = m_pscore.whist(k, (k + (j == 0 ? 1 : j - 1)) % players);
			}
			if (k == 0) {
				continue;
			}
			s = std::to_string(k);

			if (i % 2) { //3 west (east)
				x = pw / 2 + (j < 3 ? j : 0) * pw;
				if (j > 2) {
					y = vwhist(j - 2) + textmargin * (j - 2);
				} else {
					y = (j + 1) * ph;
				}
				if (players == 3 && i == 3 && j < 3) {
					y = textmargin;
				}
			} else { //2 south (north)
				if (j > 2) {
					x = hwhist(j - 2) + textmargin * (j - 2);
				} else {
					x = (j < 3 ? j + 1 : 1) * pw;
				}
				y = areaHeight - ph / 2 - (j < 3 ? j : 0) * ph;
			}

			cairo_save(cr);
			if (i < 2) {
				cairo_translate(cr, areaWidth - x, areaHeight - y);
				cairo_rotate(cr, G_PI);
			} else {
				cairo_translate(cr, x, y);
			}
			if (i % 2) {
				cairo_rotate(cr, G_PI / 2);
			}
			drawText(cr, s, 0, 0, DRAW_TEXT_BEGIN, DRAW_TEXT_CENTER);
			cairo_restore(cr);
		}
	}

	cairo_move_to(cr, players == 3 ? areaWidth / 2 : 0,
			players == 3 ? areaHeight / 2 : 0);
	cairo_line_to(cr, areaWidth, areaHeight);
	cairo_move_to(cr, players == 3 ? areaWidth / 2 : areaWidth,
			players == 3 ? areaHeight / 2 : 0);
	cairo_line_to(cr, 0, areaHeight);

	if (players == 3) {
		cairo_move_to(cr, areaWidth / 2, 0);
		cairo_line_to(cr, areaWidth / 2, areaHeight / 2);
		for (i = 1; i < 3; i++) {
			x = pw * i;
			cairo_move_to(cr, x, 0);
			v = areaHeight / 2 + (areaWidth / 2 - x) / areaWidth * areaHeight;
			cairo_line_to(cr, x, v);
			cairo_line_to(cr, areaWidth - x, v);
			cairo_line_to(cr, areaWidth - x, 0);
		}
	} else {
		for (i = 1; i < 3; i++) {
			x = pw * i;
			y = ph * i;
			cairo_rectangle(cr, x, y, areaWidth - 2 * x, areaHeight - 2 * y);
		}
	}

	for (i = 0; i < players - 2; i++) {
		//vertical whist lines
		v = hwhist(i + 1);
		cairo_move_to(cr, v, 0);
		cairo_line_to(cr, v, ph);
		cairo_move_to(cr, v, areaHeight);
		cairo_line_to(cr, v, areaHeight - ph);

		//horizontal whist lines
		v = vwhist(i + 1);
		cairo_move_to(cr, 0, v);
		cairo_line_to(cr, pw, v);
		cairo_move_to(cr, areaWidth, v);
		cairo_line_to(cr, areaWidth - pw, v);
	}

	cairo_stroke(cr);

	cairo_arc(cr, areaWidth / 2, areaHeight / 2, r, 0, 2 * G_PI);
	cairo_stroke_preserve(cr);
	gdk_cairo_set_source_rgba(cr, &bg);
	cairo_fill(cr);

	//draw scores
	gdk_cairo_set_source_rgba(cr, &linecolor);
	//0 - north, 1 - east, ...
	for (i = players == 3; i < 4; i++) {
		s = preferansScoreToString(player(i));
		if (i % 2) {
			x = (r + textmargin) * (i == 3 ? -1 : 1);
			xo = i == 3 ? DRAW_TEXT_END : DRAW_TEXT_BEGIN;
			y = 0;
			yo = DRAW_TEXT_CENTER;
		} else {
			x = 0;
			xo = DRAW_TEXT_CENTER;
			y = (r + textmargin) * (i == 0 ? -1 : 1);
			yo = i == 0 ? DRAW_TEXT_END : DRAW_TEXT_BEGIN;
		}

		drawText(cr, s, areaWidth / 2 + x, areaHeight / 2 + y, xo, yo);
	}

}
