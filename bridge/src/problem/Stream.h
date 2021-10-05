/*
 * Stream.h
 *
 *       Created on: 24.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef STREAM_H_
#define STREAM_H_

#include <string>

class Stream {
	std::string m_content;
	unsigned m_contentPtr;

public:
	void init(std::string s) {
		m_content = s;
		m_contentPtr = 0;
	}

	Stream() {
		m_contentPtr = 0;
	}

	inline bool end() {
		return bytesLeft() <= 0;
	}

	inline int bytesLeft() {
		return m_content.length() - m_contentPtr;
	}

	inline int integersLeft() { //can read at least [return value] integers from stream
		return bytesLeft() / 4;
	}

	inline void readDummyBytes(int bytes) {
		m_contentPtr += bytes;
	}

	inline void readDummyIntegers(int n) {
		readDummyBytes(n * 4);
	}

	int readInteger();
	std::string readString();
	void getLastTwoIntegers(int& i1, int& i2);

};

#endif /* STREAM_H_ */
