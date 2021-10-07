/*
 * BridgeCommon.cpp
 *
 *       Created on: 28.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "BridgeCommon.h"

std::string secondsToString(double seconds){
	int h,m,s,t=int(seconds);
	h = t/3600;
	t = t%3600;
	m = t/60;
	t = t%60;
	s = t;
	if(h>0){
		return format("%d:%02d:%02d",h,m,s);
	}
	else{
		return format("%d:%02d",m,s);
	}
}

std::string secondsToString(clock_t end,clock_t begin){
	return secondsToString(double(end - begin) / CLOCKS_PER_SEC);
}

bool eastOrWest(CARD_INDEX i) {
	return i == CARD_INDEX_EAST || i == CARD_INDEX_WEST;
}

bool east(CARD_INDEX i) {
	return i == CARD_INDEX_EAST;
}

bool west(CARD_INDEX i) {
	return i == CARD_INDEX_WEST;
}

bool northOrSouth(CARD_INDEX i) {
	return i == CARD_INDEX_NORTH || i == CARD_INDEX_SOUTH;
}

bool north(CARD_INDEX i) {
	return i == CARD_INDEX_NORTH;
}

bool south(CARD_INDEX i) {
	return i == CARD_INDEX_SOUTH;
}

std::string binaryCodeString(int c){
	char b[33],*pb=b;
	itoa(c,b,2);
	char h[50],*ph=h;
	for(int i=0; *pb!=0 ; i++){
		if(i%2==0 && i!=0){
			*ph++='_';
		}
		*ph++=*pb++;
	}
	*ph=0;
	return h;
}

