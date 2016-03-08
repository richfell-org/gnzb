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
#ifndef __GROUPS_LIST_STORE_HEADER__
#define __GROUPS_LIST_STORE_HEADER__

#include <gdkmm/rgba.h>
#include <gtkmm/treeiter.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/liststore.h>
#include "../../db/gnzbgroups.h"

/**
 * Columns for the GNzbGroup list store.
 */
class GroupsListStoreColumns : public Gtk::TreeModelColumnRecord
{
public:

	GroupsListStoreColumns();
	~GroupsListStoreColumns() {}

	Gtk::TreeModelColumn<int>& col_entry_idx() { return m_col_entry_idx; }
	const Gtk::TreeModelColumn<int>& col_entry_idx() const { return m_col_entry_idx; }

	Gtk::TreeModelColumn<Gdk::RGBA>& col_color() { return m_col_color; }
	const Gtk::TreeModelColumn<Gdk::RGBA>& col_color() const { return m_col_color; }

	Gtk::TreeModelColumn<Glib::ustring>& col_name() { return m_col_name; }
	const Gtk::TreeModelColumn<Glib::ustring>& col_name() const { return m_col_name; }

private:

	Gtk::TreeModelColumn<int> m_col_entry_idx;
	Gtk::TreeModelColumn<Gdk::RGBA> m_col_color;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
};

/*
 * ListStore for GNzbGroup
 */
class GroupsListStore : public Gtk::ListStore
{
// construction/destruction
public:

	GroupsListStore();
	~GroupsListStore() {}

// attributes
public:

	GroupsListStoreColumns& columns() { return m_cols; }
	const GroupsListStoreColumns& columns() const { return m_cols; }

// operations
public:

	Gtk::TreeIter add_group(int entry_idx, GNzbGroup& group);

// implementation
private:

	GroupsListStoreColumns m_cols;
};

#endif  /* __GROUPS_LIST_STORE_HEADER__ */