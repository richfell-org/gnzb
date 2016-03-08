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
#ifndef __FILE_NAME_HELPER_HEADER__
#define __FILE_NAME_HELPER_HEADER__

class FileNameHelper
{
public:

	FileNameHelper(const char *path)
		: m_path(path)
	{
		if('/' == m_path[m_path.size() - 1])
			m_path.erase(m_path.size() - 1);
	}
	~FileNameHelper() {}

	Glib::ustring get_name()
	{
		// check for the last '/' character and if found return
		// all chars after it, else just return the whole value
		Glib::ustring::size_type i = m_path.find_last_of('/');
		if(Glib::ustring::npos != i)
			return m_path.substr(i + 1);
		return m_path;
	}

	Glib::ustring get_path()
	{
		// check for the last '/' character and if found return
		// all chars before it, else just return an empty string
		// because there is no path, only a file name
		Glib::ustring::size_type i = m_path.find_last_of('/');
		if(Glib::ustring::npos != i)
			return m_path.substr(0, i);
		return Glib::ustring("");
	}

private:

	Glib::ustring m_path;
};

#endif  /* __FILE_NAME_HELPER_HEADER__ */