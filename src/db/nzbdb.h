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
#ifndef __NZB_DB_HEADER__
#define __NZB_DB_HEADER__

#include <stdexcept>
#include <string>
#include "sqlitedb.h"
#include <libusenet/nzb.h>

/*
 * 
 */
class NzbDb : public Sqlite3::Db
{
// construction
public:

	NzbDb();
	NzbDb(const std::string& filename) throw(Sqlite3::Error);
	NzbDb(const char *filename) throw(Sqlite3::Error);
	~NzbDb();

// attributes
public:

// operations
public:

	// check for the existence of the named file
	static bool does_db_exist(const char *filename);

	void open(const char *filename) throw(Sqlite3::Error);
	void close();

	unsigned long get_size(NZB::File *p_nzb_file, int *p_segment_count = nullptr) throw(Sqlite3::Error);
	bool is_article_present(NZB::Segment *p_segment) throw(Sqlite3::Error);
	void insert_article_record(NZB::Segment *p_segment, unsigned long position, unsigned long size) throw(Sqlite3::Error);

// implementation
protected:

	void init(const char *filename) throw(Sqlite3::Error);
	void init_schema() throw(Sqlite3::Error);

	Sqlite3::Stmt mStmtArticleSize, mStmtArticleQuery, mStmtArticleInsert;
};

#endif  /* __NZB_DB_HEADER__ */