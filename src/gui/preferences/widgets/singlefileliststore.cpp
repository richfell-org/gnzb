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
	(*iter)[columns().type()] = FS_ITEM;
	(*iter)[columns().text()] = sSflsSelectDirText;
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
	(*iter)[columns().fq_path()] = fq_path;
	(*iter)[columns().text()] = text;
}

std::string SingleFileListStore::get_file() const
{
	std::string result("");

	Gtk::TreeIter iter = children().begin();
	const Glib::ustring& dir_string = (Glib::ustring)(*iter)[columns().fq_path()];
	if(0 != sSflsSelectDirText.compare(dir_string))
		result.assign(dir_string.c_str());
	return result;
}

void SingleFileListStore::set_file_icon(Glib::RefPtr<Gio::Icon> ref_icon)
{
	Gtk::TreeIter iter = children().begin();
	(*iter)[columns().icon()] = ref_icon;
}

Glib::RefPtr<Gio::Icon> SingleFileListStore::get_file_icon() const
{
	Gtk::TreeIter iter = children().begin();
	return (*iter)[columns().icon()];
}

void SingleFileListStore::clear_selection()
{
	Gtk::TreeIter iter = children().begin();
	(*iter)[columns().fq_path()] = Glib::ustring();
	(*iter)[columns().type()] = FS_ITEM;
	(*iter)[columns().text()] = sSflsSelectDirText;
	(*iter)[columns().icon()] = Glib::RefPtr<Gio::Icon>();
}