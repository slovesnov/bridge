/*
 * bi3.h
 *
 *       Created on: 14.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

	int i3, v3, t3, r3, rc3;
	USC sc3;

	#ifdef BRIDGE_NODE_COUNT
	m_nodes++;
	#endif


#ifdef BRIDGE_ENDGAME
//	if(m_depth<endgameN+1){
//		printi
//		exit(1);
//	}

	if(m_depth==endgameN+1){
	for (i3 = 0; i3 < c3.length; i3++) {//like code below except mask
//		if(!mask3[i3]){
//			continue;
//		}
		sc3=c3[i3];
		r3  = sc3.c;

#define r0 sc0.c
		ADJUST_RANK(3,0)
		ADJUST_RANK(3,1)
		ADJUST_RANK(3,2)
#undef r0

		REMOVE_CARD(3)

#define t t3
		SETT;
#undef t

		/* CHECK = 0 no check only take estimate from endgame
		 * CHECK = 1 check v31(from database)==v3(by count)
		 */
#define CHECK 0


#if CHECK!=0
		m_depth--;
#endif


#ifdef NO_TRUMP
				const int in=0;
#else
				const int in=1;
#endif
				int l[4];
				int i,j,k;

				for(i=0;i<4;i++){
					l[i]=m_code[i];
				}
				std::sort(l+in,l+4,[](auto&a,auto&b){
					return (a&15)<(b&15);
				});

				k=0;
				j=0;
				for(i=0;i<4;i++){
					k|=(l[i]>>4)<<j;
					l[i]&=15;
					j+=2*l[i];
				}
				//remove two high bits
				k &= (1<<(4*endgameN*2-2))-1;
				j = l[0] + endgameMultiplier * (l[1] + endgameMultiplier * l[2]);
				assert(endgameIndex[w[t3]][k]!=-1);
				assert(endgameLength[in][j]!=-1);
				i=endgameLength[in][j]*endgameCN + endgameIndex[w[t3]][k];
				assert(i<endgameEstimateLength[in]*4);//mul 4 because length in bytes, 1 byte=4 chains
#if CHECK!=0
				int v31;
				int alpha;
#endif
				j=-endgameN+2*((endgameEstimate[in][i/4]>>((i%4)*2))&3);
				if ( t3%2==1) {
#if CHECK==0
					v3=1+j;
#else
					v31 = 1;//b alpha+1, else alpha+3
					alpha=a3 - v31;
					j=j<=alpha?alpha:alpha+2;
					v31+=j;
#endif
				}
				else{
#if CHECK==0
					v3=-1-j;
#else
					v31 = -1;//b -alpha-1, else -alpha-3
					alpha=a2+ v31;
					j=j<=alpha?alpha:alpha+2;
					v31-=j;
#endif
				}

#if CHECK!=0
		if ( t3%2==1) {
			v3 = 1;
#ifdef NO_TRUMP
			v3 += eNT(w+t3, a3 - v3);
#else
			v3 += e(w+t3, a3 - v3);
#endif
		}
		else {
			v3 = -1;
#ifdef NO_TRUMP
			v3 -= eNT(w+t3, a2+ v3);
#else
			v3 -= e(w+t3, a2+ v3);
#endif
		}
#endif //CHECK!=0

#if CHECK!=0
		m_depth++;
#endif

		RESTORE_CARD(3);

#if CHECK!=0
		//		if ( (t3%2==1 && ((v3<=1+alpha && v31<=1+alpha) || (v3>=3+alpha && v31>=3+alpha)))
		//				|| (t3%2==0 && ((v3>=-1-alpha && v31>=-1-alpha) || (v3<=-3-alpha && v31<=-3-alpha)))
		//				) {
		if(v31==v3){
//			static int counter=0;
//			if(++counter %1'000'000==0){
//				printl("ok",counter/1'000'000,"m")
//				fflush(stdout);
//			}
		}
		else{
			printl("er",v3,v31);
			exit(1);
		}
#endif

		if (v3 > a3) {
//			printi
//			exit(1);

	#ifdef STOREBEST
			SET_BEST(3)
	#endif
			a3 = v3;
			break;
		}
	}}
	else
#undef CHECK
#endif//BRIDGE_ENDGAME

#ifdef CUT4LASTLAYERS
	if (m_depth == 2) {
		for (i3 = 0; i3 < c3.length; i3 ++) {
			sc3=c3[i3];
			r3  = sc3.c;

#define r0 sc0.c
			ADJUST_RANK(3,0)
			ADJUST_RANK(3,1)
			ADJUST_RANK(3,2)
	#undef r0

			REMOVE_CARD(3)

#define t t3
			SETT;
#undef t
			{//inner block for sc0,sc1,sc2
			int i, j, c, t, l;
		  USC sc0,sc1,sc2,sc3;
		  const int*v=w+t3;
			for (i = 0; i < 4; i++) {
				l=m_code[i]&15;
				for (j = 0, c = m_code[i]>>4; j<l; c >>= 2, j++) {
					t = c & 3;
					if (t == v[0]) {
						sc0.set(j,i);
					}
					else if (t == v[1]) {
						sc1.set(j,i);
					}
					else if (t == v[2]) {
						sc2.set(j,i);
					}
					else {
						sc3.set(j,i);
					}
				}
			}

			SETT
			v3= t%2==1?0 : (t3%2==1?2:-2);
			}//end of inner block

			RESTORE_CARD(3);

			if (v3 > a3) {

		#ifdef STOREBEST
				SET_BEST(3)
		#endif
				a3 = v3;
				break;

			}
		}

	} //m_depth==2
	else{
#endif

		bool mask3[13];
		bool further_count3=true;

		for (i3 = 0; i3 < c3.length; i3++) {
			mask3[i3]=1;
			sc3=c3[i3];
			r3  = sc3.c;

	#define r0 sc0.c
			ADJUST_RANK(3,0)
			ADJUST_RANK(3,1)
			ADJUST_RANK(3,2)
	#undef r0

			REMOVE_CARD(3)

	#define t t3
			SETT;
	#undef t

			m_depth--;

			if ( t3%2==1) {
				v3 = 1;
				v3 += ep(w+t3, a3 - v3);
			}
			else {
				v3 = -1;
				v3 -= ep(w+t3, a2+ v3);
			}
			m_depth++;

			RESTORE_CARD(3);
			if(v3!=101 && v3!=-101) {
				if (v3 > a3) {
					further_count3=false;
			#ifdef STOREBEST
					SET_BEST(3)
			#endif
					a3 = v3;
					break;
				}
				else{
					mask3[i3]=0;
				}
			}
		}



if(further_count3){
//#ifdef BRIDGE_ENDGAME
//	//TODO
//	if(m_depth==endgameN+1){
//		printi
//		exit(1);
//	}
//#endif
	for (i3 = 0; i3 < c3.length; i3++) {
		if(!mask3[i3]){
			continue;
		}
		sc3=c3[i3];
		r3  = sc3.c;

#define r0 sc0.c
		ADJUST_RANK(3,0)
		ADJUST_RANK(3,1)
		ADJUST_RANK(3,2)
#undef r0

		REMOVE_CARD(3)

#define t t3
		SETT;
#undef t

		m_depth--;
		if ( t3%2==1) {
			v3 = 1;
#ifdef NO_TRUMP
			v3 += eNT(w+t3, a3 - v3);
#else
			v3 += e(w+t3, a3 - v3);
#endif
		}
		else {
			v3 = -1;
#ifdef NO_TRUMP
			v3 -= eNT(w+t3, a2+ v3);
#else
			v3 -= e(w+t3, a2+ v3);
#endif
		}
		m_depth++;

		RESTORE_CARD(3);

		if (v3 > a3) {

	#ifdef STOREBEST
			SET_BEST(3)
	#endif
			a3 = v3;
			break;
		}
	}
}//further_count3
#ifdef CUT4LASTLAYERS
	}
#endif //CUT4LASTLAYERS

