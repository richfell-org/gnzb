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
#ifndef __APP_NZB_GROUPS_MENU__
#define __APP_NZB_GROUPS_MENU__

#include <memory>
#include <vector>
#include <tuple>
#include <sigc++/sigc++.h>
#include <gtkmm/menu.h>
#include <gtkmm/checkmenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/treepath.h>
#include "../../db/gnzbgroups.h"

class GNzb;

/*
 * 
 */
class GNzbGroupsMenu : public Gtk::Menu
{
// construction
public:

	GNzbGroupsMenu();
	~GNzbGroupsMenu();

// attributes
public:

	// the GNzb* which should have it's AppNzbGroup adjusted on selection of a menu item
	// this value is passed to the signal handler invoked on menu item selection
	Gtk::TreePath& get_target() { return m_target; }
	const Gtk::TreePath& get_target() const { return m_target; }
	void set_target(Gtk::TreePath&& target) { m_target = std::move(target); }
	void set_target(const Gtk::TreePath& target) { m_target = target; }

	// emitted when a item is selected form the menu.
	// signal prototype: void on_group_selected(NZB pTargetNzb, int iGroup)
	// the iGroup value is the index of the NZB group from the vector
	// passed to resetAppNzbGroupItems
	sigc::signal<void, Gtk::TreePath, int>& signal_group_selected() { return m_sig_group_selected; }

// operations
public:

	// set the signal handler for the "Configure..." menu item
	void setConfigureSlot(sigc::slot<void> callback);

	// set the NZB group items that should show in the menu
	void resetAppNzbGroupItems(std::vector<GNzbGroup> groups);

	// set a menu item active
	void setActiveGroup(GNzbGroup& group);

	// "run" the popup menu
	void popup(guint32 eTime, int x, int y);

// implementation
private:

	// signal emitted from menu item when a group is seleted form the menu
	sigc::signal<void, Gtk::TreePath, int> m_sig_group_selected;

	void onMenuItemSelected(int i_item);

	// menu item constants...the NZB groups may change (on user edit) but these
	// items will always be in the groups popup menu
	Gtk::SeparatorMenuItem m_separator;
	Gtk::MenuItem m_configure_item;

	// the current set of AppNzbGroup/CheckMenuItem that are in the menu 
	std::vector<std::tuple<GNzbGroup, std::unique_ptr<Gtk::CheckMenuItem>>> m_group_items;

	// The "target" of the selection of the menu...should be set before poping up
	// the menu to the user.  This is sent in via the item selected signal
	Gtk::TreePath m_target;
};

#endif  /* __APP_NZB_GROUPS_MENU__ */