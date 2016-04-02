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
#ifndef __PYTHON_MODULE_HEADER__
#define __PYTHON_MODULE_HEADER__

#include <pythontypes.h>
#include <vector>
#include <map>

#if !defined(PY_MAJOR_VERSION)
#	error PY_MAJOR_VERSION not defined
#endif

namespace PyWrap {

class ExtensionModule;

/**
 * The PyObject type form module extensions.
 */
struct PyExtModule {
	PyObject_HEAD
	ExtensionModule *p_impl;
};

/**
 * Derive from this class to create a Python extension module.  Use the add_*_fun methods
 * to add you derived class methods as handler's for functions called against the module in
 * python scripts.
 *
 * Use the init_module method from an instantiated instance in order to register the
 * extension module with the python interpreter.  This will cause that instance to be
 * the one which is called for the python functions that were added.
 *
 * The idea is to make the extension module a singleton.  Prior to the instantiation of
 * the single instance, use the static methods for adding methods to handle their named
 * Python methods and then to register the module with init_module.  If extension objects
 * are also being added for the module then use the return value of init_module to pass
 * into the ExtensionObject::add_object method for those extension objects.
 *
 * Example:
 *
 *    class MyModule : public PyWrap::ExtensionModule
 *    {
 *        // TODO: fill in example
 *    };
 */
class ExtensionModule : public Module
{
// construction/destruction
public:

	ExtensionModule(PyObject*) = delete;
	ExtensionModule(ExtensionModule&&) = delete;
	virtual ~ExtensionModule() {}

	bool init_module(const char *name, const char *doc = "");

// operations
public:

	// extension module can't be changed
	bool reset(PyObject *p_obj) override;
	Module& operator =(Module&& that) noexcept;
	Object& operator =(PyObject *p_obj) override;

	// calls the method for the given python name
	Object exec_noargs_method(const std::string& name);
	Object exec_varargs_method(const std::string& name, Tuple& args);
	Object exec_varkwargs_method(const std::string& name, Tuple& args, Dict& kw);

// implementation
protected:

	// to use this class, subclass it.
	ExtensionModule() {}

	// member pointer typedefs for no-args, var-args and var+kw-args function handlers
	using ExtModNoArgs = Object (ExtensionModule::*)();
	using ExtModVarArgs = Object (ExtensionModule::*)(Tuple&);
	using ExtModVarKwArgs = Object (ExtensionModule::*)(Tuple&, Dict&);

	/**
     * Create a python method for the extension module and add the class method to be invoked
     * when the method is called from a python script.  The PyMethodDef entry is created and
     * the connection to the derived class method is mapped to it.
     *
     * The C++ methods can have one of three signatures:
     *
     *    PyObject *method()
     *    PyObject *method(PyWrap::Tuple&)
     *    PyObject *method(PyWrap::Tuple&, PyWrap::Dict&)
     */
	template<typename D, typename... Args, class = typename std::enable_if<std::is_base_of<ExtensionModule,D>::value>::type>
	static void add_method(const char *name, Object (D::*mem_fun)(Args...), const char *doc = "")
	{
		// check the argument count
		constexpr int arg_count = sizeof...(Args);
		static_assert(arg_count >= 0 && arg_count < 3, "Error - methods can take 0, 1 or two arguments");

		switch(arg_count)
		{
			case 0:
				add_noargs(name, (ExtModNoArgs)mem_fun, doc);
				break;
			case 1:
				add_varargs(name, (ExtModVarArgs)mem_fun, doc);
				break;
			case 2:
				add_varkwargs(name, (ExtModVarKwArgs)mem_fun, doc);
				break;
			default:
				break;
		}
	}

private:

	// make specific entries in the PyMethodDef
	static void add_noargs(const char *name, ExtModNoArgs mem_fun, const char *doc);
	static void add_varargs(const char *name, ExtModVarArgs mem_fun, const char *doc);
	static void add_varkwargs(const char *name, ExtModVarKwArgs mem_fun, const char *doc);

#if PY_MAJOR_VERSION >= 3
	static PyModuleDef s_module_def;
#endif

	// PyMethodDef entries for the class
	static std::vector<PyMethodDef> s_pyfuncs; 

	// python call to method mapping
	static std::map<std::string, ExtModNoArgs> s_meth_noargs;
	static std::map<std::string, ExtModVarArgs> s_meth_varargs;
	static std::map<std::string, ExtModVarKwArgs> s_meth_varkwargs;
};

}   // namespace PyWrap

#endif  /* __PYTHON_MODULE_HEADER__ */
