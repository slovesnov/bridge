	int i2, v2, t2, r2, rc2;
	USC sc2;
#ifdef PREFERANS_NODE_COUNT
	m_nodes++;
#endif

//#ifdef PREFERANS_ENDGAME
	if(m_depth==endgameN+1){
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

		/* CHECK = 0 no check only take estimate from endgame
		 * CHECK = 1 check v21(from database)==v2(by count)
		 */
#define CHECK 1

#if CHECK!=0
		m_depth--;
#endif



#ifdef MISERE
		const int in=2;
#elif defined(NO_TRUMP)
		const int in=0;
#else
		const int in=1;
#endif

		int l[4],sl[4];
		int i,j,k;

		for(i=0;i<4;i++){
			l[i]=m_code[i];
		}
		std::sort(l+(in==0?0:1),l+4,[](auto&a,auto&b){
			return a<b;
		});

		for(i=0;i<4;i++){
			sl[i]=endgameSuitLength[l[i]];
			assert(sl[i]!=-1);
		}


//		for(i=0;i<4;i++){
//			printl(binaryCodeString(m_code[i]))
//		}

//		for(i=0;i<4;i++){
//			printl(binaryCodeString(l[i]),sl[i])
//		}

		k=0;
		j=0;
		for(i=0;i<4;i++){
			k|=l[i]<<j;
			k^=3<<(j+2*sl[i]);//remove two high bits
			j+=2*sl[i];
		}
//		printl(binaryCodeString(k),"in=",in)
//		if(sl[0]!=0 &&sl[1]!=0 &&sl[2]!=0 &&sl[3]!=0){
//			exit(1);
//		}

		//remove two high bits
		k &= (1<<(3*endgameN*2-2))-1;
//		printl(binaryCodeString(k))

		j = sl[0] + endgameMultiplier * (sl[1] + endgameMultiplier * sl[2]);
//		if(endgameIndex[w[t2]][k]==-1){
//			printl(int(w[t2]),binaryCodeString(k),k)
//		}
		assert(endgameIndex[w[t2]][k]!=-1);
//		if(endgameLength[in][j]==-1){
//			printl(j,JOINS(l,','),endgameMultiplier)
//		}
		assert(endgameLength[in][j]!=-1);
		i=endgameLength[in][j]*endgameCN + endgameIndex[w[t2]][k];
//		assert(i<endgameEstimateLength[in]*4);//mul 4 because length in bytes, 1 byte=4 chains
		assert(i<endgameEstimateLength[in]*2);//mul 2 because length in bytes, 1 byte=2 chains
#if CHECK!=0
				int v21=1234567;
				int alpha,beta;
#endif
				//TODO
				j=-endgameN+2*((endgameEstimate[in][i/2]>>((i%2)*2))&15);
				//j=-endgameN+2*((endgameEstimate[in][i/4]>>((i%4)*2))&3);

#define v2 v21
				if ((t2 == 0 && w[1] == 0) || (t2 == 1 && w[0] == 0) || t2 == 2) {

	#ifdef MISERE
					v2 = -1;
					alpha=a2 - v2;
					beta=b2 - v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 += j;//eMisere(w+t2, a2 - v2, b2 - v2);
	#elif defined(NO_TRUMP)
					v2 = 1;
					alpha=a2 - v2;
					beta=b2 - v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 += j;//eNT(w+t2, a2 - v2, b2 - v2);
	#else
					v2 = 1;
					alpha=a2 - v2;
					beta=b2 - v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 += j;//e(w+t2, a2 - v2, b2 - v2);
	#endif
				}
				else {
	#ifdef MISERE
					v2 = 1;
					alpha=-b2 + v2;
					beta=-a2 + v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 -= j;//eMisere(w+t2, -b2 + v2, -a2 + v2);
	#elif defined(NO_TRUMP)
					v2 = -1;
					alpha=-b2 + v2;
					beta=-a2 + v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 -= j;//eNT(w+t2, -b2 + v2, -a2 + v2);
	#else
					v2 = -1;
					alpha=-b2 + v2;
					beta=-a2 + v2;
					j=j<=alpha?alpha:(j>=beta?beta:j);
					v2 -= j;//e(w+t2, -b2 + v2, -a2 + v2);
	#endif
				}
#undef v2

			if ((t2 == 0 && w[1] == 0) || (t2 == 1 && w[0] == 0) || t2 == 2) {
#ifdef MISERE
				v2 = -1;
				v2 += eMisere(w+t2, a2 - v2, b2 - v2);
#elif defined(NO_TRUMP)
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
#elif defined(NO_TRUMP)
				v2 = -1;
				v2 -= eNT(w+t2, -b2 + v2, -a2 + v2);
#else
				v2 = -1;
				v2 -= e(w+t2, -b2 + v2, -a2 + v2);
#endif
			}
#if CHECK!=0
			m_depth++;
#endif

			RESTORE_CARD(2)

#if CHECK!=0
		if(v21==v2){
			static int counter=0;
			if(++counter %1'000'000==0){
				printl("ok",counter/1'000'000,"m")
				fflush(stdout);
			}
		}
		else{
			printl("er",v2,v21);
			exit(1);
		}
#endif


			if (v2 > a2) {
#ifdef STOREBEST
				m_best = sc2.toIndex();
#endif
				if ((a2 = v2) >= b2) {
					break;
				}
			}
		}
	}
	else
#undef CHECK
//#endif//PREFERANS_ENDGAME

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
#elif defined(NO_TRUMP)
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
#elif defined(NO_TRUMP)
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

