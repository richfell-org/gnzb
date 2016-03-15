/*
	gnzb python2 DSO plugin - gnzb python2 integration

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
#ifndef __UTIL_HEADER__
#define __UTIL_HEADER__

#include <memory>
#include <string>

#ifndef PyObject_HEAD
struct _object;
typedef _object PyObject;
#endif

struct python_delete
{
	void operator ()(PyObject *p_o);
};

using py_ptr = std::unique_ptr<PyObject, python_delete>;

/**
 *
 */
class PySourceFile
{
public:

	PySourceFile();
	PySourceFile(const std::string& source_path);
	~PySourceFile();

public:

	const std::string& get_path() const	{ return m_path; }
	const std::string& get_file() const	{ return m_file; }
	const std::string& get_module() const  { return m_module; }

public:

	void parse_from_path(const std::string& source_path);

private:

	std::string m_path, m_file, m_module;
};

#endif  /* __UTIL_HEADER__ */