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
#ifndef __FOLDER_SELECTION_LIST_STORE__
#define __FOLDER_SELECTION_LIST_STORE__

#include "fsitemliststore.h"

/*
 * 
 */
class FolderSelectionListStore : public FileSystemItemListStore
{
public:

	FolderSelectionListStore();
	~FolderSelectionListStore();

	void setDirectory(const std::string& fq_directory_path);
	void setDirectoryIcon(Glib::RefPtr<Gio::Icon> ref_icon);
	std::string getDirectory();
};

#endif  /* __FOLDER_SELECTION_LIST_STORE__ */