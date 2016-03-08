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
#include "gnzbgroups.h"
#include "gnzbdb.h"
#include <stdexcept>
#include <regex>
#include <sstream>
#include <iostream>

#include <sys/types.h>

void GNzbGroup::parseRegexOrTokens(const char *regExOrTokens, std::vector<std::string>& result)
{
	// ignore NULL regex/token string
	if(0 == regExOrTokens) return;

	bool isRegEx = false;
	bool isEsc = false;
	bool isEndTok = false;
	std::stringstream tokStream;

	for(register unsigned int i = 0; 0 != regExOrTokens[i]; ++i)
	{
		switch(regExOrTokens[i])
		{
			case '/':
				tokStream << regExOrTokens[i];
				if(!isRegEx)
					isRegEx = true;
				else
					isRegEx = false;
				isEndTok = false;
				break;
			case '\\':
				if(isRegEx)
					tokStream << regExOrTokens[i];
				else
					isEsc = true;
				isEndTok = false;
				break;
			case ',':
				if(isRegEx || isEsc)
				{
					tokStream << regExOrTokens[i];
					isEsc = false;
				}
				else
				{
					result.push_back(tokStream.str());
					tokStream.str("");
				}
				isEndTok = true;
				break;
			case ' ':
				if(!isEndTok)
					tokStream << regExOrTokens[i];
				break;
			default:
				isEndTok = false;
				tokStream << regExOrTokens[i];
				break;
		}
	}

	if(!tokStream.str().empty())
		result.push_back(tokStream.str());
}

bool GNzbGroup::areTokensEqual(const std::vector<std::string>& lhs, const std::vector<std::string>& rhs)
{
	if(lhs.size() != rhs.size()) return false;

	for(register unsigned int i = 0; i < lhs.size(); ++i)
		if(0 != lhs[i].compare(rhs[i]))
		   return false;

	return true;
}

bool GNzbGroup::isRegexToken(const std::string& token)
{
	register std::string::size_type len = token.length();

	if((0 < len) && ('/' == token[0]) && ('/' == token[len - 1]))
		return true;

	return false;
}

bool GNzbGroup::doesMatchAny(const std::string& name, const std::vector<std::string>& tokens)
{
	for(auto& token : tokens)
	{
		if(isRegexToken(token))
		{
			if(std::regex_match(name, std::regex(token.substr(1, token.size() - 2))))
				 return true;
		}
		else if(std::string::npos != name.find(token))
		{
			return true;
		}
	}

	return false;
}

GNzbGroup::GNzbGroup()
:   mMetric(-1), mName(""), mColorText(""), mDoMove(false), mDoMoveTvSeries(false),
	mDoCleanup(false), mDoAutoAssign(false), mMoveToDir(""), mSizeMin(0), mSizeMax(0)
{
	mAutoAssignInc.clear();
	mAutoAssignExc.clear();
}

GNzbGroup::GNzbGroup(GNzbGroup&& that)
:   m_rec_id(that.m_rec_id),
	mMetric(that.mMetric),
	mName(std::move(that.mName)),
	mColorText(std::move(that.mColorText)),
	mDoMove(that.mDoMove),
	mDoMoveTvSeries(that.mDoMoveTvSeries),
	mDoCleanup(that.mDoCleanup),
	mDoAutoAssign(that.mDoAutoAssign),
	mMoveToDir(std::move(that.mMoveToDir)),
	mAutoAssignInc(std::move(that.mAutoAssignInc)),
	mAutoAssignExc(std::move(that.mAutoAssignExc)),
	mSizeMin(that.mSizeMin),
	mSizeMax(that.mSizeMax)
{
	that.m_rec_id = -1;
}

GNzbGroup::GNzbGroup(const GNzbGroup& that)
:   m_rec_id(that.m_rec_id),
	mMetric(that.mMetric),
	mName(that.mName),
	mColorText(that.mColorText),
	mDoMove(that.mDoMove),
	mDoMoveTvSeries(that.mDoMoveTvSeries),
	mDoCleanup(that.mDoCleanup),
	mDoAutoAssign(that.mDoAutoAssign),
	mMoveToDir(that.mMoveToDir),
	mAutoAssignInc(that.mAutoAssignInc),
	mAutoAssignExc(that.mAutoAssignExc),
	mSizeMin(that.mSizeMin),
	mSizeMax(that.mSizeMax)
{
}

GNzbGroup::GNzbGroup(
	int metric, const char* name, const char *colorText, bool doMove, bool doMoveTvSeries, bool doCleanup, bool doAutoAssign,
	const char *moveToDir, const char *incRegexOrTokens, const char *excRegexOrTokens, unsigned long sizeMin,
	unsigned long sizeMax)
:   mMetric(metric), mName(name), mColorText(colorText), mDoMove(doMove), mDoMoveTvSeries(doMoveTvSeries),
	mDoCleanup(doCleanup), mDoAutoAssign(doAutoAssign), mMoveToDir(moveToDir ? moveToDir : ""), mSizeMin(sizeMin),
	mSizeMax(sizeMax)
{
	mAutoAssignInc.clear();
	parseRegexOrTokens(incRegexOrTokens, mAutoAssignInc);

	mAutoAssignExc.clear();
	parseRegexOrTokens(excRegexOrTokens, mAutoAssignExc);
}

GNzbGroup::GNzbGroup(
	int dbRecId, int metric, const unsigned char* name, const unsigned char *colorText, bool doMove,
	bool doMoveTvSeries, bool doCleanup,	bool doAutoAssign, const unsigned char *moveToDir,
	const unsigned char *incRegexOrTokens, const unsigned char *excRegexOrTokens, unsigned long sizeMin,
	unsigned long sizeMax)
:   m_rec_id(dbRecId), mMetric(metric), mName((const char*)name), mColorText((const char*)colorText),
	mDoMove(doMove), mDoMoveTvSeries(doMoveTvSeries), mDoCleanup(doCleanup), mDoAutoAssign(doAutoAssign),
	mMoveToDir(moveToDir ? (const char*)moveToDir : ""), mSizeMin(sizeMin), mSizeMax(sizeMax)
{
	mAutoAssignInc.clear();
	parseRegexOrTokens((const char*)incRegexOrTokens, mAutoAssignInc);

	mAutoAssignExc.clear();
	parseRegexOrTokens((const char*)excRegexOrTokens, mAutoAssignExc);
}

GNzbGroup::~GNzbGroup()
{
}

std::string GNzbGroup::getNameIncludesCSV() const
{
	std::stringstream csvStream("");

	// get the "name includes" tokens into a CSV string
	for(register unsigned int i = 0; i < mAutoAssignInc.size(); ++i)
	{
		if(i > 0) csvStream << ", ";

		csvStream << mAutoAssignInc[i];
	}

	return std::move(csvStream.str());
}

void GNzbGroup::setNameIncludes(const char *regexOrTokenStr)
{
	mAutoAssignInc.clear();
	parseRegexOrTokens(regexOrTokenStr, mAutoAssignInc);
}

std::string GNzbGroup::getNameExcludesCSV() const
{
	std::stringstream csvStream("");

	// get the "name excludes" tokens into a CSV string
	for(register unsigned int i = 0; i < mAutoAssignExc.size(); ++i)
	{
		if(i > 0) csvStream << ", ";

		csvStream << mAutoAssignExc[i];
	}

	return std::move(csvStream.str());
}

void GNzbGroup::setNameExcludes(const char *regexOrTokenStr)
{
	mAutoAssignExc.clear();
	parseRegexOrTokens(regexOrTokenStr, mAutoAssignExc);
}

bool GNzbGroup::doesMatch(const std::string& name) const
{
	return doesMatchInclude(name) && !doesMatchExclude(name);
}

bool GNzbGroup::doesMatchInclude(const std::string& name) const
{
	return doesMatchAny(name, mAutoAssignInc);
}

bool GNzbGroup::doesMatchExclude(const std::string& name) const
{
	return doesMatchAny(name, mAutoAssignExc);
}

bool GNzbGroup::operator ==(const GNzbGroup& that) const
{
	return(
		((m_rec_id == that.m_rec_id)
			&& (mMetric == that.mMetric)
		    && (0 == mColorText.compare(that.mColorText))
			&& (mDoMove == that.mDoMove)
			&& (mDoMoveTvSeries == that.mDoMoveTvSeries)
			&& (mDoCleanup == that.mDoCleanup)
			&& (mDoAutoAssign == that.mDoAutoAssign)
			&& (0 == mMoveToDir.compare(that.mMoveToDir)))
		&& areTokensEqual(mAutoAssignInc, that.mAutoAssignInc)
		&& areTokensEqual(mAutoAssignExc, that.mAutoAssignExc));
}

GNzbGroup& GNzbGroup::operator =(GNzbGroup&& that)
{
	m_rec_id = that.m_rec_id;
	mMetric = that.mMetric;
	mName = std::move(that.mName);
	mColorText = std::move(that.mColorText);
	mDoMove = that.mDoMove;
	mDoMoveTvSeries = that.mDoMoveTvSeries;
	mDoCleanup = that.mDoCleanup;
	mDoAutoAssign = that.mDoAutoAssign;
	mMoveToDir = std::move(that.mMoveToDir);
	mAutoAssignInc = std::move(that.mAutoAssignInc);
	mAutoAssignExc = std::move(that.mAutoAssignExc);
	mSizeMin = that.mSizeMin;
	mSizeMax = that.mSizeMax;

	that.m_rec_id = -1;
}

GNzbGroup& GNzbGroup::operator =(const GNzbGroup& that)
{
	m_rec_id = that.m_rec_id;
	mMetric = that.mMetric;
	mName = that.mName;
	mColorText = that.mColorText;
	mDoMove = that.mDoMove;
	mDoMoveTvSeries = that.mDoMoveTvSeries;
	mDoCleanup = that.mDoCleanup;
	mDoAutoAssign = that.mDoAutoAssign;
	mMoveToDir = that.mMoveToDir;
	mAutoAssignInc = that.mAutoAssignInc;
	mAutoAssignExc = that.mAutoAssignExc;
	mSizeMin = that.mSizeMin;
	mSizeMax = that.mSizeMax;
}

GNzbGroups::GNzbGroups()
:   m_db(open_app_db())
{
}

GNzbGroups::~GNzbGroups()
{
}

int GNzbGroups::get_group_colors(std::vector<std::tuple<int,int,std::string,std::string>>& result)
{
	Sqlite3::Stmt stmt = m_db.prepareStatement(
		"select id, metric, name, color from NzbGroups order by metric asc, id asc");

	if(!stmt) return 0;

	register int i = 0;
	for(; SQLITE_ROW == stmt.step(); ++i)
	{
		result.push_back(std::tuple<int,int,std::string,std::string>(
			stmt.getResultInt(0),					// DB record ID
			stmt.getResultInt(1),					// metric
			(const char*)stmt.getResultText(1),		// name
			(const char*)stmt.getResultText(2)		// group color string
		));
	}

	stmt.reset();

	return i;
}

int GNzbGroups::get_groups(std::vector<GNzbGroup>& result)
{
	Sqlite3::Stmt stmt = m_db.prepareStatement("select * from NzbGroups order by metric asc, id asc");

	if(!stmt) return 0;

	register int i = 0;
	for(; SQLITE_ROW == stmt.step(); ++i)
	{
		result.push_back(GNzbGroup(
			stmt.getResultInt(0),			// DB record ID
		    stmt.getResultInt(1),			// metric
			stmt.getResultText(2),			// name
			stmt.getResultText(3),			// group color string
			(1 == stmt.getResultInt(4)),	// doMove
			(1 == stmt.getResultInt(5)),	// doMoveTvSeries
			(1 == stmt.getResultInt(6)),	// doCleanup
			(1 == stmt.getResultInt(7)),	// doAutoAssign
			stmt.getResultText(8),			// move to dir
			stmt.getResultText(9),			// includes tokens
			stmt.getResultText(10),			// excludes tokens
			stmt.getResultInt(11),			// min size
			stmt.getResultInt(12)			// max size
		));
	}

	stmt.reset();

	return i;
}

std::vector<GNzbGroup> GNzbGroups::get_groups() const
{
	std::vector<GNzbGroup> result;
	
	Sqlite3::Stmt stmt = m_db.prepareStatement("select * from NzbGroups order by metric asc, id asc");
	if(stmt)
	{
		while(SQLITE_ROW == stmt.step())
		{
			result.emplace_back(GNzbGroup(
				stmt.getResultInt(0),			// DB record ID
				stmt.getResultInt(1),			// metric
				stmt.getResultText(2),			// name
				stmt.getResultText(3),			// group color string
				(1 == stmt.getResultInt(4)),	// doMove
				(1 == stmt.getResultInt(5)),	// doMoveTvSeries
				(1 == stmt.getResultInt(6)),	// doCleanup
				(1 == stmt.getResultInt(7)),	// doAutoAssign
				stmt.getResultText(8),			// move to dir
				stmt.getResultText(9),			// includes tokens
				stmt.getResultText(10),			// excludes tokens
				stmt.getResultInt(11),			// min size
				stmt.getResultInt(12)			// max size
			));
		}

		stmt.reset();
	}

	return result;
}

void GNzbGroups::update_group(GNzbGroup& group) 
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = m_db.prepareStatement(
		"update NzbGroups set metric = ?, name = ?, color = ?, doMove = ?, doCleanup = ?, doAutoAssign = ?, moveTo = ?, "
        "autoAssignNameIncludes = ?, autoAssignNameExcludes = ?, autoAssignMinSize = ?, autoassignMaxSize = ? where id = ?");

	if(stmt)
	{
		std::string nameIncludes("");
		std::string nameExcludes("");

		// get the "name includes" tokens into a CSV string
		const std::vector<std::string>& incList = group.getNameIncludes();
		for(register unsigned int i = 0; i < incList.size(); ++i)
		{
			if(i > 0) nameIncludes.append(", ");

			nameIncludes.append(incList[i]);
		}

		// get the "name excludes" tokens into a CSV string
		const std::vector<std::string>& excList = group.getNameExcludes();
		for(register unsigned int i = 0; i < excList.size(); ++i)
		{
			if(i > 0) nameExcludes.append(", ");

			nameExcludes.append(excList[i]);
		}

		// bind the updated values to the statment
		stmt.bindInt(1, group.getMetric());
		stmt.bindText(2, group.getName().c_str());
		stmt.bindText(3, group.getColorText().c_str());
		stmt.bindInt(4, group.doMoveToDir() ? 1 : 0);
		stmt.bindInt(5, group.doCleanup() ? 1 : 0);
		stmt.bindInt(6, group.doAutoAssignGroup() ? 1 : 0);
		stmt.bindText(7, group.getMoveToDir().c_str());
		stmt.bindText(8, nameIncludes.c_str());
		stmt.bindText(9, nameExcludes.c_str());
		stmt.bindInt(10, group.getMinSizeMB());
		stmt.bindInt(11, group.getMaxSizeMB());
		stmt.bindInt(12, group.get_db_id());

		stmt.step();
		stmt.reset();
	}
}

void GNzbGroups::insert_group(GNzbGroup& group)
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = m_db.prepareStatement(
		"insert into NzbGroups (metric,name,color,doMove,doCleanup,doAutoAssign,moveTo,autoAssignNameIncludes,autoAssignNameExcludes,autoAssignMinSize,autoassignMaxSize) "
		"values(?,?,?,?,?,?,?,?,?,?,?)");

	if(stmt)
	{
		std::string nameIncludes("");
		std::string nameExcludes("");

		// get the "name includes" tokens into a CSV string
		const std::vector<std::string>& incList = group.getNameIncludes();
		for(register unsigned int i = 0; i < incList.size(); ++i)
		{
			if(i > 0) nameIncludes.append(", ");

			nameIncludes.append(incList[i]);
		}

		// get the "name excludes" tokens into a CSV string
		const std::vector<std::string>& excList = group.getNameExcludes();
		for(register unsigned int i = 0; i < excList.size(); ++i)
		{
			if(i > 0) nameExcludes.append(", ");

			nameIncludes.append(excList[i]);
		}

		// bind the updated values to the statment
		stmt.bindInt(1, group.getMetric());
		stmt.bindText(2, group.getName().c_str());
		stmt.bindText(3, group.getColorText().c_str());
		stmt.bindInt(4, group.doMoveToDir() ? 1 : 0);
		stmt.bindInt(5, group.doCleanup() ? 1 : 0);
		stmt.bindInt(6, group.doAutoAssignGroup() ? 1 : 0);
		stmt.bindText(7, group.getMoveToDir().c_str());
		stmt.bindText(8, nameIncludes.c_str());
		stmt.bindText(9, nameExcludes.c_str());
		stmt.bindInt(10, group.getMinSizeMB());
		stmt.bindInt(11, group.getMaxSizeMB());

		stmt.step();
		group.set_db_id(m_db.getLastInsertRowId());
		stmt.reset();
	}
}

void GNzbGroups::delete_group(GNzbGroup& group)
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = m_db.prepareStatement("delete from NzbGroups where id = ?");

	if(stmt && group.isDbRecord())
	{
		stmt.bindInt(1, group.get_db_id());
		stmt.step();
		stmt.reset();

		group.set_db_id(-1);
	}
}