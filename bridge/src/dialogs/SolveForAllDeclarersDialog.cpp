/*
 * SolveForAllDeclarersDialog.cpp
 *
 *       Created on: 30.11.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "SolveForAllDeclarersDialog.h"
#include "../Frame.h"

static void close_dialog(SolveForAllDeclarersDialog *, gint response_id,
		gpointer) {
	gdraw->stopSolveAllDeclarersBridgeThreads();
}

SolveForAllDeclarersDialog::SolveForAllDeclarersDialog(const int*r) :
		BaseDialog(MENU_SOLVE_FOR_ALL_DECLARERS, false) {
	int i, j, k, l;
	GtkWidget *w, *g, *w1, *w2, *border;
	std::string s, s1;

	const bool bridge = r == NULL;
	const int h = 16;
	const int titleH = bridge ? 2 * h : 3 * h / 2;
	const char BIG_FONT[] = "bigFont"; //font for titles [NT, misere, number of tricks, contract]
	const int sp = bridge ? 8 : 0;

	//Note font for labels (except captions) is defined is bridge.css It looks like smaller font but actually it is optical illusion.
	//Pictures are checked in paint (windows application)
	const int COLUMNS = NT + 1 + !bridge;

	w2 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	for (l = 0; l < (bridge ? 1 : 3); l++) {
		w1 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

		for (k = 0; k < 2; k++) {
			g = gtk_grid_new();
			gtk_grid_set_column_spacing(GTK_GRID(g), sp);
			gtk_grid_set_row_spacing(GTK_GRID(g), sp);

			if (bridge) {
				m_loading[k] = w = gtk_spinner_new();
				gtk_spinner_start(GTK_SPINNER(w));
				gtk_grid_attach(GTK_GRID(g), w, 0, 1, 1, 1);
			}

			w = gtk_label_new(
					getString(k == 0 ? STRING_NUMBER_OF_TRICKS : STRING_CONTRACT));
			gtk_grid_attach(GTK_GRID(g), w, 0, 0, 6 + isPreferans(), 1);
			if (bridge) {
				addClass(w, BIG_FONT);
			}

			for (i = 0; i < COLUMNS; i++) {
				if (i >= NT) {
					w = gtk_label_new(
							i == NT ? getNTString().c_str() : getString(STRING_MISERE));
					if (bridge) {
						addClass(w, BIG_FONT);
					}
				}
				else {
					w = getSuitImage(i, titleH);
				}
				gtk_grid_attach(GTK_GRID(g), w, i + 1, 1, 1, 1);
			}

			for (i = 0; i < maxTableCards(); i++) {
				for (j = 0; j <= COLUMNS; j++) {
					if (j == 0) {
						w = gtk_label_new(
								getPlayerString(bridge ? PLAYER[i] : getPreferansPlayer(i)));
						if (bridge) {
							addClass(w, BIG_FONT);
						}
						gtk_label_set_xalign(GTK_LABEL(w), 0);
					}
					else {
						m_label[i][j - 1][l][k] = w = gtk_label_new("?");
						if (bridge) {
							addClass(w, BIG_FONT);
						}
					}

					gtk_grid_attach(GTK_GRID(g), w, j, i + 2, 1, 1);
				}
			}

			gtk_container_add(GTK_CONTAINER(w1), g);

		}
		if (bridge) {
			gtk_container_add(GTK_CONTAINER(w2), w1);
		}
		else {
			s = getString(STRING_FIRST_MOVE);
			s += " - ";
			s1 = getPlayerString(getPreferansPlayer(l));
			s += utf8ToLowerCase(s1);
			border = gtk_frame_new(s.c_str());
			gtk_frame_set_label_align(GTK_FRAME(border), 0.2, 0.5);
			gtk_container_add(GTK_CONTAINER(border), w1);
			gtk_container_add(GTK_CONTAINER(w2), border);
		}
	}
	gtk_container_add(GTK_CONTAINER(getContentArea()), w2);

	if (bridge) {
		g_signal_connect(getWidget(), "response", G_CALLBACK(close_dialog),
				gpointer(0));
	}
	else {
		int trump, v, first;
		bool misere;
		CARD_INDEX player;
		for (v = 0; v < MAX_THREADS[1]; v++) {
			parsePreferansSolveAllDeclarersParameters(v, trump, misere, player);
			for (first = 0; first < 3; first++) {
				l = r[v * 3 + first];
				s = format("%d", l);
				auto q = m_label[indexOfPreferansPlayer(player)][misere ? 5 : trump][first];
				gtk_label_set_label(GTK_LABEL(q[0]), s.c_str());

				if (misere) {
					s1 = l == 0 ? "+" : "-";
				}
				else {
					s1 = l > 5 ? s : "-";
				}
				gtk_label_set_label(GTK_LABEL(q[1]), s1.c_str());

			}
		}
	}

	m_searches = getMaxSolveAllThreads();

	show();

}

void SolveForAllDeclarersDialog::setBridgeLabel(int trump) {
	int i,v;
	GtkWidget**w;

	for (i = 0; i < SIZEI(PLAYER); i++) {
		v=gdraw->m_solveAllDeclarersBridgeResult[trump][i];
		CARD_INDEX ci = getPreviousBridgePlayer(PLAYER[i]);
		w = m_label[indexOfPlayer(ci)][trump][0];
		gtk_label_set_text(GTK_LABEL(w[0]), format("%d", v).c_str());
		gtk_label_set_text(GTK_LABEL(w[1]),
				v > 6 ? format("%d", v - 6).c_str() : "-");

	}
	if (--m_searches == 0) {
		for (i = 0; i < 2; i++) {
			gtk_spinner_stop (GTK_SPINNER(m_loading[i]));
		}
	}
}
