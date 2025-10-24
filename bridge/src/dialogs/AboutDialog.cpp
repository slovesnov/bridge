/*
 * AboutDialog.cpp
 *
 *       Created on: 20.07.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#include "../Frame.h"
#include "AboutDialog.h"

static gboolean label_clicked(GtkWidget *label, const gchar *uri,
		AboutDialog *d) {
	d->click(label);
	return TRUE;
}

AboutDialog::AboutDialog() :
		BaseDialog(MENU_ABOUT) {
	int i;
	GtkWidget *box, *hbox, *label, *g;
	bool link;
	VString v, q;
	Pixbuf pb, np;
	std::string s;
	const CPoint LMARGIN(5, 0);
	const int spacing = 5;

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	v = split(getString(STRING_ABOUT), "\n");
	v.push_back(getBuildVersionString(false));
	v.push_back(
			"executable file size " + toString(getApplicationFileSize(), ','));

	i = (getArea().getTextExtents(
			TextWithAttributes::createUnderlinedText("Qy")).y + 2 * LMARGIN.y)
			* v.size() - spacing;
	if (i % 2 == 1) {
		i--;
	}
	createSvgSuits(i / 2);

	const int T[] = { 1, 0, 3, 2 };
	g = gtk_grid_new();
	gtk_grid_set_row_spacing(GTK_GRID(g), spacing);
	gtk_grid_set_row_spacing(GTK_GRID(g), spacing);
	for (i = 0; i < 4; i++) {
		gtk_grid_attach(GTK_GRID(g), gtk_image_new_from_pixbuf(m_suits[i]),
				T[i] % 2, T[i] / 2, 1, 1);
	}

	char *markup;
	i = 0;
	for (auto a : v) {
		link = i == 4;
		//Note mailto:... works incorrect. Background window with error appears.
		if (link) {
			q = split(a, "#");
		}

		if (link) {
			label = gtk_label_new(NULL);
			markup = g_markup_printf_escaped("%s <a href=\"#\">\%s</a>",
					q[0].c_str(), q[1].c_str());
			gtk_label_set_markup(GTK_LABEL(label), markup);
			g_free(markup);
			g_signal_connect(label, "activate-link", G_CALLBACK(label_clicked),
					gpointer(this));
		} else {
			label = gtk_label_new(a.c_str());
		}

		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_widget_set_margin_start(label, LMARGIN.x);
		gtk_widget_set_margin_end(label, LMARGIN.x);
		gtk_container_add(GTK_CONTAINER(box), label);

		/* label style in css if need later
		 dialog.background.solid-csd box.vertical.dialog-vbox box.horizontal box.vertical label{
		 padding:10px;
		 }*/

		i++;
	}

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
	gtk_container_add(GTK_CONTAINER(hbox), g);
	gtk_container_add(GTK_CONTAINER(hbox), box);
	gtk_container_add(GTK_CONTAINER(getContentArea()), hbox);

	show();
}

void AboutDialog::click(GtkWidget *label) {
	openHomepage();
}

void AboutDialog::createSvgSuits(int size) {
	int i, j, sx, dx, w, h;
	const int st[] = { 597, 68, 414, 236 };
	Pixbuf im;

	Pixbuf pb(getImagePath("suits.svg"));
	h = pb.height();
	if (h % 2) {
		h--;
	}

	im.createRGB(h, h);
	for (i = 0; i < 4; i++) {
		j = st[i] - h / 2;
		sx = std::max(j, 0);
		dx = j >= 0 ? 0 : -j;
		w = std::min(h, pb.width() - sx) - dx;
		if (w != h) { //not fully copied, fill transparent color
			gdk_pixbuf_fill(im, 0);
		}
		gdk_pixbuf_copy_area(pb, sx, 0, w, h, im, dx, 0);
		m_suits[i] = gdk_pixbuf_scale_simple(im, size, size,
				GDK_INTERP_BILINEAR);
	}
}
