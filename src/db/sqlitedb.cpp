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
#include "sqlitedb.h"
#include <chrono>
#include <thread>

namespace Sqlite3 {

Error::Error(int errnum, const char *msg, bool doFreeMsg/* = false*/)
:   std::runtime_error(msg), mErrNum(errnum)
{
	if(doFreeMsg)
		sqlite3_free((void*)msg);
}

Db::Db()
:   mPtrDb(0, sqlite3_close_v2)
{
}

Db::Db(Db&& that)
:   mPtrDb(std::move(that.mPtrDb))
{
}

Db::Db(const std::string& filename, int flags/* = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX*/)
throw(Error)
:   Db(filename.c_str(), flags)
{
}

Db::Db(const char *filename, int flags/* = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX*/)
throw(Error)
:   mPtrDb(0, sqlite3_close_v2)
{
	open(filename, flags);
}

Db::~Db()
{
	close();
}

void Db::open(const char *filename, int flags/* = SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_NOMUTEX*/)
throw(Error)
{
	sqlite3 *pDb = 0;

	// open th DB for the given file name
	register int status = sqlite3_open_v2(filename, &pDb, flags, 0);
	if(SQLITE_OK != status)
		throw Error(status, sqlite3_errmsg(pDb));
	mPtrDb.reset(pDb);
}

void Db::close()
{
	mPtrDb.reset(0);
}

void Db::exec(const char *sql)
throw(Error)
{
	if(!mPtrDb) return;

	char *pErrMsg = 0;
	register int status = sqlite3_exec(mPtrDb.get(), sql, 0, 0, &pErrMsg);
	if(0 != pErrMsg)
		throw Error(status, pErrMsg, true);
}

void Db::exec(const char *sql, int (*resultsCallback)(void*,int,char**,char**), void *pCallbackData)
throw(Error)
{
	if(!mPtrDb) return;

	char *pErrMsg = 0;
	register int status = sqlite3_exec(mPtrDb.get(), sql, resultsCallback, pCallbackData, &pErrMsg);
	if(0 != pErrMsg)
		throw Error(status, pErrMsg, true);
}

long Db::getLastInsertRowId()
{
	return sqlite3_last_insert_rowid(mPtrDb.get());
}

Stmt Db::prepareStatement(const char *sql, int sqlLen/* = -1*/)
throw(Error)
{
	Stmt prepStmt(*this, sql, sqlLen);
	return prepStmt;
}

Db& Db::operator =(Db&& that)
{
	mPtrDb = std::move(that.mPtrDb);
}

Stmt::Stmt()
:   mPtrStmt(0, sqlite3_finalize)
{
}

Stmt::Stmt(Stmt&& that)
:   mPtrStmt(0, sqlite3_finalize)
{
	mPtrStmt = std::move(that.mPtrStmt);
}

Stmt::Stmt(Db& db, const char *sql, int sqlLen/* = -1*/)
throw(Error)
:   mPtrStmt(0, sqlite3_finalize)
{
	prepare(db, sql, sqlLen);
}

Stmt::~Stmt()
{
	finalize();
}

int Stmt::parameterCount() const
{
	int result = 0;
	if(mPtrStmt)
		result = sqlite3_bind_parameter_count(mPtrStmt.get());
	return result;
}

int Stmt::paramaterNumber(const char *name) const
{
	int result = 0;
	if(mPtrStmt)
		result = sqlite3_bind_parameter_index(mPtrStmt.get(), name);
	return result;
}

const char *Stmt::parameterName(int number) const
{
	const char *result = 0;
	if(mPtrStmt)
		result = sqlite3_bind_parameter_name(mPtrStmt.get(), number);
	return result;
}

int Stmt::resultColumnCount() const
{
	int result = 0;
	if(mPtrStmt)
		result = sqlite3_column_count(mPtrStmt.get());
	return result;
}

std::string Stmt::resultColumnDeclType(int index)
{
	std::string result("");
	if(mPtrStmt)
	{
		register const char *type = sqlite3_column_decltype(mPtrStmt.get(), index);
		if(0 != type)
			result.assign(type);
	}
	return result;
}

#ifdef SQLITE_ENABLE_COLUMN_METADATA

std::string Stmt::resultColumnOriginName(int index)
{
	std::string result("");
	if(mPtrStmt)
	{
		register const char *type = sqlite3_column_origin_name(mPtrStmt.get(), index);
		if(0 != type)
			result.assign(type);
	}
	return result;
}

std::string Stmt::resultColumnTableName(int index)
{
	std::string result("");
	if(mPtrStmt)
	{
		register const char *type = sqlite3_column_table_name(mPtrStmt.get(), index);
		if(0 != type)
			result.assign(type);
	}
	return result;
}

std::string Stmt::resultColumnDatabaseName(int index)
{
	std::string result("");
	if(mPtrStmt)
	{
		register const char *type = sqlite3_column_database_name(mPtrStmt.get(), index);
		if(0 != type)
			result.assign(type);
	}
	return result;
}

#endif  /* SQLITE_ENABLE_COLUMN_METADATA */

bool Stmt::busy()
{
	return(0 != sqlite3_stmt_busy(mPtrStmt.get()));
}

bool Stmt::readOnly()
{
	if(mPtrStmt)
		return(0 != sqlite3_stmt_readonly(mPtrStmt.get()));
	return true;
}

void Stmt::prepare(Db& db, const char *sql, int sqlLen/* = -1*/)
throw(Error)
{
	sqlite3_stmt *pStmt = 0;
	register int status = sqlite3_prepare_v2(db.getSqlite3(), sql, sqlLen, &pStmt, 0);
	while((SQLITE_BUSY == status) || (SQLITE_LOCKED == status))
	{
		std::this_thread::sleep_for(std::chrono::microseconds(100));
		status = sqlite3_prepare_v2(db.getSqlite3(), sql, sqlLen, &pStmt, 0);
	}

	if(SQLITE_OK != status)
		throw Error(status, sqlite3_errmsg(db.getSqlite3()));

	mPtrStmt.reset(pStmt);
}

void Stmt::bindBlob(int paramNum, const void *blob, int numBytes)
throw(StmtError)
{
	register int status = sqlite3_bind_blob(mPtrStmt.get(), paramNum, blob, numBytes, 0);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindDouble(int paramNum, double val)
throw(StmtError)
{
	register int status = sqlite3_bind_double(mPtrStmt.get(), paramNum, val);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindInt(int paramNum, int val)
throw(StmtError)
{
	register int status = sqlite3_bind_int(mPtrStmt.get(), paramNum, val);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindInt64(int paramNum, sqlite3_int64 val)
throw(StmtError)
{
	register int status = sqlite3_bind_int64(mPtrStmt.get(), paramNum, val);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindNull(int paramNum)
throw(StmtError)
{
	register int status = sqlite3_bind_null(mPtrStmt.get(), paramNum);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindText(int paramNum, const char *text, int numBytes)
throw(StmtError)
{
	register int status = sqlite3_bind_text(mPtrStmt.get(), paramNum, text, numBytes, SQLITE_STATIC);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindText16(int paramNum, const void *text16, int numBytes)
throw(StmtError)
{
	register int status = sqlite3_bind_text16(mPtrStmt.get(), paramNum, text16, numBytes, 0);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindValue(int paramNum, const sqlite3_value *pValue)
throw(StmtError)
{
	register int status = sqlite3_bind_value(mPtrStmt.get(), paramNum, pValue);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::bindZeroBlob(int paramNum, int blobLength)
throw(StmtError)
{
	register int status = sqlite3_bind_zeroblob(mPtrStmt.get(), paramNum, blobLength);
	if(SQLITE_OK != status)
		throw StmtError(status);
}

int Stmt::step()
throw(StmtError)
{
	register int status = sqlite3_step(mPtrStmt.get());
	//while((SQLITE_BUSY == status) || (SQLITE_LOCKED == status))
	//{
	//	std::this_thread::sleep_for(std::chrono::microseconds(100));
	//	status = sqlite3_step(mPtrStmt.get());
	//}

	//if((SQLITE_ROW != status) && (SQLITE_DONE != status))
	//	throw StmtError(status);

	// don't thow an exception if DB busy or DB table locked just
	// return that vlaue and let the caller decide what is appropiate
	switch(status)
	{
		case SQLITE_ROW:
		case SQLITE_DONE:
		case SQLITE_BUSY:
		case SQLITE_LOCKED:
			break;
		default:
			throw StmtError(status);
	}

	return status;
}

void Stmt::reset()
throw(StmtError)
{
	register int status = sqlite3_reset(mPtrStmt.get());
	if(SQLITE_OK != status)
		throw StmtError(status);
}

void Stmt::finalize()
{
	if(mPtrStmt)
		mPtrStmt.reset(0);
}

int Stmt::getResultType(int colIndex) const
{
	return sqlite3_column_type(mPtrStmt.get(), colIndex);
}

const void *Stmt::getResultBlob(int colIndex) const
{
	return sqlite3_column_blob(mPtrStmt.get(), colIndex);
}

int Stmt::getResultBytes(int colIndex) const
{
	return sqlite3_column_bytes(mPtrStmt.get(), colIndex);
}

int Stmt::getResultBytes16(int colIndex) const
{
	return sqlite3_column_bytes16(mPtrStmt.get(), colIndex);
}

int Stmt::getResultInt(int colIndex) const
{
	return sqlite3_column_int(mPtrStmt.get(), colIndex);
}

sqlite3_int64 Stmt::getResultInt64(int colIndex) const
{
	return sqlite3_column_int64(mPtrStmt.get(), colIndex);
}

double Stmt::getResultDouble(int colIndex) const
{
	return sqlite3_column_double(mPtrStmt.get(), colIndex);
}

const unsigned char *Stmt::getResultText(int colIndex) const
{
	return sqlite3_column_text(mPtrStmt.get(), colIndex);
}

const void *Stmt::getResultText16(int colIndex) const
{
	return sqlite3_column_text16(mPtrStmt.get(), colIndex);
}

sqlite3_value *Stmt::getResultValue(int colIndex) const
{
	return sqlite3_column_value(mPtrStmt.get(), colIndex);
}

Stmt& Stmt::operator =(Stmt&& other)
{
	mPtrStmt = std::move(other.mPtrStmt);
	return *this;
}

std::string Stmt::getResultString(int colIndex) const
{
	std::string result("");

	register const unsigned char *text = getResultText(colIndex);
	if(0 != text)
		result.assign((const char*)text);
	return result;
}

std::u16string Stmt::getResultString16(int colIndex) const
{
	std::u16string result;

	register const void *text = getResultText16(colIndex);
	if(0 != text)
		result.assign((char16_t*)text);
	return result;
}

}   /* namespace Sqlite3 */