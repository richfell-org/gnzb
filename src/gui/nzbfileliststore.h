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
#ifndef __NZB_FILE_LISTSTORE_HEADER__
#define __NZB_FILE_LISTSTORE_HEADER__

#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/liststore.h>
#include <libusenet/binParts.h>
#include "guignzb.h"
#include "../gnzb.h"

namespace Gdk {
	class Pixbuf;
}

/**
 * 
 * 
 */
class NzbFileListStoreColumnRecord : public Gtk::TreeModelColumnRecord
{
// construction/destruction
public:

	NzbFileListStoreColumnRecord();
	~NzbFileListStoreColumnRecord();

// attributes
public:

	Gtk::TreeModelColumn<NzbFileTuple>& col_file_tuple() { return m_col_file_tuple; }
	const Gtk::TreeModelColumn<NzbFileTuple>& col_file_tuple() const { return m_col_file_tuple; }

	Gtk::TreeModelColumn<Glib::ustring>& col_name() { return m_col_name; }
	const Gtk::TreeModelColumn<Glib::ustring>& col_name() const { return m_col_name; }

	//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>& col_icon() { return m_col_icon; }
	//const Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>>& col_icon() const { return m_col_icon; }
	Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>>& col_icon() { return m_col_icon; }
	const Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>>& col_icon() const { return m_col_icon; }

// implementation
private:

	Gtk::TreeModelColumn<NzbFileTuple> m_col_file_tuple;
	Gtk::TreeModelColumn<Glib::ustring> m_col_name;
	//Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> m_col_icon;
	Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>> m_col_icon;
};

/**
 * 
 * 
 */
class NzbFileListStore : public Gtk::ListStore
{
// construction/destruction
public:

	NzbFileListStore();
	~NzbFileListStore();

// attriburtes
public:

	// access to the columns
	NzbFileListStoreColumnRecord& columns() { return m_cols; }
	const NzbFileListStoreColumnRecord& columns() const { return m_cols; }

// operations
public:

	void append_nzb_files(const GNzb& gnzb);
	Gtk::TreeModel::iterator append(const GNzb& gnzb, int file_idx);

// implementation
private:

	void set_values(Gtk::TreeIter& iter, const GNzb& gnzb, int file_idx);

	NzbFileListStoreColumnRecord m_cols;
	Glib::RefPtr<Gdk::Pixbuf> m_file_icons[BINPARTS_COUNT];
};

#endif  /* __NZB_FILE_LISTSTORE_HEADER__ */