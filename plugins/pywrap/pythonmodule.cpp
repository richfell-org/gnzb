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
#include "pythonmodule.h"
#include "pythonutil.h"

extern "C" {

PyObject *pywrap_extmod_noargs(PyObject *p_pyobject, PyObject *args)
{
	try
	{
		// look for the method name
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtModule*>(p_pyobject);
			PyWrap::Object result = p_pythonobj->p_impl->exec_noargs_method(method_name);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return result;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_noargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_noargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

PyObject *pywrap_extmod_varargs(PyObject *p_pyobject, PyObject *args)
{
	try
	{
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtModule*>(p_pyobject);
			PyWrap::Tuple args_tuple(args);
			PyWrap::Object result = p_pythonobj->p_impl->exec_varargs_method(method_name, args_tuple);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return result;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_varargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_varargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

PyObject *pywrap_extmod_varkwargs(PyObject *p_pyobject, PyObject *args, PyObject *kw_args)
{
	try
	{
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtModule*>(p_pyobject);
			PyWrap::Tuple args_tuple(args);
			PyWrap::Dict kw_dict(kw_args);
			PyWrap::Object result = p_pythonobj->p_impl->exec_varkwargs_method(method_name, args_tuple, kw_dict);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return result;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_varkwargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extmod_varkwargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

}

namespace PyWrap {

bool ExtensionModule::reset(PyObject *p_obj)
{
	return false;
}

Module& ExtensionModule::operator =(Module&& that) noexcept
{
	return *this;
}

Object& ExtensionModule::operator =(PyObject *p_obj)
{
	return *this;
}

Object ExtensionModule::exec_noargs_method(const std::string& name)
{
	ExtModNoArgs mem_fun = s_meth_noargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)();
	return None();
}

Object ExtensionModule::exec_varargs_method(const std::string& name, Tuple& args)
{
	ExtModVarArgs mem_fun = s_meth_varargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)(args);
	return None();
}

Object ExtensionModule::exec_varkwargs_method(const std::string& name, Tuple& args, Dict& kw)
{
	ExtModVarKwArgs mem_fun = s_meth_varkwargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)(args, kw);
	return None();
}

bool ExtensionModule::init_module(const char *name, const char *doc/* = ""*/)
{
	// "NULL" terminate the python method array
	PyMethodDef pmd_sentinel = {nullptr};
	s_pyfuncs.push_back(pmd_sentinel);

	// initialize the module
#if PY_MAJOR_VERSION < 3
	PyExtModule *p_module = new PyExtModule;
	p_module->p_impl = this;
	PyObject *p_mod = PyInvokeFunc(Py_InitModule4, name, &s_pyfuncs[0], doc, reinterpret_cast<PyObject*>(p_module), PYTHON_API_VERSION);
	m_ptr_object.reset(INCREF(p_mod));
	return bool(m_ptr_object);
#else
	s_module_def.m_name = name;
	s_module_def.m_doc = doc;
	s_module_def.m_methods = &s_pyfuncs[0];

	// create the module
	PyInvokeFunc(PyImport_AddModule, name);
	m_ptr_object.reset(PyInvokeFunc(PyModule_Create2, &s_module_def, PYTHON_API_VERSION));
	if(!m_ptr_object) return false;

	// set this instance in the state
	PyExtModule *p_state = reinterpret_cast<PyExtModule*>(PyInvokeMethod(PyModule_GetState, m_ptr_object.get()));
	if(p_state == nullptr) return false;
	p_state->p_impl = this;

	Dict sys_dict = Dict(PyInvokeFunc(PyImport_GetModuleDict));
	sys_dict.set_item(name, Module(m_ptr_object.get()));
	return true;
#endif	/* PY_MAJOR_VERSION < 3 */
}

void ExtensionModule::add_noargs(const char *name, ExtModNoArgs mem_fun, const char *doc)
{
	PyMethodDef pmd{const_cast<char*>(name), pywrap_extmod_noargs, METH_NOARGS, const_cast<char*>(doc)};
	s_pyfuncs.push_back(pmd);
	s_meth_noargs[name] = (ExtModNoArgs)mem_fun;
}

void ExtensionModule::add_varargs(const char *name, ExtModVarArgs mem_fun, const char *doc)
{
	PyMethodDef pmd{const_cast<char*>(name), pywrap_extmod_varargs, METH_VARARGS, const_cast<char*>(doc)};
	s_pyfuncs.push_back(pmd);
	s_meth_varargs[name] = (ExtModVarArgs)mem_fun;
}

void ExtensionModule::add_varkwargs(const char *name, ExtModVarKwArgs mem_fun, const char *doc)
{
	PyMethodDef pmd{const_cast<char*>(name), (PyCFunction)pywrap_extmod_varkwargs, METH_NOARGS, const_cast<char*>(doc)};
	s_pyfuncs.push_back(pmd);
	s_meth_varkwargs[name] = (ExtModVarKwArgs)mem_fun;
}

#if PY_MAJOR_VERSION >= 3
PyModuleDef ExtensionModule::s_module_def = {
	PyModuleDef_HEAD_INIT,
	nullptr,				/* m_name */
	nullptr,				/* m_doc */
	sizeof(PyExtModule),	/* m_size */
	nullptr,				/* m_methods */
};
#endif	/* PY_MAJOR_VERSION >= 3 */

// python method definitions
std::vector<PyMethodDef> ExtensionModule::s_pyfuncs; 

// python call to method mapping
std::map<std::string, ExtensionModule::ExtModNoArgs> ExtensionModule::s_meth_noargs;
std::map<std::string, ExtensionModule::ExtModVarArgs> ExtensionModule::s_meth_varargs;
std::map<std::string, ExtensionModule::ExtModVarKwArgs> ExtensionModule::s_meth_varkwargs;

}   // namespace PyWrap
