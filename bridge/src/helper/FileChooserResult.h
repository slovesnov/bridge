/*
 * FileChooserResult.h
 *
 *       Created on: 03.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef HELPER_FILECHOOSERRESULT_H_
#define HELPER_FILECHOOSERRESULT_H_

#include "../base/Base.h"

class FileChooserResult {
public:
	VString m_files;
	gint m_response;

	int size() const {
		return m_files.size();
	}

	VStringCI begin() const {
		return m_files.begin();
	}

	VStringCI end() const {
		return m_files.end();
	}

	bool ok() const {
		return m_response != GTK_RESPONSE_DELETE_EVENT;
	}

	void add(const char *file) {
		m_files.push_back(file);
	}

	std::string& operator[](int i) {
		assert(i >= 0 && i < size());
		return m_files[i];
	}

	const VString& files() const {
		return m_files;
	}

	std::string const& file() const {
		assert(size() == 1);
		return m_files[0];
	}

};

#endif /* HELPER_FILECHOOSERRESULT_H_ */
