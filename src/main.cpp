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
#include <gtkmm/messagedialog.h>
#include "application.h"
#include "db/sqlite3.h"
#include "nntp/fetch.h"
#include "util/sysutil.h"
#include <iostream>

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

		// configure sqlite3
		sqlite3_config(SQLITE_CONFIG_MULTITHREAD);

		// instantiate an application class and let it run
		GNzbApplication app;
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
