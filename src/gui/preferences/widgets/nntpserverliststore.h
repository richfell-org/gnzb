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
#ifndef __NNTP_SERVER_LIST_STORE_HEADER__
#define __NNTP_SERVER_LIST_STORE_HEADER__

#include <gtkmm/liststore.h>

class PrefsNntpServer;

/**
 * 
 * 
 */
class NntpServerListStoreColumns : public Gtk::TreeModelColumnRecord
{
public:

	NntpServerListStoreColumns();
	~NntpServerListStoreColumns();

	Gtk::TreeModelColumn<int>& col_entry_idx() { return m_col_entry_idx; }
	const Gtk::TreeModelColumn<int>& col_entry_idx() const { return m_col_entry_idx; }

	Gtk::TreeModelColumn<bool>& col_enabled() { return m_col_enabled; }
	const Gtk::TreeModelColumn<bool>& col_enabled() const { return m_col_enabled; }

	Gtk::TreeModelColumn<Glib::ustring>& col_name() { return m_col_name; }
	const Gtk::TreeModelColumn<Glib::ustring>& col_name() const { return m_col_name; }

private:

	Gtk::TreeModelColumn<int> m_col_entry_idx;
	Gtk::TreeModelColumn<bool> m_col_enabled;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
};

/**
 * 
 * 
 */
class NntpServerListStore : public Gtk::ListStore
{
// construction/destruction
public:

	NntpServerListStore();
	~NntpServerListStore();

// attributes
public:

	NntpServerListStoreColumns& columns() { return m_cols; }
	const NntpServerListStoreColumns& columns() const { return m_cols; }

// operatations
public:

	Gtk::TreeIter add_server(int entry_idx, PrefsNntpServer& server_entry);

// implementation
protected:

	NntpServerListStoreColumns m_cols;
};


#endif  /* __NNTP_SERVER_LIST_STORE_HEADER__ */