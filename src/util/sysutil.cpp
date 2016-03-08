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
#include <giomm/file.h>

bool does_file_exist(const char *appdb_path)
{
	Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(appdb_path);
	return file->query_exists();
}

std::string parse_file_name(const std::string& path)
{
	std::string result("");
	std::string::size_type i = path.find_last_of('/');
	if(std::string::npos != i)
		result = path.substr(i + 1);
	return result;
}