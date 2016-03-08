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
#include "nzbutil.h"
#include <libusenet/nzb.h>

NzbFileSubjectTool::NzbFileSubjectTool()
:   m_regex("^.*\"(.+)\".*$")
{
}


NzbFileSubjectTool::~NzbFileSubjectTool()
{
}

std::string NzbFileSubjectTool::get_file_name(const std::string subject) const
{
	std::string result("");
	std::smatch match;
	std::regex_match(subject, match, m_regex);
	if(match.size() > 1)
		result.assign(match[1]);
	return result;
}

std::string NzbFileSubjectTool::get_file_name(const char *subject) const
{
	std::string result("");
	std::cmatch match;
	std::regex_match(subject, match, m_regex);
	if(match.size() > 1)
		result.assign(match[1]);
	return result;
}

std::string NzbFileSubjectTool::get_file_name(const NZB::File& nzbFile) const
{
	return get_file_name(nzbFile.getSubject());
}