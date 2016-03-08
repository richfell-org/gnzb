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
#ifndef __GNZB_DB_HEADER__
#define __GNZB_DB_HEADER__

#include <string>
#include "sqlitedb.h"

/* enums for ID column in the SystemAttrubutes table */
enum SysAttrs { SA_WINSTATE = 1, SA_PYMODULE, };

/* the path to the application DB */
std::string get_app_db_filename();

/* open a connection to the application DB */
Sqlite3::Db open_app_db();

#endif  /* __GNZB_DB_HEADER__ */