/*
 * SC.cpp
 *
 *       Created on: 16.11.2020
 *           Author: aleksey slovesnov
 * Copyright(c/c++): 2020-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         Homepage: slovesnov.users.sourceforge.net
 */

#include "SC.h"

#ifndef FINAL_RELEASE
#include <algorithm>
#include "BridgeCommon.h"
#endif

std::string USC::toString()const{
	if(c<0 || s<0 || c>12 || s>3){
		return format("c=%d s=%d",c,s);
	}
	std::string q;
	q+=RANK[c];
	return q+SUITS_CHAR[s];
}

void USC::print()const{
	printl(toString());
}

SC::SC(SC const& o,bool order){
	copy(o);
	if (order) {
		std::sort(a, a + length);
	}
}

void SC::print(const char*p)const {
	std::string s;
	if(p){
		s+=p;
	}
	for(int i=0;i<length;i++) {
		s+=RANK[a[i].c];
		s+=SUITS_CHAR[a[i].s];
		//s+=format("/r%d",a[i].c);
		s+=' ';
	}
	s+=format("[%d]",length);
	printl(s);
}

void SC::printOrdered(const char*p)const{
	SC a(*this,true);
	a.print(p);
}

bool SC::operator==(SC const& o)const{
	if(length!=o.length){
		return false;
	}
	for(int i=0;i<length;i++){
		if(a[i]!=o.a[i]){
			return false;
		}
	}
	return true;
}


bool SC::equalsSequence(SC const& o,const int code[4])const{
	if(length!=o.length){
		return false;
	}
	int j,v,c;
	//if cards not equal they should be in one sequence
	for(int i=0;i<length;i++){
		auto p=a[i];
		auto q=o.a[i];
		if(p.s!=q.s){
			return false;
		}

		int min=std::min(p.c,q.c);
		int max=std::max(p.c,q.c);
		c=code[p.s]>>(4+2*min);
		v=c&3;
		for(j=min;j<max;j++,c>>=2){
			if( (c&3)!=v ){
				return false;
			}
		}
	}
	return true;

}

bool SC::equalsOrder(SC const& o)const{
	if(length!=o.length){
		return false;
	}

	SC a(*this,true);
	SC b(o,true);
	return a==b;
}
