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
#include "fsitemliststore.h"
#include "filenamehelper.h"
#include <iostream>

FileSystemItemListStore::Columns::Columns()
{
	add(m_col_type);
	add(m_col_fq_path);
	add(m_col_icon);
	add(m_col_text);
}

FileSystemItemListStore::FileSystemItemListStore()
{
	set_column_types(m_cols);

	Gtk::TreeIter iter = append();
	(*iter)[cols().col_type()] = SEPARATOR;
	(*iter)[cols().col_text()] = "separator";

	iter = append();
	(*iter)[cols().col_type()] = SELECT;
	(*iter)[cols().col_text()] = "Other...";
}

Gtk::TreeIter FileSystemItemListStore::appendFileSystemItem(const std::string& fq_path)
{
	Glib::ustring text = FileNameHelper(fq_path.c_str()).get_name();

	Gtk::TreeIter iter = insert(find_separator());
	(*iter)[cols().col_type()] = FS_ITEM;
	(*iter)[cols().col_fq_path()] = fq_path;
	(*iter)[cols().col_text()] = text;

	return iter;
}

Gtk::TreeIter FileSystemItemListStore::appendFileSystemItem(const std::string& fq_path, Glib::RefPtr<Gio::Icon> ref_icon)
{
	Glib::ustring text = FileNameHelper(fq_path.c_str()).get_name();

	Gtk::TreeIter iter = insert(find_separator());
	(*iter)[cols().col_type()] = FS_ITEM;
	(*iter)[cols().col_fq_path()] = fq_path;
	(*iter)[cols().col_text()] = text;
	(*iter)[cols().col_icon()] = ref_icon;

	return iter;
}

Gtk::TreeIter FileSystemItemListStore::prependFileSystemItem(const std::string& fq_path)
{
	Glib::ustring text = FileNameHelper(fq_path.c_str()).get_name();

	Gtk::TreeIter iter = prepend();
	(*iter)[cols().col_type()] = FS_ITEM;
	(*iter)[cols().col_fq_path()] = fq_path;
	(*iter)[cols().col_text()] = text;

	return iter;
}

Gtk::TreeIter FileSystemItemListStore::prependFileSystemItem(const std::string& fq_path, Glib::RefPtr<Gio::Icon> ref_icon)
{
	Glib::ustring text = FileNameHelper(fq_path.c_str()).get_name();

	Gtk::TreeIter iter = prepend();
	(*iter)[cols().col_type()] = FS_ITEM;
	(*iter)[cols().col_fq_path()] = fq_path;
	(*iter)[cols().col_text()] = text;
	(*iter)[cols().col_icon()] = ref_icon;

	return iter;
}

std::string FileSystemItemListStore::getFileSystemItemPath(Gtk::TreeIter& iter)
{
	return std::string((Glib::ustring)(*iter)[cols().col_fq_path()]);
}

Gtk::TreeIter FileSystemItemListStore::findByPath(const std::string& fq_path)
{
	for(auto& iter : children())
	{
		if(0 == ((Glib::ustring)(*iter)[cols().col_fq_path()]).compare(fq_path))
			return iter;
	}

	return children().end();
}

bool FileSystemItemListStore::isRowSeparator(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::iterator& iter)
{
	bool result = ((*iter)[cols().col_type()] == SEPARATOR); 
	return result;
}

Gtk::TreeIter FileSystemItemListStore::find_separator()
{
	Gtk::TreeIter result = children().begin();
	for(; result != children().end(); ++result)
	{
		if((*result)[cols().col_type()] == SEPARATOR)
			break;
	}
	return result;
}