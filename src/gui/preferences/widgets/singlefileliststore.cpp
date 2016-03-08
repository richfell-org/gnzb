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
#include "singlefileliststore.h"
#include "filenamehelper.h"

static const Glib::ustring sSflsSelectDirText("<Select file>");

SingleFileListStore::SingleFileListStore()
:   FileSystemItemListStore()
{
	Gtk::TreeIter iter = prepend();
	(*iter)[cols().col_type()] = FS_ITEM;
	(*iter)[cols().col_text()] = sSflsSelectDirText;
}

SingleFileListStore::~SingleFileListStore()
{
}

void SingleFileListStore::set_file(const std::string& fq_path)
{
	Glib::ustring text(sSflsSelectDirText);
	if(!fq_path.empty())
		text = FileNameHelper(fq_path.c_str()).get_name();

	Gtk::TreeIter iter = children().begin();
	(*iter)[cols().col_fq_path()] = fq_path;
	(*iter)[cols().col_text()] = text;
}

void SingleFileListStore::set_file_icon(Glib::RefPtr<Gio::Icon> ref_icon)
{
	Gtk::TreeIter iter = children().begin();
	(*iter)[cols().col_icon()] = ref_icon;
}

std::string SingleFileListStore::get_file()
{
	std::string result("");

	Gtk::TreeIter iter = children().begin();
	const Glib::ustring& dir_string = (Glib::ustring)(*iter)[cols().col_fq_path()];
	if(0 != sSflsSelectDirText.compare(dir_string))
		result.assign(dir_string.c_str());
	return result;
}