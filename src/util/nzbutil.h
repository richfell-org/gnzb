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
#ifndef	__NZB_UTIL_HEADER__
#define __NZB_UTIL_HEADER__

#include <regex>
#include <string>

namespace NZB {
	class File;
}

/*
 * Tool for parsing out the various logical parts of an NZB <file> subject attribute.
 */
class NzbFileSubjectTool
{
public:

	NzbFileSubjectTool();
	~NzbFileSubjectTool();

public:

	std::string get_file_name(const NZB::File& nzbFile) const;
	std::string get_file_name(const char *subject) const;
	std::string get_file_name(const std::string subject) const;

private:

	std::regex m_regex;
};


#endif  /* __NZB_UTIL_HEADER__ */