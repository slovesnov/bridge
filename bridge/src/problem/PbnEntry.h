/*
 * PbnEntry.h
 *
 *       Created on: 24.12.2016
 *           Author: alexey slovesnov
 * copyright(c/c++): 2016-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef HELPER_PBNENTRY_H_
#define HELPER_PBNENTRY_H_

#include <string>
#include <string.h>
#include <vector>

class PbnEntry {
public:
	std::string tag;
	std::string value;
	std::string add;

	PbnEntry() {
	}

	PbnEntry(const std::string& t, const std::string& v, const std::string& a) {
		tag = t;
		value = v;
		add = a;
	}

	bool operator==(const PbnEntry& e) const {
		return tag == e.tag && value == e.value && add == e.add;
	}

	bool operator!=(const PbnEntry& e) const {
		return !(*this == e);
	}

#ifndef NDEBUG
	std::string toString() const {
		std::string s = tag + " \"" + value + "\"";
		if (add.length() != 0) {
			s += "[" + add + "]";
		}
		return s;
	}
#endif
};

typedef std::vector<PbnEntry> VPbnEntry;
typedef VPbnEntry::iterator VPbnEntryI;
typedef VPbnEntry::const_iterator VPbnEntryCI;

#endif /* HELPER_PBNENTRY_H_ */
