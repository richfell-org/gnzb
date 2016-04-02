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
#include "pythonobject.h"
#include "pythonutil.h"

#ifdef DEBUG
#	include <iostream>
#endif	/* DEBUG */

extern "C" {

PyObject *pywrap_extobj_noargs(PyObject *p_pyobject, PyObject *args)
{
	try
	{
		// look for the method name
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtObject*>(p_pyobject);
			PyWrap::Object result = p_pythonobj->p_impl->exec_noargs_method(method_name);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return p_obj;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_noargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_noargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

PyObject *pywrap_extobj_varargs(PyObject *p_pyobject, PyObject *args)
{
	try
	{
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtObject*>(p_pyobject);
			PyWrap::Tuple args_tuple(args);
			PyWrap::Object result = p_pythonobj->p_impl->exec_varargs_method(method_name, args_tuple);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return p_obj;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_varargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_varargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

PyObject *pywrap_extobj_varkwargs(PyObject *p_pyobject, PyObject *args, PyObject *kw_args)
{
	try
	{
		std::string method_name = PyWrap::get_frame_pymethod_name();
		if(!method_name.empty())
		{
			auto p_pythonobj = reinterpret_cast<PyWrap::PyExtObject*>(p_pyobject);
			PyWrap::Tuple args_tuple(args);
			PyWrap::Dict kw_dict(kw_args);
			PyWrap::Object result = p_pythonobj->p_impl->exec_varkwargs_method(method_name, args_tuple, kw_dict);
			if(result)
			{
				PyObject *p_obj = static_cast<PyObject*>(result);
				Py_INCREF(p_obj);
				return p_obj;
			}
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_varkwargs: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << "pywrap_extobj_varkwargs: unknown error type caught" << std::endl;
#endif  /* DEBUG */
	}

	// else return Py_None
	Py_RETURN_NONE;
}

}

namespace PyWrap {

ExtensionObject::ExtensionObject()
:	Object(reinterpret_cast<PyObject*>(alloc_pyobject()))
{
}

ExtensionObject::ExtensionObject(ExtensionObject&& that)
:	Object(std::move(that))
{
}

ExtensionObject::~ExtensionObject()
{
}

ExtensionObject& ExtensionObject::operator =(ExtensionObject&& that)
{
	Object::operator =(std::move(that));
	return *this;
}

Object ExtensionObject::exec_noargs_method(const std::string& name)
{
	PyNoArgsMethod mem_fun = s_noargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)();
	return Object();
}

Object ExtensionObject::exec_varargs_method(const std::string& name, Tuple& args)
{
	PyVargsMethod mem_fun = s_varargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)(args);
	return Object();
}

Object ExtensionObject::exec_varkwargs_method(const std::string& name, Tuple& args, Dict& kw)
{
	PyVargsKwMethod mem_fun = s_varkwargs[name];
	if(mem_fun != nullptr)
		return (this->*mem_fun)(args, kw);
	return Object();
}

PyExtObject *ExtensionObject::alloc_pyobject()
{
	PyObject *p_obj = s_pytype.tp_alloc(&s_pytype, 0);
	PyExtObject *p_extobj = reinterpret_cast<PyExtObject*>(p_obj);
	p_extobj->p_impl = this;
	return p_extobj;
}

bool ExtensionObject::add_object(Module& module, const char *name)
{
	// set the object name
	//s_object_name.assign(PyInvokeFunc(PyModule_GetName, static_cast<PyObject*>(module)));
	s_object_name.assign(module.name());
	s_object_name.append(1, '.').append(name);
	s_pytype.tp_name = s_object_name.c_str();
	
	// set the sentinel record in the method definition table
	// make sure the sentinel push_back is done before setting
	// the tp_methods member becuase re-allocation may occur
	PyMethodDef pmd = {nullptr};
	s_methods.push_back(pmd);
	s_pytype.tp_methods = &s_methods[0];

	// make the type ready
	PyInvokeFunc(PyType_Ready, &s_pytype);

	// add this extension object to the module
	//Py_INCREF(&s_pytype);
	//return(0 == PyInvokeFunc(PyModule_AddObject, static_cast<PyObject*>(module), name, reinterpret_cast<PyObject*>(&s_pytype)));
	return module.add_object(name, &s_pytype);
}

PyTypeObject ExtensionObject::s_pytype = {
    PyObject_HEAD_INIT(NULL)
#if PY_MAJOR_VERSION < 3
    0,                         /*ob_size*/
#endif
    nullptr,                   /*tp_name*/
    sizeof(PyExtObject),       /*tp_basicsize*/
    0,                         /*tp_itemsize*/

	/*tp_dealloc*/
    (destructor)[](PyObject *p_obj) { PyObject_Del(p_obj); },

    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    PyObject_GenericGetAttr,   /*tp_getattro*/
    PyObject_GenericSetAttr,   /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    "",                        /* tp_doc */
    0,		                   /* tp_traverse */
    0,		                   /* tp_clear */
    0,		                   /* tp_richcompare */
    0,		                   /* tp_weaklistoffset */
    0,		                   /* tp_iter */
    0,		                   /* tp_iternext */
    nullptr,                   /* tp_methods */
    nullptr,                   /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */

	/* tp_init */
    (initproc)[](PyObject *self, PyObject *args, PyObject *kwds)
	{
		return int(0);
	},

	/* tp_alloc */
    [](PyTypeObject *self, Py_ssize_t nitems)
	{
		PyObject *result = PyType_GenericAlloc(self,nitems);
		(reinterpret_cast<PyExtObject*>(result))->p_impl = nullptr;
		return result;
	},

	/* tp_new */
    [](PyTypeObject *subtype, PyObject *args, PyObject *kwds)
	{
		PyObject *result = subtype->tp_alloc(subtype, 0);
		return result;
	},
};

std::string ExtensionObject::s_object_name;

std::vector<PyMethodDef> ExtensionObject::s_methods;

std::map<std::string, ExtensionObject::PyNoArgsMethod> ExtensionObject::s_noargs;
std::map<std::string, ExtensionObject::PyVargsMethod> ExtensionObject::s_varargs;
std::map<std::string, ExtensionObject::PyVargsKwMethod> ExtensionObject::s_varkwargs;

void ExtensionObject::add_noargs(const char *name, PyNoArgsMethod mem_fun, const char *doc)
{
	PyMethodDef m_def = {const_cast<char*>(name), pywrap_extobj_noargs, METH_NOARGS, const_cast<char*>(doc)};
	s_methods.push_back(m_def);
	s_noargs[name] = mem_fun;
}

void ExtensionObject::add_varargs(const char *name, PyVargsMethod mem_fun, const char *doc)
{
	PyMethodDef meth_def = { const_cast<char*>(name), pywrap_extobj_varargs, METH_VARARGS, const_cast<char*>(doc)};
	s_methods.push_back(meth_def);
	s_varargs[name] = mem_fun;
}

void ExtensionObject::add_varkwargs(const char *name, PyVargsKwMethod mem_fun, const char *doc)
{
	PyMethodDef meth_def = { const_cast<char*>(name), (PyCFunction)pywrap_extobj_varkwargs, METH_KEYWORDS, const_cast<char*>(doc) };
	s_methods.push_back(meth_def);
	s_varkwargs[name] = mem_fun;
}

}	// namespace PyWrap
