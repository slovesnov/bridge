/*
 * Permutations.cpp
 *
 *  Created on: 19.11.2013
 *      Author: alexey slovesnov
 */

#include <cassert>
#include "Permutations.h"

const int Permutations::number() const {
	int i;
	int r = 1;
	if (type == PERMUTATIONS_WITH_REPLACEMENTS) {
		for (i = 0; i < k; i++) {
			r *= n;
		}
	}
	else if (type == COMBINATION) {
		/* for big n,k
		 * C(n,k)=n*C(n-1,k-1)/k
		 * C(n,k)=n*C(n-1,k-1)/k=(n/k)*(n-1/k-1)...(n-k+1/1)C(n-k,0); C(n-k,0)=1
		 */
		for (i = 1; i <= k; i++) {
			r *= n - k + i;
			r /= i;
		}
	}
	else {
		for (i = n - k + 1; i <= n; i++) {
			r *= i;
		}
	}
	return r;
}

void Permutations::init(const int _k, const int _n, PermutationType _type) {
	n = _n;
	k = _k;
	type = _type;

	assert(k>=0 && n>=0 && "permutations. error n and k should nonnegative");
#ifndef NDEBUG
	if(type!=PERMUTATIONS_WITH_REPLACEMENTS) {
		assert(n>=k && "permutations. error n<k");
	}
#endif
	i.resize(k, k);
	if (type == PERMUTATIONS_WITHOUT_REPLACEMENTS) {
		a.resize(n, k);
	}
	reset();
}

void Permutations::add() {
	assert(k>0);
	int j = index;
	int m, l;
	i[j]++;
	for (j++; j < k; j++) {
		i[j] = type == COMBINATION ? i[j - 1] + 1 : 0;
	}

	if (type == PERMUTATIONS_WITHOUT_REPLACEMENTS) {
		for (j = 0; j < n; j++) {
			a[j] = j;
		}
		for (j = 0; j < k; j++) {
			m = j + i[j];
			l = a[m];
			for (; j != k - 1 && m > j; m--) {
				a[m] = a[m - 1];
			}
			a[j] = l;
		}
	}

}

bool Permutations::next() {
	if(k==0){
		index=-1;
		return false;
	}

	int l, j;
	for (j = k - 1; j >= 0; j--) {
		if (type == PERMUTATIONS_WITHOUT_REPLACEMENTS) {
			l = j;
		}
		else if (type == PERMUTATIONS_WITH_REPLACEMENTS) {
			l = 0;
		}
		else {
			l = k - 1 - j;
		}
		if (n - 1 != i[j] + l) {
			break;
		}
	}
	index = j;

	if (j == -1) {
		return false;
	}
	add();
	return true;
}

void Permutations::reset() {
	if(k>0){
		index = 0;
		i[0] = -1;
		add();
	}
}

//Note. Tested only for type=COMBINATION
void Permutations::saveState(State& state) const {
	state.index = index;
	state.i = i;
	if (type == PERMUTATIONS_WITHOUT_REPLACEMENTS) {
		state.a=a;
	}
}

//Note. Tested only for type=COMBINATION
void Permutations::loadState(const State& state) {
	index = state.index;
	i=state.i;
	if (type == PERMUTATIONS_WITHOUT_REPLACEMENTS) {
		a=state.a;
	}
}
