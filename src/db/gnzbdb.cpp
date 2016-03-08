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
#include <mutex>
#include <stdexcept>
#include <giomm/file.h>
#include <giomm/resource.h>
#include "sqlitedb.h"

#include <iostream>

std::string get_app_db_filename()
{
	static std::string db_filename("");
	static std::mutex init_mutex;

	std::unique_lock<std::mutex> init_lock(init_mutex);
	if(db_filename.empty())
	{
		db_filename.assign("/home/richfell/.gnzb/gnzb.db");
	}
	init_lock.unlock();

	return db_filename;
}

Sqlite3::Db open_app_db()
{
	return Sqlite3::Db(get_app_db_filename());
}

// resource path for the DB initialization SQL file
static const char *sql_path = "/org/richfell/gnzb/db/gnzb-sys.sql";

/**
 * 
 * 
 */
bool create_default_appdb(const char *appdb_path)
{
	gsize size{0};

	try
	{
		// does the directory exist?
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(appdb_path);
		Glib::RefPtr<Gio::File> dir = file->get_parent();
		if(!dir->query_exists())
			dir->make_directory_with_parents();

		// reference the SQL initialization file
		Glib::RefPtr<const Glib::Bytes> ref_bytes = Gio::Resource::lookup_data_global(sql_path);
		Sqlite3::Db app_db(appdb_path);
		app_db.exec((const char*)ref_bytes->get_data(size));
		app_db.close();
		return true;
	}
	catch(const std::exception& e)
	{
		std::cerr << "create_default_appdb: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "create_default_appdb: unknown error type" << std::endl;
	}

	return false;
}