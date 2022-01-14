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


#ifdef BRIDGE_ENDGAME

			if(m_depth==endgameN && w[t3]==0){
#ifdef NO_TRUMP
				const int in=0;
#else
				const int in=1;
#endif
				int l[4];
				int32_t code=0;
				j=0;
				for(i=0;i<4;i++){
					code|=(m_code[i]>>4)<<j;
					l[i]=m_code[i]&15;
					j+=2*l[i];
					printCode(i);
				}
				j = l[0] + 13 * (l[1] + 13 * l[2]);
				printl(endgameIndex[w[t3]][code],endgameLength[in][j],w[t3])
				assert(endgameIndex[w[t3]][code]!=-1);
				assert(endgameLength[in][j]!=-1);
				int k=endgameEstimateLength[in]*4;//mul 4 because length in bytes, 1 byte=4 chains
				int kk=endgameLength[in][j]*endgameCm(true) + endgameIndex[w[t3]][code];
				printl(kk,int(endgameEstimate[in][kk/4]),kk%4)
				assert(kk<k);
				int v31;
				v31=(endgameEstimate[in][kk/4]>>((kk%4)*2))&3;
				j=-endgameN+2*v31;
				printl("original=",v31,"e=",j,t3%2==1)
				if ( t3%2==1) {
					v31 = 1;
					int alpha=a3 - v31;
					j=j<=alpha? alpha:alpha+2;
					v31+=j;
				}
				else{
					v31 = -1;
					int alpha=a2+ v31;
					j=j<=alpha? alpha:alpha+2;
					v31-=j;
				}

				if ( t3%2==1) {
					v3 = 1;
		#ifdef NO_TRUMP
					int alpha=a3 - v3;
					j=eNT(w+t3, a3 - v3);
					v3 += j;
					printl("vbegin=1",v3,"e_got=",j,"alpha=",alpha,w[t3])
		#else
					v3 += e(w+t3, a3 - v3);
					printi
		#endif
				}
				else {
					v3 = -1;
		#ifdef NO_TRUMP
					int alpha=a2+ v3;
					j=eNT(w+t3, a2+ v3);
					v3 -= j;
					//v3 -= eNT(w+t3, a2+ v3);
					printl("vbegin=-1",v3,"e_got=",j,"alpha=",alpha,w[t3])
					for(j=0;j<4;j++){
						printCode(j);
					}
		#else
					v3 -= e(w+t3, a2+ v3);
					printi
		#endif
				}
				if(v3!=v31){
					printl("not equal v31=",v31,"v3=",v3)
					exit(1);
				}
				else{
					static int counter=0;
					counter++;
					printl("ok",counter)
					fflush(stdout);
				}
			}
#endif


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

