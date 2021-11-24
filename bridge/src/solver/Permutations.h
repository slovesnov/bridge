/*
 * Permutations.h
 *
 *  Created on: 19.11.2013
 *      Author: alexey slovesnov
 *
 *  19dec2018 modified added for each range support
 */

#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

#include <vector>

/*
 * PermutationType
 * PERMUTATIONS_WITHOUT_REPLACEMENTS k-permutations of n without replacements. Number of combinations equals n!/(n-k)!
 * PERMUTATIONS_WITH_REPLACEMENTS k-permutations of n with replacements. Number of combinations equals n^k
 * COMBINATION k-combination of n items. Number of combinations equals n!/(n-k)!/k!
 */
enum PermutationType {
	PERMUTATIONS_WITHOUT_REPLACEMENTS, PERMUTATIONS_WITH_REPLACEMENTS, COMBINATION
};

typedef std::vector<int> VInt;

class Permutations {
	class V: public VInt {
	private:
		int k;
	public:

		void resize(int n, int _k) {
			k = _k;
			VInt::resize(n);
		}

		const int*begin() const {
			return data();
		}

		const int*end() const {
			return data() + k;
		}

		void operator=(VInt const& v){
			VInt::operator=(v);
		}

	};

	int k, n, index;
	PermutationType type;
	V i, a;

	//add index
	void add();

public:
	Permutations() {
		init(1, 1, COMBINATION);
	}
	Permutations(const int k, const int n, const PermutationType type) {
		init(k, n, type);
	}
	void init(const int k, const int n, const PermutationType type);
	bool next();
	void reset();

	inline const int getK() const {
		return k;
	}

	inline const int getN() const {
		return n;
	}

	inline V const& getIndexes() const {
		return type == PERMUTATIONS_WITHOUT_REPLACEMENTS ? a : i;
	}

	inline const int getIndex(const int i) const {
		return getIndexes()[i];
	}

	//return number of combinations
	const int number() const;

	void operator++() {
		next();
	}

	bool operator!=(const Permutations&) const {
		return index != -1;
	}

	V const& operator*() const {
		return getIndexes();
	}

	Permutations& begin() {
		reset();
		return *this;
	}

	Permutations& end() {
		return *this;
	}

	struct State{
		int index;
		VInt i,a;
		int parameter;
	};

	void saveState(State& state)const;
	void loadState(State const& state);

};

#endif /* PERMUTATIONS_H_ */
