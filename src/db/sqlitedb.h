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
#ifndef __SQLITE_DB_HEADER__
#define __SQLITE_DB_HEADER__

#include <memory>
#include <functional>
#include <stdexcept>
#include <string>
#include "sqlite3.h"

namespace Sqlite3 {

class Db;
class Stmt;
	
/*
 *
 */
class Error : public std::runtime_error
{
public:

	Error(int errnum, const char *msg, bool doFreeMsg = false);
	~Error() throw() {}

	int getErrNum() const throw() { return mErrNum; }

protected:

	int mErrNum;
};

/*
 * 
 */
class Db
{
// construction
public:

	Db();
	Db(Db&& that);
	Db(const Db&) = delete;
	Db(const std::string& filename, int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX)
	throw(Error);
	Db(const char *filename, int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX)
	throw(Error);

	virtual ~Db();

// attributes
public:

	bool isOpen() const { return bool(mPtrDb); }

	sqlite3* getSqlite3() { return mPtrDb.get(); }

// operations
public:

	virtual void open(const char *filename, int flags = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX)
	throw(Error);
	virtual void close();

	void exec(const char *sql) throw(Error);
	void exec(const char *sql, int (*resultsCallback)(void*,int,char**,char**), void *pCallbackData) throw(Error);
	long getLastInsertRowId();

	Stmt prepareStatement(const char *sql, int sqlLen = -1) throw(Error);

	template<typename ... OpArgs>
	void configure(int op, OpArgs&& ... args) throw(Error);

	operator bool() const { return bool(mPtrDb); }

	Db& operator =(Db&& that);
	Db& operator =(const Db&) = delete;

// implementation
protected:

	std::unique_ptr<sqlite3, int(*)(sqlite3*)> mPtrDb;
};

class StmtError : public std::exception
{
public:

	StmtError(int errnum) : std::exception(), mErrNum(errnum) {}
	~StmtError() {}

	int getErrorNumber() const { return mErrNum; }
	const char *what() const throw() { return "Sqlite3 statement error."; }

private:

	int mErrNum;
};

/*
 * 
 */
class Stmt
{
// construction
public:

	Stmt();
	Stmt(Stmt&& that);
	Stmt(const Stmt&) = delete;
	~Stmt();

// attributes
public:

	// parameter info
	int parameterCount() const;
	int paramaterNumber(const char *name) const;
	const char *parameterName(int number) const;

	// results info
	int resultColumnCount() const;
	std::string resultColumnDeclType(int index);
	
#ifdef SQLITE_ENABLE_COLUMN_METADATA
	std::string resultColumnOriginName(int index);
	std::string resultColumnTableName(int index);
	std::string resultColumnDatabaseName(int index);
#endif

	// state info
	bool busy();
	bool readOnly();	

// operations
public:

	void bindBlob(int paramNum, const void *blob, int numBytes) throw(StmtError);
	void bindDouble(int paramNum, double val) throw(StmtError);
	void bindInt(int paramNum, int val) throw(StmtError);
	void bindInt64(int paramNum, sqlite3_int64 val) throw(StmtError);
	void bindNull(int paramNum) throw(StmtError);
	void bindText(int paramNum, const char *text, int numBytes = -1) throw(StmtError);
	void bindText16(int paramNum, const void *text16, int numBytes) throw(StmtError);
	void bindValue(int paramNum, const sqlite3_value *pValue) throw(StmtError);
	void bindZeroBlob(int paramNum, int blobLength) throw(StmtError);

	int step() throw(StmtError);
	void reset() throw(StmtError);
	void finalize();

	// get values from result columns
	int getResultType(int colIndex) const;
	const void *getResultBlob(int colIndex) const;
	int getResultBytes(int colIndex) const;
	int getResultBytes16(int colIndex) const;
	int getResultInt(int colIndex) const;
	sqlite3_int64 getResultInt64(int colIndex) const;
	double getResultDouble(int colIndex) const;
	const unsigned char *getResultText(int colIndex) const;
	const void *getResultText16(int colIndex) const;
	sqlite3_value *getResultValue(int colIndex) const;

	std::string getResultString(int colIndex) const;
	std::u16string getResultString16(int colIndex) const;

	operator bool() const { return bool(mPtrStmt); }
	Stmt& operator =(const Stmt& other) = delete;
	Stmt& operator =(Stmt&& other);

// implementation
private:

	friend class Db;

	Stmt(Db& db, const char *sql, int sqlLen = -1) throw(Error);

	void prepare(Db& db, const char *sql, int sqlLen = -1) throw(Error);

	// pointer to the sqlite3 statement structure
	std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)> mPtrStmt;
};

template<typename ... OpArgs>
inline void Db::configure(int op, OpArgs&& ... args)
throw(Error)
{
	if(mPtrDb)
	{
		register int status = sqlite3_db_config(mPtrDb.get(), op, std::forward<OpArgs>(args)...);
		if(SQLITE_OK != status)
			throw Error(status, sqlite3_errmsg(mPtrDb.get()));
	}
}

}   /* namespace Sqlite3 */

#endif  /* __SQLITE_DB_HEADER__ */