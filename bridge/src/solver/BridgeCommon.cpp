/*
 * BridgeCommon.cpp
 *
 *       Created on: 28.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include <cstring>

#include "BridgeCommon.h"

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

std::string binaryCodeString(int c, int miminumPrintBits /*= 0*/) {
	assert(miminumPrintBits<=32);
	const int f=64;
	char b[128], h[50], *pb=b+f, *ph=h;
	itoa(c, b+f, 2);
	int i,l = strlen(b+f);
	if (miminumPrintBits && (i=miminumPrintBits-l)>0 ) {
		memset(b+f-i, '0', i);
		pb-=i;
	}
	l = strlen(pb) & 1;
	for (i = 0; *pb != 0; i++) {
		if ((i & 1) == l && i != 0) {
			*ph++ = '_';
		}
		*ph++ = *pb++;
	}
	*ph = 0;
	return h;
}
