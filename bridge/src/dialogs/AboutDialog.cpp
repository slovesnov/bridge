/*
 * AboutDialog.cpp
 *
 *       Created on: 20.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "../Frame.h"
#include "AboutDialog.h"

static gboolean label_clicked(GtkWidget *label, const gchar *uri,
		AboutDialog*d) {
	d->click(label);
	return TRUE;
}

AboutDialog::AboutDialog() :
		BaseDialog(MENU_ABOUT) {
	int i, j, w, h, from, to;
	GtkWidget *box, *hbox, *label;
	bool link;
	VString v;
	VStringI it;
	GdkPixbuf *pb, *np=0;
	std::string s;
	const CSize LMARGIN(5, 0);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	v = split(getString(STRING_ABOUT), "\n");

	v.push_back(getBuildVersionString(false));

	v.push_back("executable file size " + toString(getApplicationFileSize(),','));
	m_labels = v.size();
	m_label = new GtkWidget*[m_labels];

	//BEGIN scale suits.svg to fit dialog
	int st[] = { 68, 236, 414, 597 };
	//count "he" after m_labels
	int he = (getArea().getTextExtents(
			TextWithAttributes::createUnderlinedText("Qy")).cy + 2 * LMARGIN.cy)
			* m_labels;
	if (he % 2 == 1) {
		he--; //should be even otherwise warning and bad drawing
	}

	getPixbufWH("suits.svg",w,h);
	s = getImagePath("suits.svg");

	const double scale = double(he) / h / 2;

	const int wi = w * he / h / 4; //width of np
	pb = gdk_pixbuf_new_from_file_at_size(s.c_str(), 2 * wi, he / 2, NULL);

	createNew(np,he,he);
	gdk_pixbuf_fill(np, 0);

	for (i = 0; i < 4; i++) {
		st[i] *= scale;
		from = i == 0 ? 0 : st[i] - he / 4;
		to = i == 3 ? 2 * wi : st[i] + he / 4;

		//move heart symbol up
		j = i == 0 ? 7 * scale : 0;
		gdk_pixbuf_copy_area(pb, from, j, to - from, he / 2 - j, np,
				(i == 2 || i == 3 ? 3 : 1) * he / 4 + from - st[i],
				i == 1 || i == 2 ? he / 2 : 0);
	}
	free(pb);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
	gtk_container_add(GTK_CONTAINER(hbox), gtk_image_new_from_pixbuf(np));
	//END scale suits.svg to fit dialog

	for (it = v.begin(), i = 0; it != v.end(); it++, i++) {
		link = i == 5;
		//Note mailto:... works incorrect. Background window with error appears.
		const char *format = link ? "<a href=\"#\">\%s</a>" : "%s";
		char *markup;

		label = m_label[i] = gtk_label_new("");
		markup = g_markup_printf_escaped(format, it->c_str());
		gtk_label_set_markup(GTK_LABEL(label), markup);
		g_free(markup);

		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_margin_start(label, LMARGIN.cx);
		gtk_widget_set_margin_end(label, LMARGIN.cx);
//		gtk_widget_set_margin_top(label, LMARGIN.cy);
//		gtk_widget_set_margin_bottom(label, LMARGIN.cy);
		gtk_container_add(GTK_CONTAINER(box), label);
		if (link) {
			g_signal_connect(label, "activate-link", G_CALLBACK(label_clicked),
					gpointer(this));
		}

		/* label style in css if need later
		 dialog.background.solid-csd box.vertical.dialog-vbox box.horizontal box.vertical label{
		 padding:10px;
		 }*/
	}
	gtk_container_add(GTK_CONTAINER(hbox), box);
	gtk_container_add(GTK_CONTAINER(getContentArea()), hbox);

	show();
}

AboutDialog::~AboutDialog() {
	delete[] m_label;
}

void AboutDialog::click(GtkWidget* label) {
	//cann't use indexof
	int i;
	for (i = 0; i < m_labels; i++) {
		if (m_label[i] == label) {
			if (i == 5) {
				openURL(BASE_ADDRESS);
			}
			return;

		}
	}
}
