/*
 * ParseException.h
 *
 *       Created on: 01.01.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef PROBLEM_PARSEEXCEPTION_H_
#define PROBLEM_PARSEEXCEPTION_H_

#include "../helper/Enums.h"
#include <vector>
#include <string>

class ParseException {
public:
	ParseException(const char *message, const STRING_ID &error,
			const char *file, int line, const char *function,
			const std::string &content, const std::string &addon);

	std::string m_message;
	STRING_ID m_error;
	std::string m_file; //file where exception occurred = __FILE__ For example Problem.cpp
	int m_line; //line of file where exception occurred = __LINE__
	std::string m_function;
	std::string m_parseFile; //file which was parsed example For example "Darwen4.bts"
	std::string m_content; //parsing content from  m_parseFile
	std::string m_addon; //parsing content from  m_parseFile

	std::string getPlaceInfo() const;
	std::string getErrorString() const;

	void setParseFile(const std::string &s) {
		m_parseFile = s;
	}
	virtual ~ParseException();

};

typedef std::vector<ParseException> VParseException;
typedef VParseException::iterator VParseExceptionI;

#endif /* PROBLEM_PARSEEXCEPTION_H_ */
