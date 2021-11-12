/*
 * SvgParameters.h
 *
 *       Created on: 10.12.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef SVGPARAMETERS_H_
#define SVGPARAMETERS_H_

#include <gtk/gtk.h>
#include <string>
#include <atomic>

#include "Pixbuf.h"

class SvgParameters {
	std::atomic_bool loaded;
public:
	int suitsOrder[4], cardsOrder[13];

	//svg file parameters
	double cw, ch; //card width, height
	int startx, starty;//top left corner of very first card
	int addx,addy;//margin between cards

	Pixbuf p;

	SvgParameters();
	~SvgParameters();
	double getScale(int width);
	void loadPixbuf(std::string const& file);
	bool isLoaded();
	double getScaledHeight(int width);
};

#endif /* SVGPARAMETERS_H_ */
