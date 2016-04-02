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
#ifndef __PYTHON_OBJECT_HEADER__
#define __PYTHON_OBJECT_HEADER__

#include <Python.h>
#include <type_traits>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include "pythontypes.h"

namespace PyWrap {

class ExtensionObject;

/**
 * This is PyWrap's extension object PyObject type.
 */
struct PyExtObject {
	PyObject_HEAD
	ExtensionObject *p_impl;
};

/**
 * Derive from this class to create an extension type for a python module.  Use
 * the add_*_method to add both python methods for the type as well as set the
 * derived class methods which will be called for those python invoked methods.
 *
 */
class ExtensionObject : public Object
{
// construction/destruction
public:

	ExtensionObject(ExtensionObject&& that);
	~ExtensionObject();

// operations
public:

	// calls the class instance method for the given python name
	Object exec_noargs_method(const std::string& name);
	Object exec_varargs_method(const std::string& name, Tuple& args);
	Object exec_varkwargs_method(const std::string& name, Tuple& args, Dict& kw);

	ExtensionObject& operator =(ExtensionObject&& that);

	// class-wide initialization with python interpreter
	static bool add_object(Module& module, const char *name);

// implementation for subclasses
protected:

	// To use this class, subclass it.
	ExtensionObject();

	// allocate our version of PyObject
	virtual PyExtObject *alloc_pyobject();

	// "typedefs" for python method handlers
	using PyNoArgsMethod = Object (ExtensionObject::*)();
	using PyVargsMethod = Object (ExtensionObject::*)(Tuple&);
	using PyVargsKwMethod = Object (ExtensionObject::*)(Tuple&,Dict&);

	/**
     * Create a python method for the extension object and add the class method to be invoked
     * when the method is called from a python script.  The PyMethodDef entry is created and
     * the connection to the derived class method is mapped to it.
     *
     * The C++ methods can have one of three signatures:
     *
     *    PyObject *method()
     *    PyObject *method(PyWrap::Tuple&)
     *    PyObject *method(PyWrap::Tuple&, PyWrap::Dict&)
     */
	template<typename D, typename... Args, class = typename std::enable_if<std::is_base_of<ExtensionObject,D>::value>::type>
	static void add_method(const char *name, Object (D::*mem_fun)(Args...), const char *doc = "")
	{
		// check the argument count
		constexpr int arg_count = sizeof...(Args);
		static_assert(arg_count >= 0 && arg_count < 3, "Error - methods can take 0, 1 or two arguments");

		switch(arg_count)
		{
			case 0:
				add_noargs(name, (PyNoArgsMethod)mem_fun, doc);
				break;
			case 1:
				add_varargs(name, (PyVargsMethod)mem_fun, doc);
				break;
			case 2:
				add_varkwargs(name, (PyVargsKwMethod)mem_fun, doc);
				break;
			default:
				break;
		}
	}

// implementation
private:

	static void add_noargs(const char *name, PyNoArgsMethod mem_fun, const char *doc);
	static void add_varargs(const char *name, PyVargsMethod mem_fun, const char *doc);
	static void add_varkwargs(const char *name, PyVargsKwMethod mem_fun, const char *doc);

	// store the name of the object: format is <module_name>.<ExtensionObject_name>
	static std::string s_object_name;

	// the python type
	static PyTypeObject s_pytype;

	// python method definitions
	static std::vector<PyMethodDef> s_methods;

	// maps of class methods to named python object methods
	static std::map<std::string, PyNoArgsMethod> s_noargs;
	static std::map<std::string, PyVargsMethod> s_varargs;
	static std::map<std::string, PyVargsKwMethod> s_varkwargs;
};

}   // namespace PyWrap

#endif  /* __PYTHON_OBJECT_HEADER__ */
