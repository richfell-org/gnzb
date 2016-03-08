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
#ifndef __FILESYSTEM_ITEM_LIST_STORE_HEADER__
#define __FILESYSTEM_ITEM_LIST_STORE_HEADER__

#include <string>
#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/liststore.h>

/*
 * 
 */
class FileSystemItemListStore : public Gtk::ListStore
{
public:

	class Columns : public Gtk::TreeModelColumnRecord
	{
	public:
		Columns();
		~Columns() {}

		Gtk::TreeModelColumn<int>& col_type() { return m_col_type; }
		const Gtk::TreeModelColumn<int>& col_type() const { return m_col_type; }

		Gtk::TreeModelColumn<Glib::ustring>& col_fq_path() { return m_col_fq_path; }
		const Gtk::TreeModelColumn<Glib::ustring>& col_fq_path() const { return m_col_fq_path; }

		Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>> col_icon() { return m_col_icon; }
		const Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>> col_icon() const { return m_col_icon; }

		Gtk::TreeModelColumn<Glib::ustring>& col_text() { return m_col_text; }
		const Gtk::TreeModelColumn<Glib::ustring>& col_text() const { return m_col_text; }
		
	private:

		Gtk::TreeModelColumn<int> m_col_type;
		Gtk::TreeModelColumn<Glib::ustring> m_col_fq_path;
		Gtk::TreeModelColumn<Glib::RefPtr<Gio::Icon>> m_col_icon;
		Gtk::TreeModelColumn<Glib::ustring> m_col_text;
	};

	Columns& cols() { return m_cols; }
	const Columns& cols() const { return m_cols; }

	enum EntryType { FS_ITEM, SEPARATOR, SELECT, };

	FileSystemItemListStore();
	virtual ~FileSystemItemListStore() {}

	Gtk::TreeIter appendFileSystemItem(const std::string& fq_path);
	Gtk::TreeIter appendFileSystemItem(const std::string& fq_path, Glib::RefPtr<Gio::Icon> ref_icon);
	Gtk::TreeIter prependFileSystemItem(const std::string& fq_path);
	Gtk::TreeIter prependFileSystemItem(const std::string& fq_path, Glib::RefPtr<Gio::Icon> ref_icon);

	std::string getFileSystemItemPath(Gtk::TreeIter& iter);

	Gtk::TreeIter findByPath(const std::string& fqPath);

	bool isRowSeparator(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::iterator& iter);

protected:

	Gtk::TreeIter find_separator();

	Columns m_cols;
};

#endif  /* __FILESYSTEM_ITEM_LIST_STORE_HEADER__ */