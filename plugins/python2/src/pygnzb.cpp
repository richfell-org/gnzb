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
#include "pygnzb.h"
#include <gnzb.h>

PyWrap::Object PyGNzb::path()
{
	return PyWrap::String(m_ptr_gnzb->path());
}

PyWrap::Object PyGNzb::output_path()
{
	return PyWrap::String(m_ptr_gnzb->output_path());
}

PyWrap::Object PyGNzb::set_output_path(PyWrap::Tuple& args)
{
	bool status = false;
	if((m_ptr_gnzb->download_data().current_size() == 0) && m_ptr_gnzb->not_state(GNzbState::DOWNLOADING))
	{
		try
		{
			PyWrap::String path = args.get_item<PyWrap::String>(0);
			m_ptr_gnzb->output_path(path.to_string());
			status = true;
		}
		catch(const PyWrap::Error& e)
		{
		}
	}
	return status ? PyWrap::Long(1L) : PyWrap::Long(0L);
}

PyWrap::Object PyGNzb::echo_str(PyWrap::Tuple& args)
{
	return args.get_item<PyWrap::String>(0);
}

bool PyGNzb::init_type(PyWrap::Module& module)
{
	add_method("path", &PyGNzb::path, "Gets the NZB file's path");
	add_method("output_path", &PyGNzb::output_path, "Gets the output path for downloaded NZB content");
	add_method("set_output_path", &PyGNzb::set_output_path, "Sets the output path for downloaded NZB content");
	add_method("echo_str", &PyGNzb::echo_str, "Echos first string arg");

	return PyWrap::ExtensionObject::add_object(module, "GNzb");
}

