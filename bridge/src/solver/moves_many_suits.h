/*
 * moves_namy_suits.h
 *
 *       Created on: 06.02.2021
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2021-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

/* O should be defined and SKIP_SUITS one of the values [0,1,2]
 total MOVES_MANY_SUITS_OPTIONS_NT options

 0 high0->low0, high1->low1...
 1 low0, low1..., high0->low0, high1->low1...
 2 high0, high1..., high0->low0, high1->low1...
 3 low0, high0, low1, high1..., high0->low0, high1->low1...
 4 high0, low0, high1, low1..., high0->low0, high1->low1...

 5 low0->high0, low1->high1...
 6 low0, low1..., low0->high0, low1->high1...
 7 high0, high1..., low0->high0, low1->high1...
 8 low0, high0, low1, high1..., low0->high0, low1->high1...
 9 high0, low0, high1, low1..., low0->high0, low1->high1...

*/
	int8_t*aa[4],*bb[4];

	const int OM=MOVES_MANY_SUITS_OPTIONS_NT/2;
	const int O1=O%OM;

	for (i = 0; i < 4; i++)
#if SKIP_SUITS==1
	if (i != suit)
#elif SKIP_SUITS==2
	if (i != suit && i!=m_trump)
#endif
	{
		MOVES_INIT(i,w,aa[i],bb[i])

		if(O1!=0){
			if( O1%2) {
				MOVES_LOW(i,w,c,aa[i],bb[i])
			}
			else {
				MOVES_HIGH(i,w,c,aa[i],bb[i])
			}

			if( O1==3) {
				MOVES_HIGH(i,w,c,aa[i],bb[i])
			}
			else if( O1==4) {
				MOVES_LOW(i,w,c,aa[i],bb[i])
			}
		}
	}

		for (i = 0; i < 4; i++)
#if SKIP_SUITS==1
			if (i != suit)
#elif SKIP_SUITS==2
			if (i != suit && i!=m_trump)
#endif
		{

			if( O < OM ) {
				MOVES_HIGH_LOW_OTHERS(i,w,c,aa[i],bb[i])
			}
			else {
				MOVES_LOW_HIGH_OTHERS(i,w,c,aa[i],bb[i])
			}
		}

