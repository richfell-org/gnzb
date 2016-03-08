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
#ifndef __GNZB_TOOLBAR_HEADER__
#define __GNZB_TOOLBAR_HEADER__

#include <tuple>
#include <vector>
#include <sigc++/sigc++.h>
#include <gtkmm/toolbar.h>

namespace Gio {
	class Action;
}

namespace Gtk {
	class ToolButton;
	class ToggleToolButton;
	class Builder;
}

class GNzbToolButton;

/*
 * 
 * 
 */
class GNzbToolbar : public Gtk::Toolbar
{
// construction
public:

	GNzbToolbar(GtkToolbar *p_object, Glib::RefPtr<Gtk::Builder>& ref_builder);
	GNzbToolbar(GNzbToolbar&&) = delete;
	GNzbToolbar(const GNzbToolbar&) = delete;
	~GNzbToolbar();

// attributes
public:

	// button IDs (separator indicies are skipped)
	enum ButtonId { ADD_FILE, RESUME = 2, PAUSE, CANCEL, DETAILS = 6, SUMMARY, PREFS = 9, ABOUT, BTN_COUNT, };

	Gtk::ToolButton *get_button(ButtonId id);

// operations
public:

	void set_clicked_handler(ButtonId id, const sigc::slot<void>& handler_slot);
	void set_toggled_handler(ButtonId id, const sigc::slot<void>& handler_slot);

	void set_clicked_action(ButtonId id, Glib::RefPtr<Gio::Action> ref_action);
	void set_toggled_action(ButtonId id, Glib::RefPtr<Gio::Action> ref_action);

	void signal_block(ButtonId id);
	void signal_unblock(ButtonId id);
	bool is_signal_blocked(ButtonId id) const;

// implementation
protected:

	void disconnect(ButtonId id);
	void add_separator(int width, bool bDdraw = true);

	bool get_action_state(const Glib::RefPtr<Gio::Action>& ref_action);

	// signal connection items
	enum SigId { SIG_ACTION, SIG_ENABLED, SIG_STATE, };
	std::vector<std::tuple<sigc::connection, sigc::connection, sigc::connection>> m_sig_conns;
};

#endif  /* __GNZB_TOOLBAR_HEADER__ */