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
#include <exception>
#include <string>
#include <glibmm/thread.h>
#include <glibmm/error.h>
#include <gstreamermm/init.h>
#include <gtkmm/messagedialog.h>
#include "application.h"
#include "runtimesettings.h"
#include "db/sqlite3.h"
#include "db/gnzbdb.h"
#include "db/preferences.h"
#include "nntp/fetch.h"
#include "util/sysutil.h"
#include <giomm/contenttype.h>
#include <iostream>

bool create_default_appdb(const char *appdb_path);

/**
 * 
 */
static bool startup_check()
{
	std::string db_filename = get_app_db_filename();
	if(!does_file_exist(db_filename.c_str()))
		if(!create_default_appdb(db_filename.c_str()))
			return false;

	return true;
}

static void init_settings()
{
	try
	{
		AppPreferences prefs;
		RuntimeSettings::locations().load(prefs);
		RuntimeSettings::notifications().load(prefs);
		RuntimeSettings::scripting().load(prefs);
	}
	catch(const Sqlite3::Error& e)
	{
		std::cerr << "init_settings: " << e.what() << std::endl;
	}
}

static void init_fetch_pools()
{
	try
	{
		AppPreferences prefs;
		NntpFetch::update_server_pools(prefs.get_nntp_servers());
	}
	catch(const Sqlite3::Error& e)
	{
		std::cerr << "init_fetch_pools: " << e.what() << std::endl;
	}
}

/**
 * 
 * 
 */
int main(int argc, char *argv[])
{
	try
	{
		// library init
		if(!Glib::thread_supported())
			Glib::thread_init();
		Gst::init_check();

		// configure sqlite3
		sqlite3_config(SQLITE_CONFIG_MULTITHREAD);

		// create the app DB if needed
		if(!startup_check())
			return 1;

		// load the settings
		init_settings();

		// instantiate an application class and let it run
		GNzbApplication app;

		init_fetch_pools();

		int result = app.run(argc, argv);

		NntpFetch::stop_server_pools();

		return result;
	}
	catch(const Glib::Error& e)
	{
		NntpFetch::stop_server_pools();
		std::cerr << argv[0] << ": error - " << e.what() << std::endl;
		Gtk::MessageDialog err_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		err_dialog.run();
	}
	catch(const std::exception& e)
	{
		NntpFetch::stop_server_pools();
		std::cerr << argv[0] << ": error - " << e.what() << std::endl;
		Gtk::MessageDialog err_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		err_dialog.run();
	}
	catch(...)
	{
		NntpFetch::stop_server_pools();
		std::cerr << argv[0] << ": unknown error" << std::endl;
		Gtk::MessageDialog err_dialog("Unknown error", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
		err_dialog.run();
	}

	// return from here means we encountered an exception
	return 1;
}
