int i1, r1, rc1, a2, b2;
USC sc1;
#ifdef PREFERANS_NODE_COUNT
	m_nodes++;
#endif

for (i1 = 0; i1 < c1.length; i1 ++) {
	sc1=c1[i1];
	r1 = sc1.c;

#define r0 sc0.c
	ADJUST_RANK(1,0)
#undef r0

	REMOVE_CARD(1)

	if (w[0] == 0) {
		a2 = a1;
		b2 = b1;
#ifdef STOREBEST
#undef STOREBEST
#include "pi2.h"
#define STOREBEST
#else
#include "pi2.h"
#endif
	}
	else {
		a2 = -b1;
		b2 = -a1;
#ifdef STOREBEST
#undef STOREBEST
#include "pi2.h"
#define STOREBEST
#else
#include "pi2.h"
#endif
		a2 = -a2;
	}

	RESTORE_CARD(1);
	if (a2 > a1) {
#ifdef STOREBEST
			m_best = sc1.toIndex();
#endif

		if ((a1 = a2) >= b1) {
			break;
		}
	}
}

