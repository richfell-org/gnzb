/*
	gnzb python common - gnzb python integration

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
#ifndef __PYGNZB_HEADER__
#define __PYGNZB_HEADER__

#include <pythonobject.h>
#include <memory>

class GNzb;

/**
 * Pythonized shared_ptr<GNzb> class
 *
 */
class PyGNzb : public PyWrap::ExtensionObject
{
// construction/destruction
public:

	PyGNzb() {}
	PyGNzb(const std::shared_ptr<GNzb>& ptr_gnzb) : m_ptr_gnzb(ptr_gnzb) {}
	~PyGNzb() {}

// attributes
public:

	PyGNzb& gnzb(const std::shared_ptr<GNzb>& ptr_gnzb) { m_ptr_gnzb = ptr_gnzb; return *this; }
	std::shared_ptr<GNzb> gnzb() const { return m_ptr_gnzb; }

// python methods
public:

	// the path of the NZB file
	PyWrap::Object path();

	// the output path for downloaded NZB content
	PyWrap::Object output_path();
	PyWrap::Object set_output_path(PyWrap::Tuple& args);

	PyWrap::Object echo_str(PyWrap::Tuple& args);

	// type initialization with python
	static bool init_type(PyWrap::Module& module);

// implementation
protected:

	std::shared_ptr<GNzb> m_ptr_gnzb;
};


#endif	/* __PYGNZB_HEADER__ */
