/*
 * SvgParameters.cpp
 *
 *       Created on: 10.12.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "SvgParameters.h"
#include "../base/Base.h"

SvgParameters::SvgParameters() {
	p = 0;
	startx = starty = addx = addy = 0;
	int i;
	for (i = 0; i < 13; i++) {
		cardsOrder[i] = i == 12 ? 0 : i+1;
	}
	loaded=false;
	g_mutex_init(&mutex);
}

SvgParameters::~SvgParameters() {
	destroy (p);
	g_mutex_clear(&mutex);
}

void SvgParameters::loadPixbuf(std::string const& file){
	if(isLoaded()){//not need to load many times
		return;
	}
	p = pixbuf(file);
	g_mutex_lock(&mutex);
	loaded=true;
	g_mutex_unlock(&mutex);
}

double SvgParameters::getScale(int width) {
	return width / cw;
}

double SvgParameters::getScaledHeight(int width){
	return  ch *width / cw;
}

bool SvgParameters::isLoaded() {
	g_mutex_lock(&mutex);
	bool b=loaded;
	g_mutex_unlock(&mutex);
	return b;
}
