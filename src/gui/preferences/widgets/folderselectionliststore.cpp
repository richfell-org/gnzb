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
#include "folderselectionliststore.h"
#include "filenamehelper.h"

static const Glib::ustring sFslsSelectDirText("<Select directory>");

FolderSelectionListStore::FolderSelectionListStore()
:   FileSystemItemListStore()
{
	Gtk::TreeIter iter = prepend();
	(*iter)[columns().type()] = FS_ITEM;
	(*iter)[columns().text()] = sFslsSelectDirText;
}

FolderSelectionListStore::~FolderSelectionListStore()
{
}

void FolderSelectionListStore::setDirectory(const std::string& fq_directory_path)
{
	Glib::ustring text(sFslsSelectDirText);
	if(!fq_directory_path.empty())
		text = FileNameHelper(fq_directory_path.c_str()).get_name();

	Gtk::TreeIter iter = children().begin();
	(*iter)[columns().fq_path()] = fq_directory_path;
	(*iter)[columns().text()] = text;
}

void FolderSelectionListStore::setDirectoryIcon(Glib::RefPtr<Gio::Icon> ref_icon)
{
	Gtk::TreeIter iter = children().begin();
	(*iter)[columns().icon()] = ref_icon;
}

std::string FolderSelectionListStore::getDirectory()
{
	std::string result("");

	Gtk::TreeIter iter = children().begin();
	const Glib::ustring& dirString = (Glib::ustring)(*iter)[columns().fq_path()];
	if(0 != sFslsSelectDirText.compare(dirString))
		result.assign(dirString.c_str());
	return result;
}