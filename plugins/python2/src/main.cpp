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
#include <memory>
#include <thread>
#include <Python.h>
#include "module/gnzbmodule.h"
//#include "pyobject/gnzbpyobject.h"
#include <gnzb.h>
#include <plugin/gnzbplugin.h>
#include <dlfcn.h>
#include "util.h"

#include "../config.h"

#ifdef DEBUG
#   include <iostream>
#endif  /* DEBUG */

#define	 PATH_MAX_ 256

// global python main thread state
static PyThreadState* p_py_main_threadstate;

/**
 *
 */
class GnzbPyThreadLock
{
public:

	GnzbPyThreadLock(PyInterpreterState* p_interp_state)
	:   m_thread_state(PyThreadState_New(p_interp_state))
	{
		PyEval_RestoreThread(m_thread_state);
	}

	~GnzbPyThreadLock() { unlock(); }

	void unlock()
	{
		if(m_thread_state != nullptr)
		{
			PyEval_SaveThread();
			m_thread_state = nullptr;
		}
	}

private:

	PyThreadState* m_thread_state{nullptr};
};

static void run_event_handler(std::shared_ptr<GNzb> ptr_gnzb, PyObject *p_callable)
{
	if(nullptr == p_callable) return;

	try
	{
		GnzbPyThreadLock py_thread_guard(p_py_main_threadstate->interp);

		PyObject_CallFunction(p_callable, nullptr);

		py_thread_guard.unlock();
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cerr << PACKAGE " run_event_handler: error - " << e.what() << std::endl;
#endif  /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cerr << PACKAGE " run_event_handler: unknown error type" << std::endl;
#endif  /* DEBUG */
	}
}

/**
 *
 *
 */
class Py2Plugin : public GNzbPlugin
{
public:

	Py2Plugin() {}
	~Py2Plugin() {}

public:

	bool init(const std::string& path);

	void on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb);
	void on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb);
	void on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb);

private:

	// script event handlers (borrowed refs)
	PyObject *mp_gnzb_added{nullptr};
	PyObject *mp_gnzb_finished{nullptr};
	PyObject *mp_gnzb_cancelled{nullptr};	
};

bool Py2Plugin::init(const std::string& path)
{
	bool result = true;

	// parse the path and module info from the given path
	PySourceFile source_info(path);

#ifdef DEBUG
	std::cout << PACKAGE " load_module: module name is " << source_info.get_module() << std::endl;
#endif  /* DEBUG */

	PyEval_RestoreThread(p_py_main_threadstate);

	// add the path of the script file to the module load path
	PyObject *sys_path = PySys_GetObject("path");
	PyObject *module_path = PyString_FromString(source_info.get_path().c_str());
	PyList_Append(sys_path, module_path);

	// load the given module
	PyObject *p_module = PyImport_ImportModule(source_info.get_module().c_str());
	if(p_module == 0)
		PyErr_Print();
	else
	{
		try
		{
			// the dictionary instanace is "borrowed"
			PyObject *pDict = PyModule_GetDict(p_module);

			// get the event handlers
			mp_gnzb_added = PyDict_GetItemString(pDict, "nzb_added");
			mp_gnzb_finished = PyDict_GetItemString(pDict, "nzb_finished");
			mp_gnzb_cancelled =PyDict_GetItemString(pDict, "nzb_cancelled");

			// if the script provided on on_load entry point then call it
			PyObject* p_on_load = PyDict_GetItemString(pDict, "on_load");

			// get any declared gnzb event handlers
			if(PyCallable_Check(p_on_load))
				 PyObject_CallFunction(p_on_load, nullptr);

			// Clean up module reference
			Py_DECREF(p_module);
		}
		catch(const std::exception& e)
		{
	#ifdef DEBUG
			std::cout << PACKAGE " init_module: error in module's on_load - " << e.what() << std::endl;
	#endif /* DEBUG */
			result = false;
		}
		catch(...)
		{
	#ifdef DEBUG
			std::cout << PACKAGE " init_module: unknown error in module's on_load" << std::endl;
	#endif /* DEBUG */
			result = false;
		}
	}

	p_py_main_threadstate = PyEval_SaveThread();

	return result;
}

void Py2Plugin::on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(PyCallable_Check(mp_gnzb_added))
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, mp_gnzb_added};
		handler_thread.detach();
	}
}

void Py2Plugin::on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(PyCallable_Check(mp_gnzb_finished))
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, mp_gnzb_finished};
		handler_thread.detach();
	}
}

void Py2Plugin::on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(PyCallable_Check(mp_gnzb_cancelled))
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, mp_gnzb_cancelled};
		handler_thread.detach();
	}
}

/**
 * plugin_load
 *	  gnzb plugins have this called when they are loaded by the gnzb app
 */   
extern "C" void plugin_load()
{
	// load the python intrepert SO
	dlopen(PYLIB_NAME, RTLD_NOW|RTLD_GLOBAL);

	// initialize the python library if needed
	if(!Py_IsInitialized())
	{
		// initialize python
		Py_Initialize();
		PyEval_InitThreads();

		init_gnzb_module();

		// access the main thread state
		p_py_main_threadstate = PyGILState_GetThisThreadState();
		PyEval_ReleaseThread(p_py_main_threadstate);
	}

#ifdef DEBUG
	std::cout << PACKAGE " plugin_load()" << std::endl;
#endif  /* DEBUG */
}

/**
 * plugin_unload
 *	  gnzb plugins have this called when they are unloaded by the gnzb app
 */   
extern "C" void plugin_unload()
{
	// finialize the python library if needed
	if(Py_IsInitialized())
	{
		if(nullptr != p_py_main_threadstate)
		{
			PyEval_RestoreThread(p_py_main_threadstate);
			p_py_main_threadstate = nullptr;
		}
		Py_Finalize();
	}

#ifdef DEBUG
	std::cout << PACKAGE " plugin_unload()" << std::endl;
#endif  /* DEBUG */
}

extern "C"
{

static GNzbPlugin *allocate_plugin()
{
	return new Py2Plugin;
}

GNzbPluginDescriptor plugin_descriptor =
{
	GNZB_PLUGIN_API_VERSION,

	plugin_load,
	plugin_unload,

	allocate_plugin
};

};