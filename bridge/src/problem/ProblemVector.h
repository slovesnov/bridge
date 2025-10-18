/*
 * ProblemVector.h
 *
 *       Created on: 20.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.rf.gd
 */

#ifndef PROBLEMVECTOR_H_
#define PROBLEMVECTOR_H_

#include "Problem.h"

//vector of problems
class ProblemVector {
	//this class cann't have it's own filepath, because it can consist several files
	void openPbn(const std::string &filepath);
	void openDf(const std::string &filepath);
	void openBts(const std::string &filepath, FILE_TYPE type);

	void splitAndParse(FILE *f, FILE_TYPE type);

	//no messages
	void add(const std::string &filepath);

	static std::string getFileFormat(int size);
	static std::string fileName(const std::string &filepath,
			const std::string &buffer, int i);

public:
	VProblem m_problems;
	VParseException m_errors;

	ProblemVector() {
	}
	ProblemVector(const std::string &filepath) {
		set(filepath, true);
	}

	VProblemI begin() {
		return m_problems.begin();
	}

	VProblemI end() {
		return m_problems.end();
	}

	VProblemCI begin() const {
		return m_problems.begin();
	}

	VProblemCI end() const {
		return m_problems.end();
	}

	void insert(int i, VProblem const &vp) {
		m_problems.insert(begin() + i, vp.begin(), vp.end());
	}

	inline int size() const {
		return m_problems.size();
	}

	void push_back(const Problem &p) {
		m_problems.push_back(p);
	}

	Problem& operator[](int i) {
		assert(i >= 0 && i < size());
		return m_problems[i];
	}

	const Problem& operator[](int i) const {
		assert(i >= 0 && i < size());
		return m_problems[i];
	}

	inline int esize() const {
		return m_errors.size();
	}

	inline void clear() {
		m_problems.clear();
		m_errors.clear();
	}

	void set(const VProblem &v) {
		m_problems = v;
		m_errors.clear();
	}

	//add should show errors one time only, so add(vector) and it shows all error one time
	bool set(const std::string &filepath, bool add) {
		VString v;
		v.push_back(filepath);
		return set(v, add);
	}

	//returns true if at least one problem recognized without errors
	bool set(const VString &v, bool append);

	static const int SAVE_OK = 0;
	static const int SAVE_WARNING = 1;
	static const int SAVE_ERROR = 2;
	int save(std::string filepath, bool split); //filepath utf8

	static void addSave(const VString &v, const std::string &filepath,
			bool split); //filepath utf8

	static std::string unzipFile(std::string filepath); //filepath utf8

	bool onlyPreferansProblems() const;

	void swap(int i1, int i2);

	void removeAllPreferansProblems();

};

#endif /* PROBLEMVECTOR_H_ */
