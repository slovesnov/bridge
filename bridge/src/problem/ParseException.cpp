/*
 * ParseException.cpp
 *
 *       Created on: 01.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "ParseException.h"
#include "../base/Base.h"

ParseException::ParseException(const char* message, const STRING_ID& error,
		const char* file, int line, const char* function,
		const std::string& content, const std::string& addon) {
	m_message = message;
	m_error = error;
	m_file = file;
	m_line = line;
	m_function = function;
	m_content = content;
	m_addon = addon;
}

std::string ParseException::getPlaceInfo() const {
	const char*p = strrchr(m_file.c_str(), G_DIR_SEPARATOR);
	assert(p);
	return format("%s:%d\n%s()", p + 1, m_line, m_function.c_str());
}

std::string ParseException::getErrorString() const {
	std::string s = getString(m_error);
	if (m_addon.length() != 0) {
		s += " ";
		s += m_addon;
	}
	return s;
}

ParseException::~ParseException() {
}

