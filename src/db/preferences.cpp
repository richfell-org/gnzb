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
#include "preferences.h"
#include "gnzbdb.h"
#include <mutex>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <string.h>

/*
std::string getApplicationDbPath()
{
	static std::string result("");
	static std::mutex dbPathInitMutex;

	std::unique_lock<std::mutex> initLock(dbPathInitMutex);
	if(result.empty())
	{
		struct passwd *pPasswdEntry = getpwuid(getuid());
		if(0 == pPasswdEntry)
			result.assign("enzybee.db");
		else
		{
			result.assign(pPasswdEntry->pw_dir);
			if('/' != result[result.length() - 1])
				result.append("/");
			result.append(".enzybee/enzybee.db");
		}
	}
	initLock.unlock();

	return result;
}
*/

PrefsNntpServer::PrefsNntpServer()
:   mRecId(-1), mConnCount(-1), mUseSsl(false), mEnabled(false)
{
}

PrefsNntpServer::PrefsNntpServer(
	int dbRecId, const unsigned char* name, const unsigned char* url, const unsigned char* port,
	const unsigned char* user, const unsigned char* passwd, int connCount, bool usesSsl, bool isEnabled)
:   mRecId(dbRecId),
	mName((const char *)name),
	mUrl((const char *)url),
	mPort(port ? (const char *)port : "119"),
	mUser(user ? (const char *)user : ""),
	mPass(passwd ? (const char *)passwd : ""),
	mConnCount(connCount),
	mUseSsl(usesSsl),
	mEnabled(isEnabled)
{
}

PrefsNntpServer::PrefsNntpServer(
   	const char* name, const char* url, const char* port, const char* user,
	const char* passwd,	int connCount, bool usesSsl, bool isEnabled)
:   mRecId(-1),
	mName(name),
	mUrl(url),
	mPort(port ? port : "119"),
	mUser(user ? user : ""),
	mPass(passwd ? passwd : ""),
	mConnCount(connCount),
	mUseSsl(usesSsl),
	mEnabled(isEnabled)
{
}

PrefsNntpServer::~PrefsNntpServer()
{
	mRecId = -1;
}

bool PrefsNntpServer::operator ==(const PrefsNntpServer& that) const
{
	return(
		(mRecId == that.mRecId)
		&& (0 == mName.compare(that.mName))
		&& (0 == mUrl.compare(that.mUrl))
		&& (0 == mPort.compare(that.mPort))
	    && (0 == mUser.compare(that.mUser))
	    && (0 == mPass.compare(that.mPass))
	    && (mConnCount == that.mConnCount)
	    && (mUseSsl == that.mUseSsl)
	    && (mEnabled == that.mEnabled));
}

AppPreferences::AppPreferences()
:   mDb(get_app_db_filename())
{
	mStmtCountNntpServers = mDb.prepareStatement("select count(id) from NntpServers");
	mStmtSelectNntpServers = mDb.prepareStatement("select * from NntpServers");
}

AppPreferences::AppPreferences(AppPreferences&& that)
:   mDb(std::move(that.mDb)),
	mStmtCountNntpServers(std::move(that.mStmtCountNntpServers)),
	mStmtSelectNntpServers(std::move(that.mStmtSelectNntpServers))
{
}

AppPreferences::~AppPreferences()
{
}

int AppPreferences::getNntpServerCount()
throw(Sqlite3::Error)
{
	int result = 0;
	if(mStmtCountNntpServers)
	{
		if(SQLITE_ROW == mStmtCountNntpServers.step())
			result = mStmtCountNntpServers.getResultInt(0);
		mStmtCountNntpServers.reset();
	}
	return result;
}

int AppPreferences::getNntpServers(std::vector<PrefsNntpServer>& entries) const
throw(Sqlite3::Error)
{
	if(!mStmtSelectNntpServers) return 0;

	register int i = 0;
	for(; SQLITE_ROW == mStmtSelectNntpServers.step(); ++i)
	{
		entries.push_back(PrefsNntpServer(
			mStmtSelectNntpServers.getResultInt(0),		// DB record ID
			mStmtSelectNntpServers.getResultText(1),	// user given name (alias)
			mStmtSelectNntpServers.getResultText(2),	// URL for NNTP server
			mStmtSelectNntpServers.getResultText(3),	// NNTP server port
		    mStmtSelectNntpServers.getResultText(4),	// user name for NNTP service
			mStmtSelectNntpServers.getResultText(5),	// password for NNTP service
			mStmtSelectNntpServers.getResultInt(6),		// Connection allowed to NNTP server
			(1 == mStmtSelectNntpServers.getResultInt(7)), // Uses SSL connections?
			(1 == mStmtSelectNntpServers.getResultInt(8))  // User enabled/disabled?
		));
	}

	mStmtSelectNntpServers.reset();
	return i;
}

std::vector<PrefsNntpServer> AppPreferences::get_nntp_servers() const
throw(Sqlite3::Error)
{
	std::vector<PrefsNntpServer> result;

	if(mStmtSelectNntpServers)
	{
		while(SQLITE_ROW == mStmtSelectNntpServers.step())
		{
			result.push_back(PrefsNntpServer(
				mStmtSelectNntpServers.getResultInt(0),		// DB record ID
				mStmtSelectNntpServers.getResultText(1),	// user given name (alias)
				mStmtSelectNntpServers.getResultText(2),	// URL for NNTP server
				mStmtSelectNntpServers.getResultText(3),	// NNTP server port
				mStmtSelectNntpServers.getResultText(4),	// user name for NNTP service
				mStmtSelectNntpServers.getResultText(5),	// password for NNTP service
				mStmtSelectNntpServers.getResultInt(6),		// Connection allowed to NNTP server
				(1 == mStmtSelectNntpServers.getResultInt(7)), // Uses SSL connections?
				(1 == mStmtSelectNntpServers.getResultInt(8))  // User enabled/disabled?
			));
		}

		mStmtSelectNntpServers.reset();
	}

	return result;
}

void AppPreferences::updateNntpServer(PrefsNntpServer& nntpServer)
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = mDb.prepareStatement(
		"update NntpServers set name = ?, url = ?, port = ?, user = ?, "
        "password = ?, connCount = ?, useSsl = ?, enabled = ? where id = ?");

	if(stmt)
	{
		// bind the updated values to the statment
		stmt.bindText(1, nntpServer.getName().c_str());
		stmt.bindText(2, nntpServer.getUrl().c_str());
		stmt.bindText(3, nntpServer.getPort().c_str());
		stmt.bindText(4, nntpServer.getUsername().c_str());
		stmt.bindText(5, nntpServer.getPassword().c_str());
		stmt.bindInt(6, nntpServer.getConnectionCount());
		stmt.bindInt(7, nntpServer.useSsl() ? 1 : 0);
		stmt.bindInt(8, nntpServer.isEnabled() ? 1 : 0);
		stmt.bindInt(9, nntpServer.getDbId());

		stmt.step();
		stmt.reset();
	}
}

void AppPreferences::insertNntpServer(PrefsNntpServer& nntpServer)
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = mDb.prepareStatement(
		"insert into NntpServers (name,url,port,user,password,connCount,useSsl,enabled) values(?,?,?,?,?,?,?,?)");

	if(stmt)
	{
		// bind the new data values to the statement
		stmt.bindText(1, nntpServer.getName().c_str());
		stmt.bindText(2, nntpServer.getUrl().c_str());
		stmt.bindText(3, nntpServer.getPort().c_str());
		stmt.bindText(4, (nntpServer.getUsername().empty() ? 0 : nntpServer.getUsername().c_str()));
		stmt.bindText(5, (nntpServer.getPassword().empty() ? 0 : nntpServer.getPassword().c_str()));
		stmt.bindInt(6, nntpServer.getConnectionCount());
		stmt.bindInt(7, nntpServer.useSsl() ? 1 : 0);
		stmt.bindInt(8, nntpServer.isEnabled() ? 1 : 0);

		stmt.step();
		nntpServer.setDbId(mDb.getLastInsertRowId());
		stmt.reset();
	}
}

void AppPreferences::deleteNntpServer(PrefsNntpServer& nntpServer)
throw(Sqlite3::Error)
{
	Sqlite3::Stmt stmt = mDb.prepareStatement("delete from NntpServers where id = ?");

	if(stmt && nntpServer.isDbRecord())
	{
		stmt.bindInt(1, nntpServer.getDbId());
		stmt.step();
		stmt.reset();

		nntpServer.setDbId(-1);
	}
}

std::string AppPreferences::getSysAttrString(int id) const
{
	std::string result;
	Sqlite3::Stmt stmt = mDb.prepareStatement("select data from SystemAttributes where id = ?");
	if(stmt)
	{
		stmt.bindInt(1, id);
		if(SQLITE_ROW == stmt.step())
		{
			int len = stmt.getResultBytes(0);
			if(len > 0)
				result.assign((const char *)stmt.getResultBlob(0), len);
		}
		stmt.reset();
	}
	return result;
}

// TODO: this may need to throw or return a boolean
void AppPreferences::setSysAttrSring(int id, const std::string& value)
{
	Sqlite3::Stmt stmt = mDb.prepareStatement(
		"insert or replace into SystemAttributes (id, name, data) values(?, (select name from SystemAttributes where id = ?), ?)");
	//Sqlite3::Stmt stmt = mDb.prepareStatement("update SystemAttributes set data = ? where id = ?");
	if(stmt)
	{
		// bind the text and the ID values
		stmt.bindInt(1, id);
		stmt.bindInt(2, id);
		stmt.bindBlob(3, value.c_str(), value.size());

		// execute the statement
		stmt.step();
		stmt.reset();
	}
}

AppPreferences& AppPreferences::operator =(AppPreferences&& that)
{
	mDb = std::move(that.mDb);
	mStmtCountNntpServers = std::move(that.mStmtCountNntpServers);
	mStmtSelectNntpServers = std::move(that.mStmtSelectNntpServers);
	return *this;
}

std::string AppPreferences::getTextOption(const char *colName) const
throw(Sqlite3::Error)
{
	char sql[256];
	sprintf(sql, "select %s from SystemOptions where id = (select max(id) from SystemOptions)", colName);
	Sqlite3::Stmt stmt = mDb.prepareStatement(sql);

	std::string result("");
	if(stmt)
	{
		if(SQLITE_ROW == stmt.step())
		{
			const char *pBuf = (const char*)stmt.getResultText(0);
			result.assign(pBuf ? pBuf : "");
		}
		stmt.reset();
	}
	return std::move(result);
}

void AppPreferences::setTextOption(const char *colName, const char *colValue)
throw(Sqlite3::Error)
{
	char sql[256];
	sprintf(sql, "update SystemOptions set %s = ? where id = (select max(id) from SystemOptions)", colName);
	Sqlite3::Stmt stmt = mDb.prepareStatement(sql);

	if(stmt)
	{
		stmt.bindText(1, colValue);
		stmt.step();
		stmt.reset();
	}
}

bool AppPreferences::getBoolOption(const char *colName) const
throw(Sqlite3::Error)
{
	char sql[256];
	sprintf(sql, "select %s from SystemOptions where id = (select max(id) from SystemOptions)", colName);
	Sqlite3::Stmt stmt = mDb.prepareStatement(sql);

	bool result = false;
	if(stmt)
	{
		if(SQLITE_ROW == stmt.step())
			result = (0 != stmt.getResultInt(0));
		stmt.reset();
	}
	return result;
}

void AppPreferences::setBoolOption(const char *colName, bool colValue)
throw(Sqlite3::Error)
{
	char sql[256];
	sprintf(sql, "update SystemOptions set %s = ? where id = (select max(id) from SystemOptions)", colName);
	Sqlite3::Stmt stmt = mDb.prepareStatement(sql);

	if(stmt)
	{
		int bVal = colValue ? 1 : 0;
		stmt.bindInt(1, bVal);
		stmt.step();
		stmt.reset();
	}
}
