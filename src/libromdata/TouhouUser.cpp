/***************************************************************************   
* ROM Properties Page shell extension. (libromdata)                       *
* TouhouUser.cpp: Touhou Replay USER section parser.                      *
*                                                                         *
* Copyright (c) 2017 by Egor.                                             *
*                                                                         *
* This program is free software; you can redistribute it and/or modify it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation; either version 2 of the License, or (at your  *
* option) any later version.                                              *
*                                                                         *
* This program is distributed in the hope that it will be useful, but     *
* WITHOUT ANY WARRANTY; without even the implied warranty of              *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
* GNU General Public License for more details.                            *
*                                                                         *
* You should have received a copy of the GNU General Public License along *
* with this program; if not, write to the Free Software Foundation, Inc., *
* 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
***************************************************************************/

#include "TouhouUser.hpp"
#include "file/IRpFile.hpp"
#include "th_structs.h"

#include <cassert>
#include <ctime>

#include <string>


namespace LibRomData {

	/** TouhouUserReader **/

	class TouhouUserReader {
	private:
		bool is_valid;
	public:
		char* text;
		uint32_t text_sz;
		char* comment;
		uint32_t  comment_sz;
	private:
		TouhouUserReader(const TouhouUserReader& other);
		TouhouUserReader& operator=(const TouhouUserReader& other);
	public:
		TouhouUserReader(int gameType, IRpFile* file);
		~TouhouUserReader();
		bool isValid();
	};
	TouhouUserReader::TouhouUserReader(int gameType, IRpFile *file)
				:is_valid(false),text(nullptr),text_sz(0),comment(nullptr),comment_sz(0) {
		
		assert(file);
		assert(file->isOpen());
		if (file && file->isOpen()) {
			// Read offset of the USER header
			file->seek(offsetof(THRP_GenericHeader, useroffset));
			uint32_t offset;
			if (sizeof(offset) != file->read(&offset, sizeof(offset))) {
				assert(0);
			}

			file->seek(offset);
			// Read all USER sections
			while ((uint64_t)file->tell() <= file->fileSize() - sizeof(THRP_USERHeader)) {
				// TODO: don't allocate/read stuff until last section is found
				// this doesn't really matter, since there's never going to be 2 sections with same type
				// Why do we support duplicate sections? Because replayview.exe does

				// Read USER header
				THRP_USERHeader uhead;
				if (sizeof(uhead) != file->read(&uhead, sizeof(uhead))) {
					break;
				}

				// Check magic
				static const char user_magic[] = { 'U', 'S', 'E', 'R' };
				if (!memcmp(uhead.magic, user_magic, sizeof(user_magic))) {
					// Make sure the size is valid
					assert(uhead.size >= sizeof(uhead));
					if (uhead.size >= sizeof(uhead)) {
						// Read the text data
						uint32_t size = uhead.size - sizeof(uhead);
						char *ntext = new char[size+1]; // allocating +1 in case there's no terminating null
						if (size != file->read(ntext, size)) {
							// read failed, deallocate the text
							delete[] ntext;
							assert(0);
							continue;
						}

						// Read successful
						if (ntext[size - 1] != 0) {
							assert(!"USER section should end with null");
							ntext[size] = 0;
							size++;
						}

						switch (uhead.type) {
						case 0:
							delete[] text; // delete old text
							text = ntext;
							text_sz = size;
							break;
						case 1:
							delete[] comment; // delete old text
							comment = ntext;
							comment_sz = size;
							break;
						default:
							assert(!"Unknown USER section type");
							delete[] ntext;
							break;
						}

					}
				}
				// Try to skip the unknown section
				else if (-1 == file->seek(file->tell() + uhead.size)) {
					break;
				}
			}

			// If comment is default one, delete it.
			// This way we can provide a translatable string in the Ui
			if (comment) {
				// SJIS for "コメントを書けます"
				static const uint8_t youCanAddComments[] = {
					0x83, 0x52, 0x83, 0x81, 0x83, 0x93, 0x83, 0x67,
					0x82, 0xF0, 0x8F, 0x91, 0x82, 0xAF, 0x82, 0xDC,
					0x82, 0xB7,
				};
				// english version of the above string as seen in th11 patch
				static const uint8_t englishComments[] = {
					'W', 'i', 'l', 'l', ' ',
					'w', 'r', 'i', 't', 'e', ' ',
					'c', 'o', 'm', 'm', 'e', 'n', 't', 's',
				};

				if (// Check for SJIS default comment
						gameType >= TouhouReplay::TH_095 // th08/09 don't have a default comment at all
						&& comment_sz >= sizeof(youCanAddComments)
						&& !memcmp(comment, youCanAddComments, sizeof(youCanAddComments))
					|| // Check for english comment
						gameType == TouhouReplay::TH_11
						&& comment_sz >= sizeof(englishComments)
						&& !memcmp(comment, englishComments, sizeof(englishComments))
					) {
					// replace comment with null
					delete[] comment;
					comment = nullptr;
					comment_sz = 0;
				}
			}
		}
	}
	TouhouUserReader::~TouhouUserReader() {
		delete[] text;
		delete[] comment;
	}
	bool TouhouUserReader::isValid() {
		return is_valid;
	}

	/** Touhou10UserParser **/

	class Touhou10UserParser : public ITouhouUserParser {
	public:
		// stage
		bool is_extra;
		int beginStage;
		int endStage;

	private:
		bool parseLines(const std::string(&lines)[9]);
	public:
		rp_string getStage();
		Touhou10UserParser(int gameType, IRpFile* file);
	};

	bool Touhou10UserParser::parseLines(const std::string(&lines)[9]) {
		// Line 0 - "東方XYZ リプレイファイル情報" - "Touhou XYZ replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Chara %s"
		chara = spaceString(lines[4], "Chara");

		// Line 5 - "Rank %s"
		rank = spaceString(lines[5], "Rank");

		// Line 6 - which stage was completed
		if (lines[6] == "Extra Stage Clear") {
			is_extra = true;
			is_clear = true;
		}
		else if (lines[6] == "Extra Stage") {
			is_extra = true;
			is_clear = false;
		}
		else if (lines[6] == "Stage All Clear") {
			is_extra = false;
			is_clear = true;
		}
		else {
			is_extra = false;
			is_clear = false;
			std::string stage = spaceString(lines[6], "Stage");
			size_t pos = stage.find(' ');
			beginStage = std::stoi(stage.substr(0, pos));
			if (pos == std::string::npos) {
				endStage = beginStage;
			}
			else {
				endStage = std::stoi(spaceString(stage.substr(pos + 1), "\x81\x60")); // \x81\x60 is SJIS '〜'
			}
		}

		// Line 7 - "Score %d"
		score = std::stol(spaceString(lines[7], "Score"));

		// Line 8 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[8], "Slow"), "Rate"));

		return true;
	}

	rp_string Touhou10UserParser::getStage() {
		char buf[6];
		if (is_extra) {
			return _RP("Extra");
		}
		else if (is_clear) {
			return _RP("All");
		}
		else if (beginStage != endStage) {
			snprintf(buf, sizeof(buf), "%d ~ %d", beginStage, endStage);
			return cp1252_sjis_to_rp_string(buf, -1);
		}
		else {
			snprintf(buf, sizeof(buf), "%d", beginStage);
			return cp1252_sjis_to_rp_string(buf, -1);
		}
	}
	Touhou10UserParser::Touhou10UserParser(int gameType, IRpFile* file) :
					beginStage(0), endStage(0), is_extra(false) {
		TouhouUserReader ur(gameType, file);
		if (ur.comment) {
			comment = std::string(ur.comment);
			comment_present = true;
		}
		else {
			comment_present = false;
		}

		if (!ur.text) {
			return;
		}

		std::string lines[9];
		if (!breakLines(ur.text, ur.text_sz, lines, 9)) {
			return;
		}

		is_valid = parseLines(lines);

	}

	/** Touhou095UserParser **/

	class Touhou095UserParser : public ITouhouUserParser {
	public:
		// stage
		bool is_extra;
		int level;
		int scene;

	private:
		bool parseLines(const std::string(&lines)[8]);
	public:
		rp_string getStage();
		Touhou095UserParser(int gameType, IRpFile* file);

		rp_string Touhou095UserParser::isClear();
	};

	bool Touhou095UserParser::parseLines(const std::string(&lines)[8]) {
		// Line 0 - "東方文花帖 リプレイファイル情報" - "Touhou Bunkachou replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - Level
		// Line 4 - Scene
		// the stage is combination of level and scene, e.g. 1-1, 4-2, etc.
		// level can also be EX (extra) e.g. EX-1, EX-2

		std::string strlevel = spaceString(lines[3], "Level");
		if (strlevel == "EX") {
			is_extra = true;
			level = 0;
		}
		else {
			is_extra = false;
			level = std::stoi(strlevel);
		}

		scene = std::stoi(spaceString(lines[4], "Scene"));

		// Line 5 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[5], "Date"));

		// Line 6 - "Score %d"
		score = std::stol(spaceString(lines[6], "Score"));

		// Line 7 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[7], "Slow"), "Rate"));

		

		// In StB you always play as Aya.
		chara = "Aya Shameimaru";

		// There are no Ranks in StB
		rank = "N/a";

		return true;
	}

	rp_string Touhou095UserParser::getStage() {
		char buf[5];
		if (is_extra) {
			snprintf(buf, sizeof(buf), "EX-%d", scene);
		}
		else {
			snprintf(buf, sizeof(buf), "%d-%d", level, scene);
		}
		return cp1252_sjis_to_rp_string(buf, -1);
	}

	Touhou095UserParser::Touhou095UserParser(int gameType, IRpFile* file) :
		level(0), scene(0), is_extra(false) {
		TouhouUserReader ur(gameType, file);
		if (ur.comment) {
			comment = std::string(ur.comment);
			comment_present = true;
		}
		else {
			comment_present = false;
		}

		if (!ur.text) {
			return;
		}

		std::string lines[8];
		if (!breakLines(ur.text, ur.text_sz, lines, 8)) {
			return;
		}

		is_valid = parseLines(lines);

	}

	rp_string Touhou095UserParser::isClear() {
		// TODO: figure out how to find this value
		return _RP("N/a");
	}

	/** Touhou125UserParser **/
	class Touhou125UserParser : public ITouhouUserParser {
	public:
		// stage
		std::string stage;

	private:
		bool parseLines(const std::string(&lines)[8]);
	public:
		rp_string getStage();
		Touhou125UserParser(int gameType, IRpFile* file);

		rp_string Touhou125UserParser::isClear();
	};

	bool Touhou125UserParser::parseLines(const std::string(&lines)[8]) {
		// Line 0 - "東方文花帖 リプレイファイル情報" - "Touhou Bunkachou replay file info"
		// (skipped)

		// Line 1 - "Version %s"
		version = spaceString(lines[1], "Version");

		// Line 2 - "Name %s"
		name = spaceString(lines[2], "Name");

		// Line 3 - "Date %.2d/%.2d/%.2d %.2d:%.2d" (yy/mm/dd)
		time = parseDate(spaceString(lines[3], "Date"));

		// Line 4 - "Chara %s"
		chara = spaceString(lines[4], "Chara");

		// Line 5 - "%s-%d" (where %s is number from 1 to 12, or "EX" or "SP")
		stage = lines[5];

		// Line 6 - "Score %d"
		score = std::stol(spaceString(lines[6], "Score"));

		// Line 7 - "Slow Rate %2.2f"
		// TODO: maybe make it less hacky?
		slowrate = std::stof(spaceString(spaceString(lines[7], "Slow"), "Rate"));

		// There are no Ranks in DS
		rank = "N/a";

		return true;
	}

	rp_string Touhou125UserParser::getStage() {
		return cp1252_sjis_to_rp_string(stage.c_str(), -1);
	}

	Touhou125UserParser::Touhou125UserParser(int gameType, IRpFile* file) {
		TouhouUserReader ur(gameType, file);
		if (ur.comment) {
			comment = std::string(ur.comment);
			comment_present = true;
		}
		else {
			comment_present = false;
		}

		if (!ur.text) {
			return;
		}

		std::string lines[8];
		if (!breakLines(ur.text, ur.text_sz, lines, 8)) {
			return;
		}

		is_valid = parseLines(lines);

	}

	rp_string Touhou125UserParser::isClear() {
		// TODO: figure out how to find this value
		return _RP("N/a");
	}

	/** ITouhouUserParser **/
	std::string ITouhouUserParser::spaceString(std::string line, const std::string& keyword) {
		size_t pos = line.find(' ');

		if (pos != std::string::npos) {
			// Check the keyword
			if (line.substr(0, pos) != keyword) is_broken = true;

			return line.substr(pos + 1);
		}
		else {
			is_broken = true;
			return "";
		}
	}

	time_t ITouhouUserParser::parseDate(std::string datestr) {
		// format: yy/mm/dd hh:mm
		if (datestr.length() != 14
			|| datestr[2] != '/'
			|| datestr[5] != '/'
			|| datestr[8] != ' '
			|| datestr[11] != ':') return -1;

		tm t;
		t.tm_year = 100 + std::stoi(datestr);
		t.tm_mon = std::stoi(datestr.substr(3));
		t.tm_mday = std::stoi(datestr.substr(6));
		t.tm_hour = std::stoi(datestr.substr(9));
		t.tm_min = std::stoi(datestr.substr(12));
		t.tm_sec = 0;
		t.tm_isdst = 0; // unknown
						// other fields don't need to be set for mktime to work.
		return mktime(&t);
	}

	bool ITouhouUserParser::breakLines(char* text, uint32_t size, std::string *lines, int linecount) {
		// Make sure there's a null at the very end
		assert(!text[size - 1]);
		if (text[size - 1]) return false;

		char *p = text;
		for (int i = 0; i < linecount; i++) {
			// Look for a new-line
			char* p2 = strstr(p, "\r\n");
			if (!p2) return false; // line not found

			lines[i] = std::string(p, p2 - p);
			p = p2 + 2; // skip \r\n
		}

		return true;
	}

	ITouhouUserParser::ITouhouUserParser() : is_valid(false), is_broken(false),
		time(-1), is_clear(false), score(0), slowrate(99.99f), comment_present(false)
	{
	}
	bool ITouhouUserParser::isValid() {
		return is_valid;
	}
	bool ITouhouUserParser::isBroken() {
		return is_broken;
	}
	rp_string ITouhouUserParser::getVersion() {
		return cp1252_sjis_to_rp_string(version.c_str(), -1);
	}
	rp_string ITouhouUserParser::getName() {
		return cp1252_sjis_to_rp_string(name.c_str(), -1);
	}
	time_t ITouhouUserParser::getTime() {
		return time;
	}
	rp_string ITouhouUserParser::getChara() {
		return cp1252_sjis_to_rp_string(chara.c_str(), -1);
	}
	rp_string ITouhouUserParser::getRank() {
		return cp1252_sjis_to_rp_string(rank.c_str(), -1);
	}

	rp_string ITouhouUserParser::isClear() {
		return is_clear ? _RP("Yes") : _RP("No");
	}
	int ITouhouUserParser::getScore() {
		return score;
	}
	rp_string ITouhouUserParser::getSlowRate() {
		char buf[6];
		snprintf(buf, sizeof(buf), "%2.2f", slowrate);
		return cp1252_sjis_to_rp_string(buf, -1);
	}
	rp_string ITouhouUserParser::getComment() {
		if (comment_present) {
			return cp1252_sjis_to_rp_string(comment.c_str(), -1);
		}
		else {
			return _RP("None");
		}
	}
	ITouhouUserParser::~ITouhouUserParser()
	{
	}
	
	/** TouhouUserParserFactory **/
	template<typename T>
	ITouhouUserParser* TouhouUserParserFactory::construct(int gameType, IRpFile* file) {
		return new T(gameType, file);
	}
	ITouhouUserParser* TouhouUserParserFactory::getInstance(int gameType, IRpFile* file) {
		switch (gameType) {
			case TouhouReplay::TH_095:
				return construct<Touhou095UserParser>(gameType, file);
			case TouhouReplay::TH_125:
				return construct<Touhou125UserParser>(gameType, file);
			case TouhouReplay::TH_10:
			case TouhouReplay::TH_11:
			case TouhouReplay::TH_12:
			case TouhouReplay::TH_13:
			case TouhouReplay::TH_14:
			case TouhouReplay::TH_15:
				return construct<Touhou10UserParser>(gameType, file);
			default:
				return nullptr; // not supported
		}
	}
}