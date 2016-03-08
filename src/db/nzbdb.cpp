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
#include "nzbdb.h"
#include <thread>
#include <chrono>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

bool NzbDb::does_db_exist(const char *filename)
{
	struct stat dbStat;
	return (0 == stat(filename, &dbStat)) && S_ISREG(dbStat.st_mode);
}

NzbDb::NzbDb()
{
}

NzbDb::NzbDb(const std::string& filename)
throw(Sqlite3::Error)
:   NzbDb(filename.c_str())
{
}

NzbDb::NzbDb(const char *filename)
throw(Sqlite3::Error)
{
	open(filename);
}

NzbDb::~NzbDb()
{
	close();
}

void NzbDb::close()
{
	if(mStmtArticleSize)
		mStmtArticleSize.finalize();
	if(mStmtArticleQuery)
		mStmtArticleQuery.finalize();
	if(mStmtArticleInsert)
		mStmtArticleInsert.finalize();

	Sqlite3::Db::close();
}

unsigned long NzbDb::get_size(NZB::File *p_nzb_file, int *p_segment_count/* = nullptr*/)
throw(Sqlite3::Error)
{
	unsigned long result = 0;

	if(nullptr != p_segment_count) *p_segment_count = int(0);

	if(!mStmtArticleSize)
		mStmtArticleSize = prepareStatement("select article_size from nzb_segment_table where article_id = ?", 63);

	for(int i = 0; i < p_nzb_file->getSegmentCount(); ++i)
	{
		try
		{
			mStmtArticleSize.bindText(1, p_nzb_file->getSegment(i).getMessageId().c_str());

			register int status = mStmtArticleSize.step();
			while((SQLITE_BUSY == status) || (SQLITE_LOCKED == status))
			{
				std::this_thread::sleep_for(std::chrono::microseconds(100));
				status = mStmtArticleSize.step();
			}
			if(SQLITE_ROW == status)
			{
				result += mStmtArticleSize.getResultInt64(0);
				if(nullptr != p_segment_count) ++*p_segment_count;
			}
		}
		catch(const Sqlite3::StmtError& e)
		{
			throw Sqlite3::Error(e.getErrorNumber(), sqlite3_errmsg(mPtrDb.get()));
		}

		mStmtArticleSize.reset();
	}

	return result;
}

void NzbDb::open(const char *filename)
throw(Sqlite3::Error)
{
	bool isNewDb = !does_db_exist(filename);
	Sqlite3::Db::open(filename);

	if(isNewDb) init_schema();
}

bool NzbDb::is_article_present(NZB::Segment *p_segment)
throw(Sqlite3::Error)
{
	bool result = false;

	// prepare the statement if needed
	if(!mStmtArticleQuery)
		mStmtArticleQuery = prepareStatement("select 1 from nzb_segment_table where article_id = ?", 52);

	// bind the variable paramaeter and execute the query
	mStmtArticleQuery.bindText(1, p_segment->getMessageId().c_str());

	try
	{
		// execute the query
		int status = mStmtArticleQuery.step();
		while((SQLITE_BUSY == status) || (SQLITE_LOCKED == status))
		{
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			status = mStmtArticleQuery.step();
		}

		// get the result (if available)
		if(SQLITE_ROW == status)
			result = mStmtArticleQuery.getResultInt(0) == 1;

		mStmtArticleQuery.reset();
	}
	catch(const Sqlite3::StmtError& e)
	{
		mStmtArticleQuery.reset();
		throw Sqlite3::Error(e.getErrorNumber(), sqlite3_errmsg(mPtrDb.get()));
	}

	return result;
}

void NzbDb::insert_article_record(NZB::Segment *p_segment, unsigned long position, unsigned long size)
throw(Sqlite3::Error)
{
	// prepare the statement if needed
	if(!mStmtArticleInsert)
	{
		mStmtArticleInsert = prepareStatement(
			"insert into nzb_segment_table(article_id,segment_num,article_size,file_pos,size) VALUES(?,?,?,?,?)", 98);
	}

	// bind the parameter values
	mStmtArticleInsert.bindText(1, p_segment->getMessageId().c_str(), -1);
	mStmtArticleInsert.bindInt(2, p_segment->getNumber());
	mStmtArticleInsert.bindInt64(3, p_segment->getByteCount());
	mStmtArticleInsert.bindInt64(4, position);
	mStmtArticleInsert.bindInt(5, size);

	try
	{
		// execute the insert
		int status = mStmtArticleInsert.step();
		while((SQLITE_BUSY == status) || (SQLITE_LOCKED == status))
		{
			std::this_thread::sleep_for(std::chrono::microseconds(100));
			status = mStmtArticleInsert.step();
		}

		mStmtArticleInsert.reset();
	}
	catch(const Sqlite3::StmtError& e)
	{
		mStmtArticleInsert.reset();
		throw Sqlite3::Error(e.getErrorNumber(), sqlite3_errmsg(mPtrDb.get()));
	}
}

void NzbDb::init_schema()
throw(Sqlite3::Error)
{
	exec(
		"create table nzb_segment_table("
        "   article_id varchar(128) not null primary key, "
        "   segment_num smallint unsigned not null, "
        "   article_size integer unsigned not null, "
        "   file_pos integer unsigned not null, "
        "   size integer unsigned not null)");
}