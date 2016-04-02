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
#include <Python.h>
#include <frameobject.h>
#include "pythonutil.h"

#if defined(DEBUG) || defined(DEBUG_PYSTACK)
#	include <iostream>
#	include <iomanip>
#endif	/* DEBUG */

namespace PyWrap {

constexpr PyCFunctionObject *PyCFunction(PyObject *p_obj)
{
	return reinterpret_cast<PyCFunctionObject*>(p_obj);
}

#ifdef DEBUG_PYSTACK
template<typename Ch, typename Tr>
decltype(auto) operator <<(std::basic_ostream<Ch,Tr>& out, PyObject *p_obj)
{
	if(p_obj == nullptr)
		out << "NULL";
	else
	{
		out << p_obj->ob_type->tp_name;
		if(PyCFunction_Check(p_obj))
			out << ": \"" << PyCFunction(p_obj)->m_ml->ml_name << '\"';
#if PY_MAJOR_VERSION < 3
		else if(PyString_Check(p_obj))
			out << ": \"" << PyString_AsString(p_obj) << '\"';
#else
		else if(PyUnicode_Check(p_obj))
			out << ": \"" << PyUnicode_AsUTF8(p_obj) << '\"';
#endif
	}
	return out;
}

static void examine_stack(PyObject **stack, const Py_ssize_t size)
{
	for(Py_ssize_t i = 0; i < size; ++stack, ++i)
	{
		std::cout << "stack[" << i << "]: " << *stack << std::endl;
	}
}
#endif	/* DEBUG_PYSTACK */

static std::string get_py_method_name(PyObject **localsplus, Py_ssize_t size)
{
	// look for the first PyCFunction in the given frame's localsplus stack
	// starting at the top (i.e. the largest index) because nested method
	// calls will cause multiple PyCFunction types to be on the stack
	while(--size >= 0)
	{
		// the stack can contain "empty" positions
		if(localsplus[size] == nullptr) continue;

		// check for PyCFunction
		if(PyCFunction_Check(localsplus[size]) && (PyCFunction(localsplus[size])->m_ml != nullptr))
			return std::string(PyCFunction(localsplus[size])->m_ml->ml_name);
	}

	// PyCFunction_Type not found in stack
	return std::string();
}

std::string get_frame_pymethod_name()
{
	PyFrameObject* p_pyframe = PyEval_GetFrame();
#if PY_MAJOR_VERSION < 3
	PyObject **stack = p_pyframe->f_localsplus;
	const Py_ssize_t stack_size = p_pyframe->ob_size;
#else
	PyObject **stack = p_pyframe->f_valuestack;
	const Py_ssize_t stack_size = p_pyframe->f_code->co_stacksize;
#endif
#ifdef DEBUG_PYSTACK
	std::cout << "#ARGS: " << p_pyframe->f_code->co_argcount << " #LOCALS: " << p_pyframe->f_code->co_nlocals << " STACK: " << p_pyframe->f_code->co_stacksize << " FLAGS: 0x" << std::setw(8) << std::setfill('0') << std::hex << p_pyframe->f_code->co_flags << std::endl;
	std::cout << "- localsplus -------------------------------------------------------------------" << std::endl;
	examine_stack(p_pyframe->f_localsplus, stack_size);
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
	std::cout << "- valuestack -------------------------------------------------------------------" << std::endl;
	examine_stack(p_pyframe->f_valuestack, stack_size);
	std::cout << "--------------------------------------------------------------------------------" << std::endl;
#endif	/* DEBUG_PYSTACK */
	return get_py_method_name(stack, stack_size);
}

}   // namespace PyWrap
