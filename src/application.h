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
#ifndef __GNZB_APPLICATION_HEADER__
#define __GNZB_APPLICATION_HEADER__

#include <memory>
#include <gtkmm/application.h>
#include "gui/guignzb.h"

namespace Gtk {
	class TreePath;
}

class GNzb;
class GNzbApplicationWindow;

/**
 * The application class.  Handles application registration,
 * creating the main window and responding to shell events
 *
 * The application registers a a unique instance and any
 * attempts to open NZB files are handled by the one running
 * instance.
 */
class GNzbApplication : public Gtk::Application
{
// construction/destruction
public:

	GNzbApplication();
	~GNzbApplication();

// attributes
public:

	GNzbApplicationWindow* get_main_window() { return m_ptr_mainwin.get(); }

// operations
public:

	// application instance access
	static Glib::RefPtr<GNzbApplication> get_instance();

	// file system operations
	bool chdir_to_output(const GNzb& gnzb);
	void move_completed(GNzb& gnzb);

	// GNzb download
	bool is_download_active();
	bool start_download(std::shared_ptr<GNzb>& ptr_gnzb);
	void pause_download();

// implementation
protected:

	// overrides
	void on_startup();
	void on_activate();
	void on_open(const type_vec_files& files, const Glib::ustring& hint);

	// action handlers
	void on_help_about();
	void on_preferences();
	void on_quit();

	// signal handlers
	void on_gnzb_action(const Gtk::TreePath& path, GNzbAction action);
	bool on_download_progress();

	// initialization
	void create_actions();
	void create_app_menu();
	void create_menubar();
	void allocate_main_window();

	// main window
	std::unique_ptr<GNzbApplicationWindow> m_ptr_mainwin;

	// GNzb download methods
	bool start_next_download();
	void on_download_complete();

	// GNzb download state
	std::shared_ptr<GNzb> m_ptr_active_gnzb;
	unsigned long m_last_update_seconds{0};
	sigc::connection m_idle_connection;
};

#endif	/* __GNZB_APPLICATION_HEADER__ */
