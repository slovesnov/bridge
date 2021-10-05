/*
 * SC.h
 *
 *       Created on: 16.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#ifndef SC_H_
#define SC_H_

#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>
#ifndef FINAL_RELEASE
#include <string>
#include "BridgeCommon.h"
#endif

union USC {
	struct {
		int8_t c, s;
	};
	int16_t sc;

	inline int toIndex() const {
		return s * 13 + c;
	}

	inline void set(int _c, int _s) {
		c = _c;
		s = _s;
	}

	inline void fromIndex(int i){
		c=i%13;
		s=i/13;
	}

#ifndef FINAL_RELEASE
	std::string toString()const{
		std::string q;
		q+=RANK[c];
		return q+SUITS_CHAR[s];
	}

	void print()const{
		printl(toString());
	}

	bool operator==(USC const& o)const{
		return sc==o.sc;
	}

	bool operator!=(USC const& o)const{
		return sc!=o.sc;
	}

	//for SC equalsOrder
	bool operator<(USC const& o)const{
		return toIndex()<o.toIndex();
	}
#endif

};

struct SC {
	static const int ml = 13;
	USC a[ml];
	int length;
	int o;//TODO

	SC() {
		length = 0;
	}

	inline void push(USC sc) {
		assert(length < ml);
		a[length++].sc = sc.sc;
	}

	inline void push(int c,int s) {
		assert(length < ml);
		a[length++].sc = (s<<8) | c;
	}

	inline USC operator[](int i) const {
		assert(i >= 0 && i < ml);
		return a[i];
	}

	inline void copy(SC const & o){
		length=o.length;
		memcpy(a,o.a,length*sizeof(int16_t));
	}

#ifndef FINAL_RELEASE
	SC(SC const& o,bool order);

	void print(const char*p=nullptr)const;

	void printOrdered(const char*p=nullptr)const;

	bool operator==(SC const& o)const;

	bool operator!=(SC const& o)const{
		return !operator==(o);
	}

	bool equalsSequence(SC const& o,const int code[4])const;

	//return true if sequences are equal but can have different moves order
	bool equalsOrder(SC const& o)const;

#endif

};

typedef std::vector<SC> VSC;

#endif /* SC_H_ */
