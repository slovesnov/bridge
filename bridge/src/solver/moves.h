#if defined(PREFERANS_H_) ^ defined(BRIDGE_H_)==0
#error "only exactly one of PREFERANS_H_ or BRIDGE_H_ should be defined"
#endif

int i;

#ifdef BRIDGE_H_
	#if BRIDGE_MAX_PRECOUNT_SUIT_CARDS==11
	int j;
	#endif
	int k,l,m,n;
	int8_t*z;
#endif

#define O ( ORDER % MOVES_ONE_SUIT_OPTIONS )
#include "moves_one_suit.h"
#undef O

if (c.length != 0) {
	return;
}

#ifdef PREFERANS_TRUMP_GAME
	#define O ( ORDER / MOVES_ONE_SUIT_OPTIONS) % MOVES_ONE_SUIT_OPTIONS
	#define suit m_trump
	#define NO_DECLARATION
	#include "moves_one_suit.h"
	#undef NO_DECLARATION
	#undef suit
	#undef O

		if (c.length != 0) {
			return;
		}
#endif

#ifdef PREFERANS_TRUMP_GAME
	#define O	( ORDER / MOVES_ONE_SUIT_OPTIONS / MOVES_ONE_SUIT_OPTIONS)
#else
#define O	( ORDER / MOVES_ONE_SUIT_OPTIONS )
#endif

#ifdef PREFERANS_TRUMP_GAME
	#define SKIP_SUITS 2
#else
#define SKIP_SUITS 1
#endif
#include "moves_many_suits.h"
#undef SKIP_SUITS
#undef O
