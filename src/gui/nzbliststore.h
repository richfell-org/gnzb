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
#ifndef __NZB_LISTSTORE_HEADER__
#define __NZB_LISTSTORE_HEADER__

#include <memory>
#include <glibmm/ustring.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include "guignzb.h"

class GNzb;

/**
 * 
 */
class NzbTreeModelColumnRecord : public Gtk::TreeModelColumnRecord
{
public:

	NzbTreeModelColumnRecord();
	~NzbTreeModelColumnRecord();

// attributes
public:

	Gtk::TreeModelColumn<std::shared_ptr<GNzb>>& gnzb() { return m_col_nzb; }
	const Gtk::TreeModelColumn<std::shared_ptr<GNzb>>& gnzb() const { return m_col_nzb; }

	Gtk::TreeModelColumn<Glib::ustring>& name() { return m_col_name; }
	const Gtk::TreeModelColumn<Glib::ustring>& name() const { return m_col_name; }

	Gtk::TreeModelColumn<Glib::ustring>& group_color_text() { return m_col_group_color_text; }
	const Gtk::TreeModelColumn<Glib::ustring>& group_color_text() const { return m_col_group_color_text; }

	Gtk::TreeModelColumn<GNzbAction>& action1() { return m_col_action1; }
	const Gtk::TreeModelColumn<GNzbAction>& action1() const { return m_col_action1; }

	Gtk::TreeModelColumn<GNzbAction>& action2() { return m_col_action2; }
	const Gtk::TreeModelColumn<GNzbAction>& action2() const { return m_col_action2; }

	Gtk::TreeModelColumn<bool>& action1_pressed() { return m_col_action1_pressed; }
	const Gtk::TreeModelColumn<bool>& action1_pressed() const { return m_col_action1_pressed; }

	Gtk::TreeModelColumn<bool>& action2_pressed() { return m_col_action2_pressed; }
	const Gtk::TreeModelColumn<bool>& action2_pressed() const { return m_col_action2_pressed; }

// implementation
private:

	Gtk::TreeModelColumn<std::shared_ptr<GNzb>> m_col_nzb;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
	Gtk::TreeModelColumn<Glib::ustring> m_col_group_color_text;
	Gtk::TreeModelColumn<GNzbAction> m_col_action1;
	Gtk::TreeModelColumn<GNzbAction> m_col_action2;
	Gtk::TreeModelColumn<bool> m_col_action1_pressed;
	Gtk::TreeModelColumn<bool> m_col_action2_pressed;
};

/**
 *
 * 
 */
class NzbListStore : public Gtk::ListStore
{
public:

	NzbListStore();
	~NzbListStore();

// attributes
public:

	// access to the columns
	const NzbTreeModelColumnRecord& columns() const { return m_cols; }

// operations
public:

	// insertion
	Gtk::TreeIter insert(const Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb);
	Gtk::TreeIter insert_after(const Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb);
	Gtk::TreeIter prepend(const std::shared_ptr<GNzb>& ptr_gnzb);
	Gtk::TreeIter append(const std::shared_ptr<GNzb>& ptr_gnzb);

	// search
	Gtk::TreeIter find_gnzb(const GNzb *p_gnzb) const;

// implementation
private:

	// sets the GNzb values for a row
	void set_values(Gtk::TreeIter& iter, const std::shared_ptr<GNzb>& ptr_gnzb);

	NzbTreeModelColumnRecord m_cols;
};

#endif  /* __NZB_LISTSTORE_HEADER__ */