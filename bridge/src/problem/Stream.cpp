/*
 * Stream.cpp
 *
 *       Created on: 24.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "Stream.h"
#include "../base/Widget.h"

int Stream::readInteger() {
	int i;
	assert(sizeof(int) == 4);
	if (bytesLeft() < 4) {
		println("error ProblemVector::readInteger() %d %d\n", m_contentPtr,
				int(m_content.length()));
		return 0;
	}

	memcpy(&i, m_content.c_str() + m_contentPtr, sizeof(int));
	m_contentPtr += 4;
	return i;
}

void Stream::getLastTwoIntegers(int& i1, int& i2) {
	memcpy(&i1, m_content.c_str() + m_contentPtr + bytesLeft() - 8, sizeof(int));
	memcpy(&i2, m_content.c_str() + m_contentPtr + bytesLeft() - 4, sizeof(int));
}

std::string Stream::readString() {
	std::string s;
	/* should be unsigned char! 30dec2016
	 * unsigned char length=m_content[m_contentPtr];
	 * char c=0x80;
	 * int length=c //length=-128 0xffffff80
	 *
	 * char c=0x80;
	 * unsigned char length=c; length=128 0x80
	 */
	unsigned char l1 = m_content[m_contentPtr];
	unsigned short length;
	m_contentPtr++;
	if (l1 == 0xff) { //first byte =-1 means next two bytes have length
		memcpy((void*) (&length), m_content.c_str() + m_contentPtr, 2); //read two bytes
		m_contentPtr += 2;
	}
	else {
		length = l1;
	}

	s = m_content.substr(m_contentPtr, length);
	m_contentPtr += length;

	return localeToUtf8(s);
}

