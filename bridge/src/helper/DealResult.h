/*
 * DealResult.h
 *
 *  Created on: 23.11.2021
 *      Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#ifndef HELPER_DEALRESULT_H_
#define HELPER_DEALRESULT_H_

#include <vector>

class DealResult {
public:
	char a[2][17];//max 13cards +3points + null character = 17 chars
	char result;
	bool operator<(const DealResult& dr)const;
};

using VDealResult = std::vector<DealResult>;

#endif /* HELPER_DEALRESULT_H_ */
