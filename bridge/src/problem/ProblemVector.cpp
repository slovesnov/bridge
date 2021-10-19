/*
 * ProblemVector.cpp
 *
 *       Created on: 20.08.2014
 *           Author: alexey slovesnov
 * copyright(c/c++): 2014-doomsday
 *           E-mail: slovesnov@yandex.ru
 *         homepage: slovesnov.users.sourceforge.net
 */

#include "ProblemVector.h"

#include <cerrno>
#include "../dialogs/MessageDialog.h"
#include "../Frame.h"
#include <mz_compat.h>

//this definition is differ with Problem.h
#define throwOnError(_Expression,error) if(!(_Expression))throw ParseException(#_Expression,error,__FILE__,__LINE__,__func__,"","");

//show all errors only one time
bool ProblemVector::set(const VString& v, bool append) {
	VStringCI itv;
	VParseExceptionCI it;
	VProblem p;

	if (!append) {
		p = m_problems;
		clear();
	}

	const unsigned psz = m_problems.size();
	const unsigned esz = m_errors.size();

	for (itv = v.begin(); itv != v.end(); itv++) {
		add(*itv);
	}

	//returns true if at least one problem recognized without errors
	bool r = m_problems.size() != psz;

	if (m_errors.size() == esz) { //no new errors
		if (m_problems.size() == psz) { //and no new problems
			if (psz == 0 && esz == 0) {
				//was set/added empty list no error
				if (!append) { //empty list was set. restore m_problems
					message(MESSAGE_ICON_ERROR, STRING_ERROR_EMPTY_FILE);
					m_problems = p;
				}
			}
			else {
				message(MESSAGE_ICON_ERROR, STRING_THIS_FILE_IS_DAMAGED);
			}
		}
	}
	else {
		if (m_problems.size() == psz) { //no new problems, errors only
			if (!append) { //restore m_problems
				m_problems = p;
			}
		}
		MessageDialog d(m_errors);
	}

	return r;

}

void ProblemVector::add(const std::string& filepath) {
	const unsigned psz = m_problems.size();
	const unsigned esz = m_errors.size();
	FILE_TYPE type = getFileType(filepath);
	try {
		if (type == FILE_TYPE_PBN) {
			openPbn(filepath);
		}
		else if (type == FILE_TYPE_BRIDGE || type == FILE_TYPE_PREFERANS) {
			openBts(filepath, type);
		}
		else if (type == FILE_TYPE_DF) {
			openDf(filepath);
		}
		else {
			/* this function can be called upon start so prevents assert
			 * For example "bridge.exe 1.xls" 1.xls - invalid file type
			 */
			throwOnError(0, STRING_ERROR_INVALID_FILE_TYPE)
		}
	}
	catch (ParseException& e) {
		m_errors.push_back(e);
	}

	//setup m_filename for all problems
	VProblemI it;
	for (it = m_problems.begin() + psz; it != m_problems.end(); it++) {
		it->m_filepath = filepath;
	}

	VParseExceptionI it1;
	for (it1 = m_errors.begin() + esz; it1 != m_errors.end(); it1++) {
		it1->setParseFile(filepath);
	}
}

void ProblemVector::openDf(const std::string& filepath) {
	FILE*f = open(filepath.c_str(), "r");
	throwOnError(f, STRING_ERROR_COULD_NOT_OPEN_FILE);
	splitAndParse(f, FILE_TYPE_DF);
}

void ProblemVector::openPbn(const std::string& filepath) {
	std::string s, q;
	const std::string fileName = getFileInfo(filepath, FILEINFO::SHORT_NAME);
	std::vector<int>::const_iterator vi;

	FILE*f = open(filepath.c_str(), "r");
	throwOnError(f, STRING_ERROR_COULD_NOT_OPEN_FILE);

	const char MARKER[] = "\n\n";
	const int MARKER_SIZE = strlen(MARKER);

	const char*pb, *pe;
	int i, length;
	VStringI it;
	VString vs;

	/* pbn file format site
	 * http://www.tistis.nl/pbn/
	 * empty line is problem separator
	 */

	fseek(f, 0, SEEK_END);
	i = ftell(f);
	char* content = new char[i + MARKER_SIZE + 1];
	fseek(f, 0, SEEK_SET);
	length = fread(content, 1, i, f); //note fread() return number of readed bytes because "\r\n"->"\n" length<i
	fclose(f);
	strcpy(content + length, MARKER); //append end of problem marker to the end

	if (content[0] == '[') {
		pb = content - 1; //in do{}while pb++
	}
	else {
		pb = strstr(content, "\n["); //Note "\n[" instead of strchr(,'[') because '[' could appear in comment
		if (pb == NULL) {
			m_errors.push_back(
					ParseException("", STRING_ERROR_EMPTY_FILE, __FILE__, __LINE__,
							__func__, "", ""));
			return;
		}
	}

	do {
		pb++;
		pe = strstr(pb + 1, MARKER);
		assert(pe!=NULL);
		vs.push_back(std::string(pb, pe - pb));
	} while ((pb = strstr(pe, "\n[")) != NULL); //Note "\n[" instead of strchr(,'[') because '[' could appear in comment

	delete[] content;

	if (vs.size() == 0) {
		return;
	}

	for (it = vs.begin(); it != vs.end(); it++) {
		Problem problem;
		try {
			problem.parse(FILE_TYPE_PBN, *it); //can be exception here
			m_problems.push_back(problem);
		}
		catch (ParseException& e) {
			m_errors.push_back(e);
		}
	}

}

void ProblemVector::openBts(const std::string& filepath, FILE_TYPE type) {
	const char PK[] = "PK";
	const int BTS_SIGNATURE_LENGTH = strlen(BTS_SIGNATURE);

	FILE*f;
	int i;
	char buffer[BTS_SIGNATURE_LENGTH];
	std::string content;
	char*p;
	/* READ BINARY because sometimes i=fread(buffer,1,BTS_SIGNATURE_LENGTH,f)<BTS_SIGNATURE_LENGTH
	 * if file open as "r" not binary. fread stops on 0x1a byte interpret as end of file
	 * so open "rb"
	 */
	f = open(filepath.c_str(), "rb");
	throwOnError(f, STRING_ERROR_COULD_NOT_OPEN_FILE);

	i = fread(buffer, 1, BTS_SIGNATURE_LENGTH, f);
	if (i != BTS_SIGNATURE_LENGTH) {
		fclose(f);
		throwOnError(0, i == 0 ? STRING_ERROR_EMPTY_FILE : STRING_UNKNOWN_ERROR);
	}

	//also recognize without BTS_SIGNATURE at the beginning of file
	if (startsWith(buffer, BTS_SIGNATURE) || startsWith(buffer, BTS_BEGIN_MARKER)) { //new format
	//should reopen in NOT binary mode
		fclose(f);
		f = open(filepath.c_str(), "r");
		splitAndParse(f, FILE_TYPE_BRIDGE);
	}
	else {
		if (startsWith(buffer, PK)) {		//packed
			fclose(f);
			content = unzipFile(filepath);		//external functions so use locale
			throwOnError(content.length() != 0, STRING_UNKNOWN_ERROR);
		}
		else {
			fseek(f, 0L, SEEK_END);
			i = ftell(f);
			p = new char[i];
			fseek(f, 0L, SEEK_SET);
			if (int(fread(p, 1, i, f)) != i) {
				delete[] p;
				throwOnError(0, STRING_UNKNOWN_ERROR);
			}
			fclose(f);
			content.append(p, i);		//use append cause '\0' should happens
			delete[] p;
		}

		Problem problem;
		try {
			problem.parse(type, content, true,
					type == FILE_TYPE_BRIDGE ? BRIDGE : PREFERANS);	//can be exception here
			m_problems.push_back(problem);
		}
		catch (ParseException& e) {
			m_errors.push_back(e);
		}
	}
}

void ProblemVector::splitAndParse(FILE* f, FILE_TYPE type) {
	int i;
	const char*p, *e, *q;
	char* content;
	VString v;
	std::string s;
	std::string original;
	char marker[32];

	assert(type == FILE_TYPE_BRIDGE || type == FILE_TYPE_DF);

	//Note add "\n" better. For example word 'deal' which we search can appear in comment
	sprintf(marker, "\n%s",
			type == FILE_TYPE_BRIDGE ? BTS_BEGIN_MARKER : DF_BEGIN_MARKER_L);	//use lower case
	const int markerSize = strlen(marker);

	fseek(f, 0, SEEK_END);
	i = ftell(f);
	content = new char[i + markerSize + 2];
	fseek(f, 0, SEEK_SET);
	i = fread(content + 1, 1, i, f);//note fread() return number of read bytes because "\r\n"->"\n" length<i
	fclose(f);
	strcpy(content + i + 1, marker);		//append end of problem marker to the end
	content[0] = marker[0];	//add "\n" at the beginning to find whether file starts with "\nDeal"

	original = content;		//copy with additions to prevent mess up with indexes
	std::transform(content, content + strlen(content), content, ::tolower);

	p = strstr(content, marker);

	try {
		do {
			e = strstr(p + markerSize, marker);
			if (e == NULL) {
				break;
			}
			q = e;
			while (strchr("\n\r", *--q))
				;

			//pass original string
			s = std::string((p - content) + original.c_str() + 1, q - p);	//p starts from "\n"

			Problem problem;
			try {
				problem.parse(type, s);		//can be exception here
				m_problems.push_back(problem);
			}
			catch (ParseException& e) {
				m_errors.push_back(e);
			}

			p = e;
		} while (e != NULL);
	}
	catch (ParseException&) {
		delete[] content;
		throw;
	}
}

int ProblemVector::save(std::string filepath, bool split) {
	FILE*f = NULL;
	std::string s, b;
	FILE_TYPE t = getFileType(filepath);
	bool warning = false;

	if (split) {
		b = getFileFormat(size());
	}

	int i = 1;
	for (auto& pr: m_problems) {
		s = pr.getContent(t, split ? 1 : i,m_problems.size() );
		if (s.length() > 0) {
			if (t == FILE_TYPE_HTML) {
				s = Problem::postproceedHTML(s, gconfig->m_htmlStoreWithImages);
			}

			if (split) {
				f = open(fileName(filepath, b, i), "w+");
				if(!f){
					showError();
					return SAVE_ERROR;
				}
				fprintf(f, "%s", s.c_str());
				fclose(f);
			}
			else {
				/* try to store only one preferans problem to df or pbn file, file will not be created at all
				 * if have some bridge problems file will be created
				 */
				if (!f) {
					f = open(filepath, "w+");
					if(!f){
						showError();
						return SAVE_ERROR;
					}
					if (t == FILE_TYPE_HTML) {
						fprintf(f, PROBLEM_HTML_BEGIN);
					}
				}
				fprintf(f, "%s", s.c_str());
			}
			i++;
		}
		else {
			warning = true;
		}

	}

	if (!split && f) {
		if (t == FILE_TYPE_HTML) {
			fprintf(f, PROBLEM_HTML_END);
		}
		fclose(f);
	}

	if (t == FILE_TYPE_DF || t == FILE_TYPE_PBN) {
		for (auto& pr: m_problems) {
			if (!pr.supportFileFormat(t)) {
				message(MESSAGE_ICON_MESSAGE,
						STRING_WARNING_STORE_NO_CONTRACT_OR_NO_TRUMP_TO_DF_PBN);
				break;
			}
		}
	}

	if (warning) {
		message(MESSAGE_ICON_MESSAGE, STRING_WARNING_STORE_PREF_TO_DF_PBN);
	}
	return warning?SAVE_WARNING:SAVE_OK;
}

void ProblemVector::showError(){
	std::string s=getString(STRING_ERROR_COULD_NOT_OPEN_FILE_FOR_WRITING);
	message(MESSAGE_ICON_ERROR,s+" "+strerror(errno));
}

std::string ProblemVector::getFileFormat(int size) {
	int i, d;
	const int start = gconfig->m_firstSplitNumber;
	char b[16];

	for (d = 0, i = size + start - 1; i != 0; i /= 10) {//max number size()+start-1
		d++;
	}
	sprintf(b, "%%0%dd", d);
	return b;
}

std::string ProblemVector::fileName(const std::string& filepath,
		const std::string& buffer, int i) {
	const char*p, *q;
	std::string s;

	p = filepath.c_str();
	q = strrchr(p, '.');
	assert(q);

	s = format("%.*s", q - p, p);
	s += format(buffer.c_str(), i);
	s += q;
	return s;
}

void ProblemVector::addSave(const VString& v, const std::string& filepath,
		bool split) {
	VStringCI it;
	ProblemVector pv;
	int i;
	std::string s, s1, b;
	VProblemI it1;
	FILE*f;
	const FILE_TYPE t = getFileType(filepath);
	const int start = gconfig->m_firstSplitNumber;

	if (split) {
		for (it = v.begin(); it != v.end(); it++) {
			pv.set(*it, true);
		}
		//after pv is set
		b = getFileFormat(pv.size());
		for (i = start, it1 = pv.begin(); it1 != pv.end(); it1++) {
			s1 = it1->getContent(t, 1,pv.size());	//use problem number 1 indicates problem caption

			if (s1.length() == 0) {
				continue;
			}

			f = open(fileName(filepath, b, i).c_str(), "w+");
			assert(f);
			if (t == FILE_TYPE_HTML) {
				s1 = Problem::postproceedHTML(s1, gconfig->m_htmlStoreWithImages);
				s1 = PROBLEM_HTML_BEGIN + s1 + PROBLEM_HTML_END;
			}
			fprintf(f, "%s", s1.c_str());
			fclose(f);
			i++;
		}
	}
	else {
		b = getFileFormat(v.size());
		for (i = start, it = v.begin(); it != v.end(); it++, i++) {
			pv.set(*it, false);
			if(pv.save(fileName(filepath, b, i), false)==SAVE_ERROR){
				break;
			}
		}

	}

}

/*
 * Note sample is copied from http://stackoverflow.com/questions/10440113/simple-way-to-unzip-a-zip-file-using-zlib
 * to work copy mizip library from http://www.winimage.com/zLibDll/minizip.html
 * url http://www.winimage.com/zLibDll/unzip101h.zip unpack it copy all files except minizip.c,miniunz.c,makefile
 * for link need to add 'libz' library menu Project/properties  c/c++ build/settings mingw linker libraries add 'z' library
 * */
std::string ProblemVector::unzipFile(std::string filepath) {
	//zip vars
	const char dir_delimter = '/';
	const int MAX_FILENAME = 512;
	const int READ_SIZE = 8192;

	auto s=utf8ToLocale(filepath);
	unzFile zipfile = unzOpen(s.c_str());
	if (zipfile == NULL) {
		println("%s: not found", filepath.c_str());
		return "";
	}

	// Get info about the zip file
	unz_global_info global_info;
	if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
		println("could not read file global info");
		unzClose(zipfile);
		return "";
	}

	// Buffer to hold data read from the zip file.
	char read_buffer[READ_SIZE];
	std::string unzipped;

	// Loop to extract all files
	uint32_t i;//was in 32bit "uLong i"

	if (global_info.number_entry != 1) {
		println("error: global_info.number_entry!=1");
	}

	for (i = 0; i < global_info.number_entry; ++i) {
		// Get info about current file.
		unz_file_info file_info;
		char filename[MAX_FILENAME];
		if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME,
		NULL, 0, NULL, 0) != UNZ_OK) {
			println("could not read file info\n");
			unzClose(zipfile);
			return "";
		}

		// Check if this entry is a directory or file.
		const size_t filename_length = strlen(filename);
		if (filename[filename_length - 1] == dir_delimter) {
			// Entry is a directory, so create it.
			//println( "dir:%s", filename );
			//mkdir( filename );
		}
		else {
			// Entry is a file, so extract it.
			//println( "file:%s", filename );
			if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
				println("could not open file");
				unzClose(zipfile);
				return "";
			}

			int error = UNZ_OK;
			do {
				error = unzReadCurrentFile(zipfile, read_buffer, READ_SIZE);
				if (error < 0) {
					println("error %d", error);
					unzCloseCurrentFile(zipfile);
					unzClose(zipfile);
					return "";
				}

				// Write data to file.
				if (error > 0) {
					unzipped.append(read_buffer, error);//error - number of bytes,append this bytes
				}
			} while (error > 0);

		}

		unzCloseCurrentFile(zipfile);

		// Go the the next entry listed in the zip file.
		if ((i + 1) < global_info.number_entry) {
			if (unzGoToNextFile(zipfile) != UNZ_OK) {
				println("cound not read next file");
				unzClose(zipfile);
				return "";
			}
		}
	}

	unzClose(zipfile);

	return unzipped;

}

bool ProblemVector::onlyPreferansProblems() const {
	VProblemCI it;
	for (it = begin(); it != end(); it++) {
		if (it->isBridge()) {
			return false;
		}
	}
	return true;
}

void ProblemVector::swap(int i1, int i2) {
	Problem p = m_problems[i1];
	m_problems[i1] = m_problems[i2];
	m_problems[i2] = p;
}

void ProblemVector::removeAllPreferansProblems() {
	VProblem v;
	VProblemCI it;
	for (it = begin(); it != end(); it++) {
		if (it->isBridge()) {
			v.push_back(*it);
		}
	}
	m_problems = v;
}
