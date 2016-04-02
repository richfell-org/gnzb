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
#ifndef __PYTHON_UTIL_HEADER__
#define __PYTHON_UTIL_HEADER__

#include <string>
#include <stdexcept>

#ifdef DEBUG
#	include <iostream>
#endif

namespace PyWrap {

/**
 * Error thrown when an exception is raised during a python call.
 */
class Error : public std::runtime_error
{
public:

	using std::runtime_error::runtime_error;
	~Error() {}

protected:
};

// get the name of the current python object method being
// invoked, it is the name given in the PyMethodDef struct
std::string get_frame_pymethod_name();

inline void check_pyerr()
{
	PyObject *p_error = PyErr_Occurred();
	if(nullptr != p_error)
	{
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
#if PY_MAJOR_VERSION < 3
		std::string errmsg(pvalue ? PyString_AsString(pvalue) : "Unknown python error message");
#else
		PyObject *str = PyObject_Str(pvalue);
		std::string errmsg(str ? PyUnicode_AsUTF8(str) : "Unknown python error message");
#endif	/* PY_MAJOR_VERSION < 3 */
		if(nullptr != ptype) Py_DECREF(ptype);
		if(nullptr != pvalue) Py_DECREF(pvalue);
		if(nullptr != ptraceback) Py_DECREF(ptraceback);
		throw Error(errmsg);
	}
}

template<typename R, typename... T_Args>
R PyInvokeMethod(R (*function)(PyObject*, T_Args...), PyObject *py_this, T_Args... args)
{
	R result = function(py_this, std::forward<T_Args>(args)...);
	check_pyerr();
	return result;
}

template<typename... T_Args>
void PyInvokeMethod(void (*function)(PyObject*, T_Args...), PyObject *py_this, T_Args... args)
{
	function(py_this, std::forward<T_Args>(args)...);
	check_pyerr();
}

template<typename R, typename... T_Args>
R PyInvokeFunc(R (*function)(T_Args...), T_Args... args)
{
	R result = function(std::forward<T_Args>(args)...);
	check_pyerr();
	return result;
}

template<typename... T_Args>
void PyInvokeFunc(void (*function)(T_Args...), T_Args... args)
{
	function(std::forward<T_Args>(args)...);
	check_pyerr();
}

}   // namespace PyWrap

#endif	/* __PYTHON_UTIL_HEADER__ */
