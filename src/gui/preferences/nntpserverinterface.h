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
#ifndef __NNTP_SERVER_INTERFACE_HEADER__
#define __NNTP_SERVER_INTERFACE_HEADER__

#include <vector>
#include <thread>
#include <future>
#include <sigc++/connection.h>
#include <glibmm/dispatcher.h>
#include "preferencespage.h"
#include "../../db/preferences.h"

struct _GdkEventKey;
struct _GdkEventFocus;

namespace Glib {
	template <typename T> class RefPtr;
	class ustring;
}
namespace Gtk {
	class TreeView;
	class Entry;
	class Image;
	class Button;
	class ToggleButton;
	class Widget;
}

class AppPreferences;

/**
 * 
 * 
 */
class NntpServerInterface : public PreferencesPage
{
public:

	NntpServerInterface();
	~NntpServerInterface();

// operations
public:

	void init(const AppPreferences& app_prefs, Glib::RefPtr<Gtk::Builder>& ref_builder);
	void save(AppPreferences& app_prefs);

// implementation
protected:

	// signal handlers
	void on_name_edited(const Glib::ustring& path, const Glib::ustring& new_text);
	void on_enable_toggled(const Glib::ustring& path);
	void on_option_changed(Gtk::ToggleButton *pToggle);
	bool on_key_release(_GdkEventKey* p_event, Gtk::Widget *pWidget);
	void on_server_selection();
	void on_remove();
	void on_add();

	enum class EditState { CLEAN, DIRTY, NEW, DELETED, };

	// PrefsNntpServer instances and their associated edit state
	std::vector<PrefsNntpServer> m_servers;
	std::vector<EditState> m_server_editstate;

	// UI widgets
	Gtk::TreeView *p_server_treeview;
	Gtk::Entry *p_entry_url, *p_entry_port, *p_entry_username, *p_entry_password, *p_entry_conncount;
	Gtk::Entry *p_entry_retention, *p_entry_volume, *p_entry_used_volume;
	Gtk::Image *p_image_status;
	Gtk::ToggleButton *p_toggle_usessl, *p_toggle_volume_warn, *p_toggle_volume_reset_monthly;
	Gtk::Button *p_btn_addserver, *p_btn_removeserver, *p_btn_volume_reset;

	// timeout callback items for NNTP setting check
	Glib::Dispatcher m_check_dispatch;
	std::future<bool> m_server_check_future;
	std::thread m_server_check_thread;
	sigc::connection m_to_conn;
	PrefsNntpServer const * get_selected_server();
	void on_check_complete();
	bool on_settings_changed_timeout();
};

#endif  /* __NNTP_SERVER_INTERFACE_HEADER__ */