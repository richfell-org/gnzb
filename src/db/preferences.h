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
#ifndef __PREFERENCES_HEADER__
#define __PREFERENCES_HEADER__

#include <vector>
#include <string>
#include "sqlitedb.h"

class PrefsNntpServer;
class AppPreferences;

/*
 * Represents a record in the databse for an NNTP server.
 */
class PrefsNntpServer
{
// construction
public:

	PrefsNntpServer();
	PrefsNntpServer(
    	const char* name,
	    const char* url,
	    const char* port,
        const char* user,
        const char* passwd,
        int connCount,
		bool usesSsl,
        bool isEnabled);
	~PrefsNntpServer();

// attributes
public:

	int getDbId() const { return mRecId; }

	const std::string& getName() const { return mName; }
	void setName(const char *name) { mName.assign(name); }

	const std::string& getUrl() const { return mUrl; }
	void setUrl(const char *url) { mUrl.assign(url); }

	const std::string& getPort() const { return mPort; }
	void setPort(const char *port) { mPort.assign(port); }

	const std::string& getUsername() const { return mUser; }
	void setUsername(const char *username) { mUser.assign(username); }

	const std::string& getPassword() const { return mPass; }
	void setPassword(const char *password) { mPass.assign(password); }

	int getConnectionCount() const { return mConnCount; }
	void setConnectionCount(int count) { mConnCount = count; }

	bool useSsl() const { return mUseSsl; }
	void setSsl(bool bSsl) { mUseSsl = bSsl; }

	bool isEnabled() const { return mEnabled; }
	void setEnabled(bool bEnabled) { mEnabled = bEnabled; }

// operations
public:

	bool isDbRecord() const { return 0 < mRecId; }
	bool operator ==(const PrefsNntpServer& that) const;
	bool operator !=(const PrefsNntpServer& that) const { return !operator ==(that); }

// implementation
private:

	friend class AppPreferences;

	PrefsNntpServer(
		int dbRecId,
    	const unsigned char* name,
	    const unsigned char* url,
	    const unsigned char* port,
        const unsigned char* user,
        const unsigned char* passwd,
        int connCount,
		bool usesSsl,
        bool isEnabled);

	void setDbId(int id) { mRecId = id; }

	int mRecId;
	std::string mName;
	std::string mUrl;
	std::string mPort;
	std::string mUser;
	std::string mPass;
	int mConnCount;
	bool mUseSsl;
	bool mEnabled;
};

/*
 * 
 */
class AppPreferences
{
public:

	AppPreferences();
	AppPreferences(const AppPreferences&) = delete;
	AppPreferences(AppPreferences&& that);
	~AppPreferences();

public:

	int getNntpServerCount() throw(Sqlite3::Error);
	int getNntpServers(std::vector<PrefsNntpServer>& entries) const throw(Sqlite3::Error);
	std::vector<PrefsNntpServer> get_nntp_servers() const throw(Sqlite3::Error);
	void updateNntpServer(PrefsNntpServer& nntpServer) throw(Sqlite3::Error);
	void insertNntpServer(PrefsNntpServer& nntpServer) throw(Sqlite3::Error);
	void deleteNntpServer(PrefsNntpServer& nntpServer) throw(Sqlite3::Error);

	std::string getDownloadDir() const throw(Sqlite3::Error) { return getTextOption("downloadDir"); }
	void setDownloadDir(const char *dir) throw(Sqlite3::Error) { setTextOption("downloadDir", dir); }

	bool getAlwayAskForDir() const throw(Sqlite3::Error) { return getBoolOption("doAlwaysAskForDir"); }
	void setAlwayAskForDir(bool doAsk) throw(Sqlite3::Error) { setBoolOption("doAlwaysAskForDir", doAsk); }

	bool getDoMoveToDir() const throw(Sqlite3::Error) { return getBoolOption("doMoveToDir"); }
	void setDoMoveToDir(bool doMove) throw(Sqlite3::Error) { setBoolOption("doMoveToDir", doMove); }

	std::string getMoveToDir() const throw(Sqlite3::Error) { return getTextOption("moveToDir"); }
	void setMoveToDir(const char *dir) throw(Sqlite3::Error) { setTextOption("moveToDir", dir); }

	bool getShowNotifications() const throw(Sqlite3::Error) { return getBoolOption("showNotifications"); }
	void setShowNotifications(bool doShow) throw(Sqlite3::Error) { setBoolOption("showNotifications", doShow); }

	bool getSoundOnNzbFinish() const throw(Sqlite3::Error) { return getBoolOption("soundOnNzbFinish"); }
	void setSoundOnNzbFinish(bool doSound) throw(Sqlite3::Error) { setBoolOption("soundOnNzbFinish", doSound); }

	bool getSoundOnQueueFinish() const throw(Sqlite3::Error) { return getBoolOption("soundOnQueueFinish"); }
	void setSoundOnQueueFinish(bool doSound) throw(Sqlite3::Error) { setBoolOption("soundOnQueueFinish", doSound); }

	std::string getNzbFinishSound() const throw(Sqlite3::Error) { return getTextOption("nzbFinishSound"); }
	void setNzbFinishSound(const char *path) throw(Sqlite3::Error) { setTextOption("nzbFinishSound", path); }

	std::string getQueueFinishSound() const throw(Sqlite3::Error) { return getTextOption("queueFinishSound"); }
	void setQueueFinishSound(const char *path) throw(Sqlite3::Error) { setTextOption("queueFinishSound", path); }

	bool getScriptingEnabled() const throw(Sqlite3::Error) { return getBoolOption("enableScripting"); }
	void setScriptingEnabled(bool enableScripting) throw(Sqlite3::Error) { setBoolOption("enableScripting", enableScripting); }

	bool getRunScriptOnNzbAdded() const throw(Sqlite3::Error) { return getBoolOption("runScriptOnNzbAdded"); }
	void setRunScriptOnNzbAdded(bool doScript) throw(Sqlite3::Error) { setBoolOption("runScriptOnNzbAdded", doScript); }

	bool getRunScriptOnNzbFinished() const throw(Sqlite3::Error) { return getBoolOption("runScriptOnNzbFinished"); }
	void setRunScriptOnNzbFinished(bool doScript) throw(Sqlite3::Error) { setBoolOption("runScriptOnNzbFinished", doScript); }

	bool getRunScriptOnNzbCancelled() const throw(Sqlite3::Error) { return getBoolOption("runScriptOnNzbCancelled"); }
	void setRunScriptOnNzbCancelled(bool doScript) throw(Sqlite3::Error) { setBoolOption("runScriptOnNzbCancelled", doScript); }

	std::string getNzbAddedScript() const throw(Sqlite3::Error) { return getTextOption("scriptOnNzbAdded"); }
	void setNzbAddedScript(const char *path) throw(Sqlite3::Error) { setTextOption("scriptOnNzbAdded", path); }

	std::string getNzbFinishedScript() const throw(Sqlite3::Error) { return getTextOption("scriptOnNzbFinished"); }
	void setNzbFinishedScript(const char *path) throw(Sqlite3::Error) { setTextOption("scriptOnNzbFinished", path); }

	std::string getNzbCancelledScript() const throw(Sqlite3::Error) { return getTextOption("scriptOnNzbCancelled"); }
	void setNzbCancelledScript(const char *path) throw(Sqlite3::Error) { setTextOption("scriptOnNzbCancelled", path); }

	std::string getSysAttrString(int id) const;
	void setSysAttrSring(int id, const std::string& value);

	AppPreferences& operator =(const AppPreferences&) = delete;
	AppPreferences& operator =(AppPreferences&& that);

// implementation
private:

	std::string getTextOption(const char *colName) const throw(Sqlite3::Error);
	void setTextOption(const char *colName, const char *colValue) throw(Sqlite3::Error);

	bool getBoolOption(const char *colName) const throw(Sqlite3::Error);
	void setBoolOption(const char *colName, bool colValue) throw(Sqlite3::Error);

	mutable Sqlite3::Db mDb;
	mutable Sqlite3::Stmt mStmtCountNntpServers;
	mutable Sqlite3::Stmt mStmtSelectNntpServers;
};

#endif  /* __PREFERENCES_HEADER__ */