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
#ifndef __SINGLE_FILE_LIST_STORE__
#define __SINGLE_FILE_LIST_STORE__

#include "fsitemliststore.h"

/*
 * 
 */
class SingleFileListStore : public FileSystemItemListStore
{
public:

	SingleFileListStore();
	~SingleFileListStore();

// operations
public:

	void set_file(const std::string& fq_path);
	std::string get_file() const;

	void set_file_icon(Glib::RefPtr<Gio::Icon> ref_icon);
	Glib::RefPtr<Gio::Icon> get_file_icon() const;

	void clear_selection();
};

#endif  /* __SINGLE_FILE_LIST_STORE__ */