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
#ifndef __GNZB_PYTHON_OBJECT_HEADER__
#define __GNZB_PYTHON_OBJECT_HEADER__

#include <memory>
#include <CXX/Extensions.hxx>

class GNzb;

/**
 *
 *
 */
class GnzbPyObject : public Py::PythonExtension<GnzbPyObject>
{
public:

	static void init_type();

	GnzbPyObject(const std::shared_ptr<GNzb>& ptr_gnzb);
	~GnzbPyObject();

// attributes
public:

	// paths
	Py::Object path(const Py::Tuple &args);
	Py::Object output_path(const Py::Tuple &args);

	// download state
	Py::Object is_download_complete(const Py::Tuple &args);
	Py::Object fraction_download_complete(const Py::Tuple &args);

private:

	// this instance is not owner
	std::shared_ptr<GNzb> m_ptr_gnzb;
};

#endif  /* __GNZB_PYTHON_OBJECT_HEADER__ */