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
#include <Python.h>
#include "gnzbmodule.h"
#include <vector>
#include <runtimesettings.h>

PyObject *gnzbmod_download_dir(PyObject *self, PyObject *args)
{
	std::string path = RuntimeSettings::locations().base_output_path();
	return Py_BuildValue("s", path.c_str());
}

PyObject *gnzbmod_has_moveto_dir(PyObject *self, PyObject *args)
{
	int is_moveto = RuntimeSettings::locations().move_completed() ? 1 : 0;
	return Py_BuildValue("i", is_moveto);
}

PyObject *gnzbmod_moveto_dir(PyObject *self, PyObject *args)
{
	std::string path = RuntimeSettings::locations().moveto_path();
	return Py_BuildValue("s", path.c_str());
}

static std::vector<PyMethodDef> gnzbmod_py_methods{
	{ "download_dir", gnzbmod_download_dir, METH_VARARGS, "" },
	{ "has_moveto_dir", gnzbmod_has_moveto_dir, METH_VARARGS, "" },
	{ "moveto_dir", gnzbmod_moveto_dir, METH_VARARGS, "" },
	{ nullptr, nullptr, 0, nullptr } /* Sentinel */
};

void init_gnzb_module()
{
	Py_InitModule3("gnzbapp", &gnzbmod_py_methods[0], "gnzb app module");
}

