	int i2, v2, t2, r2, rc2;
	USC sc2;
#ifdef PREFERANS_NODE_COUNT
	m_nodes++;
#endif

#ifdef CUT3LASTLAYERS
	if (m_depth == 2) {
		int j2, k2;
		bool l2;
		for (i2 = 0; i2 < c2.length; i2 ++) {
			sc2=c2[i2];
			r2  = sc2.c;

	#define r0 sc0.c
			ADJUST_RANK(2,0)
			ADJUST_RANK(2,1)
	#undef r0

			REMOVE_CARD(2)

#define t t2
			SETT;
#undef t
			l2 = (t2 == 0 && w[1] == 0) || (t2 == 1 && w[0] == 0) || t2 == 2;

#ifdef PREFERANS_NODE_COUNT
			m_nodes++;
#endif
			{//inner block for other variables sc0,sc1,sc2

		  const int*v=w+t2;

			USC sc0,sc1,sc2;
			for (v2 = 0; v2 < 4; v2++) {
				for (j2 = 0, k2 = m_code[v2]; k2 != 3; k2 >>= 2, j2++) {
					t2 = k2 & 3;
					if (t2 == v[0]) {
						sc0.set(j2,v2);
					}
					else if (t2 == v[1]) {
						sc1.set(j2,v2);
					}
					else {
						sc2.set(j2,v2);
					}
				}
			}

#define t t2
			SETT;
#undef t

			if (t2 == 0 || (t2 == 1 && v[2] == 0) || (t2 == 2 && v[1] == 0)) {
				v2 = l2 ?
#ifdef MISERE
									-2:2
#else
									2:-2
#endif
									;
			}
			else {
				v2 = 0;
			}

			}//end of inner block

			RESTORE_CARD(2)
			if (v2 > a2) {
#ifdef STOREBEST
				m_best = sc2.toIndex();
#endif
				if ((a2 = v2) >= b2) {
					break;
				}
			}
		}
	} //m_depth==2
	else {
#endif //CUT3LASTLAYERS

		for (i2 = 0; i2 < c2.length; i2 ++) {
			sc2=c2[i2];
			r2  = sc2.c;

#define r0 sc0.c
		ADJUST_RANK(2,0)
		ADJUST_RANK(2,1)
#undef r0

			REMOVE_CARD(2)

#define t t2
			SETT;
#undef t

			m_depth--;
			if ((t2 == 0 && w[1] == 0) || (t2 == 1 && w[0] == 0) || t2 == 2) {
#ifdef MISERE
				v2 = -1;
				v2 += eMisere(w+t2, a2 - v2, b2 - v2);
#elif defined(NT)
				v2 = 1;
				v2 += eNT(w+t2, a2 - v2, b2 - v2);
#else
				v2 = 1;
				v2 += e(w+t2, a2 - v2, b2 - v2);
#endif

			}
			else {
#ifdef MISERE
				v2 = 1;
				v2 -= eMisere(w+t2, -b2 + v2, -a2 + v2);
#elif defined(NT)
				v2 = -1;
				v2 -= eNT(w+t2, -b2 + v2, -a2 + v2);
#else
				v2 = -1;
				v2 -= e(w+t2, -b2 + v2, -a2 + v2);
#endif

			}
			m_depth++;

			RESTORE_CARD(2)

			if (v2 > a2) {
#ifdef STOREBEST
				m_best = sc2.toIndex();
#endif
				if ((a2 = v2) >= b2) {
					break;
				}
			}
		}
#ifdef CUT3LASTLAYERS
	}
#endif //CUT3LASTLAYERS

