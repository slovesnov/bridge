/*
 * DigitEntry.cpp
 *
 *       Created on: 16.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "DigitEntry.h"

static void entry_insert(GtkWidget *entry, gchar *new_text,
		gint new_text_length, gpointer position, DigitEntry *e) {
	e->highlight();
}

static void entry_delete(GtkWidget *entry, gint start_pos, gint end_pos,
		DigitEntry *e) {
	e->highlight();
}

void DigitEntry::create(int min, int max, int startValue,
		ButtonsDialog *dialog) {
	m_widget = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_widget), format("%d", startValue).c_str());
	m_min = min;
	m_max = max;
	m_dialog = dialog;
	m_value = startValue;

	g_signal_connect_after(G_OBJECT(m_widget), "insert-text",
			G_CALLBACK(entry_insert), gpointer(this));
	g_signal_connect_after(G_OBJECT(m_widget), "delete-text",
			G_CALLBACK(entry_delete), gpointer(this));
}

DigitEntry::~DigitEntry() {
}

void DigitEntry::check() {
	char *q;
	long v = strtol(gtk_entry_get_text(GTK_ENTRY(m_widget)), &q, 10);
	m_valid = *q == 0 && v >= m_min && v <= m_max;
	if (m_valid) {
		m_value = v;
	}
}

void DigitEntry::highlight() {
	check();
	if (m_valid) {
		removeClass(m_widget, "cerror");
	} else {
		addClass(m_widget, "cerror");
	}
	m_dialog->enableButtons();
}
