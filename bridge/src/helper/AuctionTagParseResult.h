/*
 * AuctionTagParseResult.h
 *
 *       Created on: 18.02.2017
 *           Author: alexey slovesnov
 * copyright(c/c++): 2017-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef PROBLEM_AUCTIONTAGPARSERESULT_H_
#define PROBLEM_AUCTIONTAGPARSERESULT_H_

#include <string>

struct AuctionTagParseResult {
	STRING_ID id;
	std::string s;
	char declarer;

	AuctionTagParseResult(STRING_ID _id, std::string _s = "", char _declarer =
			' ') {
		id = _id;
		s = _s;
		declarer = _declarer;
	}

};

#endif /* PROBLEM_AUCTIONTAGPARSERESULT_H_ */
