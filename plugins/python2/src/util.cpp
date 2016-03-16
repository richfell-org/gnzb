/*
	gnzb python3 DSO plugin - gnzb python3 integration

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
#include <Python.h>
#include "util.h"

void python_delete::operator ()(PyObject *p_o)
{
	Py_DECREF(p_o);
}

PySourceFile::PySourceFile()
{
}

PySourceFile::PySourceFile(const std::string& source_path)
{
	parse_from_path(source_path);
}

PySourceFile::~PySourceFile()
{
}

void PySourceFile::parse_from_path(const std::string& source_path)
{
	// check for last path separator
	unsigned int pos = source_path.find_last_of('/');

	// if no path separator found then just file name
	// else parse out path and file name
	if(pos == std::string::npos)
	{
		m_path.clear();
		m_file = source_path;
	}
	else
	{
		m_path = source_path.substr(0, pos);
		m_file = source_path.substr(pos + 1);
	}

	// parse out the module name from the file name,
	// it is the name less the extension
	pos = m_file.find_last_of('.');
	m_module = (pos == std::string::npos)
		? m_file
		: m_file.substr(0, pos);
}