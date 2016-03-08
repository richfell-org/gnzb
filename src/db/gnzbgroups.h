/*
	gnzb Usenet downloads using NZB index files

    Copyright (C) 2016  Richard J. Fellinger, Jr

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, see <http://www.gnu.org/licenses/> or write
	to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301 USA.
*/
#ifndef __APP_GROUPS_HEADER__
#define __APP_GROUPS_HEADER__

#include <vector>
#include <tuple>
#include "sqlitedb.h"

class GNzbGroup
{
// construction
public:

	GNzbGroup();
	GNzbGroup(GNzbGroup&& that);
	GNzbGroup(const GNzbGroup& that);
	GNzbGroup(
		int metric,
    	const char* name,
		const char *colorText,
		bool doMove = false,
		bool doMoveTvSeries = false,
		bool doCleanup = false,
		bool doAutoAssign = false,
		const char *moveToDir = 0,
		const char *incRegexOrTokens = 0,
		const char *excRegexOrTokens = 0,
		unsigned long sizeMin = 0,
		unsigned long sizeMax = 0);
	~GNzbGroup();

// atributes
public:

	int get_db_id() const { return m_rec_id; }

	int getMetric() const { return mMetric; }
	void setMetric(int metric) { mMetric = metric; }

	const std::string& getName() const { return mName; }
	void setName(const char *name) { mName.assign(name); }

	const std::string& getColorText() const { return mColorText; }
	void setColorText(const char *colorText) { mColorText.assign(colorText); }

	bool doMoveToDir() const { return mDoMove; }
	void setDoMoveToDir(bool doMove = true) { mDoMove = doMove; }

	bool doMoveToTvSeriesDir() const { return mDoMoveTvSeries; }
	void setDoMoveToTvSeriesDir(bool doMove = true) { mDoMoveTvSeries = doMove; }

	bool doCleanup() const { return mDoCleanup; }
	void setDoCleanup(bool doClean = true) { mDoCleanup = doClean; }

	bool doAutoAssignGroup() const { return mDoAutoAssign; }
	void setAutoAssignGroup(bool doAutoAssign = true) { mDoAutoAssign = doAutoAssign; }

	const std::string& getMoveToDir() const { return mMoveToDir; }
	void setMoveToDir(const char *moveToDir) { mMoveToDir = moveToDir; }

	const std::vector<std::string>& getNameIncludes() const { return mAutoAssignInc; }
	std::string getNameIncludesCSV() const;
	void setNameIncludes(const char *regexOrTokenStr);
	void addNameIncludesToken(const char *token);

	const std::vector<std::string>& getNameExcludes() const { return mAutoAssignExc; }
	std::string getNameExcludesCSV() const;
	void setNameExcludes(const char *regexOrTokenStr);
	void addNameExcludesToken(const char *token);

	unsigned long getMinSizeMB() const { return mSizeMin; }
	void setMinSizeMB(unsigned long minMB) { mSizeMin = minMB; }

	unsigned long getMaxSizeMB() const { return mSizeMax; }
	void setMaxSizeMB(unsigned long maxMB) { mSizeMax = maxMB; }

// operations
public:

	bool doesMatch(const std::string& name) const;

	bool doesMatchInclude(const std::string& name) const;
	bool doesMatchExclude(const std::string& name) const;

	bool isDbRecord() const { return 0 < m_rec_id; }
	bool operator ==(const GNzbGroup& that) const;
	bool operator !=(const GNzbGroup& that) const { return !operator ==(that); }

	GNzbGroup& operator =(GNzbGroup&& that);
	GNzbGroup& operator =(const GNzbGroup& that);

private:

	friend class GNzbGroups;

	GNzbGroup(
		int dbRecId,
		int metric,
    	const unsigned char* name,
		const unsigned char *colorText,
		bool doMove,
		bool doMoveTvSeries,
		bool doCleanup,
		bool doAutoAssign,
		const unsigned char *moveToDir,
		const unsigned char *incString,
		const unsigned char *excString,
		unsigned long sizeMin,
		unsigned long sizeMax);

	void set_db_id(int id) { m_rec_id = id; }
	static void parseRegexOrTokens(const char *regExOrTokens, std::vector<std::string>& result);
	static bool areTokensEqual(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs);
	static bool isRegexToken(const std::string& token);
	static bool doesMatchAny(const std::string& name, const std::vector<std::string>& tokens);

	int m_rec_id;
	int mMetric;
	std::string mName;
	std::string mColorText;
	bool mDoMove;
	bool mDoMoveTvSeries;
	bool mDoCleanup;
	bool mDoAutoAssign;
	std::string mMoveToDir;
	std::vector<std::string> mAutoAssignInc;
	std::vector<std::string> mAutoAssignExc;
	unsigned long mSizeMin;
	unsigned long mSizeMax;
};

/*
 * 
 */
class GNzbGroups
{
// construction
public:

	GNzbGroups();
	~GNzbGroups();

// operations
public:

	int get_group_colors(std::vector<std::tuple<int,int,std::string,std::string>>& result);
	int get_groups(std::vector<GNzbGroup>& result);

	std::vector<GNzbGroup> get_groups() const;

	void update_group(GNzbGroup& group) throw(Sqlite3::Error);
	void insert_group(GNzbGroup& group) throw(Sqlite3::Error);
	void delete_group(GNzbGroup& group) throw(Sqlite3::Error);

// implementation
private:

	mutable Sqlite3::Db m_db;
};

#endif  /* __APP_GROUPS_HEADER__ */