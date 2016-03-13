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
#include "gnzbpyobject.h"
#include <gnzb.h>

#include <iostream>

void GnzbPyObject::init_type()
{
	//Py::PythonType &behaviors = behaviors();
	behaviors().name("gnzb");
	behaviors().doc("GNzb python representation");

	// python object methods
	add_varargs_method("path", &GnzbPyObject::path, "The path of the NZB file in the file system");
	add_varargs_method("output_path", &GnzbPyObject::output_path, "The output path of the NZB file in the file system");
	add_varargs_method("is_download_complete", &GnzbPyObject::is_download_complete, "Download complete flag");
	add_varargs_method("fraction_download_complete", &GnzbPyObject::fraction_download_complete, "Fraction of downloaded data");
}

GnzbPyObject::GnzbPyObject(const std::shared_ptr<GNzb>& ptr_gnzb)
:   Py::PythonExtension<GnzbPyObject>(),
	m_ptr_gnzb(ptr_gnzb)
{
}

GnzbPyObject::~GnzbPyObject()
{
}

Py::Object GnzbPyObject::path(const Py::Tuple &args)
{
	return Py::String(m_ptr_gnzb->path());
}

Py::Object GnzbPyObject::output_path(const Py::Tuple &args)
{
	return Py::String(m_ptr_gnzb->output_path());
}

Py::Object GnzbPyObject::is_download_complete(const Py::Tuple &args)
{
	return m_ptr_gnzb->download_data().is_complete() ? Py::Int(1) : Py::Int(0);
}

Py::Object GnzbPyObject::fraction_download_complete(const Py::Tuple &args)
{
	return Py::Long(long(m_ptr_gnzb->download_data().get_fraction_complete()));
}