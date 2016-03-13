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
#include <CXX/WrapPython.h>
#include "module/gnzbmodule.h"
#include "pyobject/gnzbpyobject.h"
#include <gnzb.h>
#include <dlfcn.h>

#include "../config.h"

#ifdef DEBUG
#   include <iostream>
#endif  /* DEBUG */

#define	 PATH_MAX_ 256

// used for PySys_SetArgv
static char py_progname[PATH_MAX_];
static char *py_argv[] = { py_progname, nullptr };

// global python main thread state
PyThreadState* p_py_main_threadstate;

// gnzb's python extension module
static std::unique_ptr<GNzbPyModule> ptr_pymodule;

// python script entry points
static Py::Callable gnzb_added;
static Py::Callable gnzb_finished;
static Py::Callable gnzb_cancelled;

/**
 * init_module
 *	  does the one-time python init needed by this plugin
 */   
static void init_module(const std::string& script_path)
{
	// promote the python SO to RTLD_GLOBAL so symbols can
	// properly resolved for python loaded SO modules
	dlclose(dlopen(PYLIB_NAME, RTLD_NOW|RTLD_GLOBAL|RTLD_NOLOAD));

	if(!ptr_pymodule)
	{
		// init types
		GnzbPyObject::init_type();

		// allocate the gnzb module
		ptr_pymodule.reset(new GNzbPyModule);
	}
}

/**
 * finalize_module
 *	  does the python cleanup needed when this plugin is unloaded
 */   
static void finalize_module()
{
	if(ptr_pymodule)
		ptr_pymodule.reset();
}

/**
 * init_module
 *	  capture the script's GNzb event callables and call it on_load
 *	  if it is defined  
 */
static void init_module(PyObject *p_module)
{
	// the dictionary instnace is "borrowed" (see python docs for explaination of this silliness)
	PyObject *pDict = PyModule_GetDict(p_module);

	// get the scripting event handlers from the module
	gnzb_added = PyDict_GetItemString(pDict, "nzb_added");
	gnzb_finished = PyDict_GetItemString(pDict, "nzb_finished");
	gnzb_cancelled = PyDict_GetItemString(pDict, "nzb_cancelled");

	// if the script provided on on_load entry point then call it
	Py::Callable on_load;
	on_load = PyDict_GetItemString(pDict, "on_load");
	try
	{
		if(on_load.isCallable())
		{
			Py::TupleN args;
			on_load.apply(args);
		}
	}
	catch(const std::exception& e)
	{
#ifdef DEBUG
		std::cout << PACKAGE " load_module: error in module's on_load - " << e.what() << std::endl;
#endif /* DEBUG */
	}
	catch(...)
	{
#ifdef DEBUG
		std::cout << PACKAGE " load_module: unknown error in module's on_load" << std::endl;
#endif /* DEBUG */
	}
}

/**
 * load_module
 *	  ASSUMES that Py_Initialize has been previously called.
 *
 *	  sets python to add the directory of the given path to
 *    list of locations for loading modules and then loads
 *	  the module given in module_path into the interpreter.
 *
 *  module_path - The fully qualified path to the python script
 *
 *  returns false for any python call falures, true otherwise
 */
static bool load_module(const std::string& module_path)
{
	bool result = false;

	// check for last path separator
	unsigned int pos = module_path.find_last_of('/');

	// parse out the module name from the path, it
	// is the file name part less the extension
	std::string module_name = (pos == std::string::npos)
		? module_path
		: module_path.substr(pos + 1);
	pos = module_name.find_last_of('.');
	if(pos != std::string::npos)
		module_name.erase(pos);

#ifdef DEBUG
	std::cout << PACKAGE " load_module: module name is " << module_name << std::endl;
#endif  /* DEBUG */

	// copy the entire path into our argv area
	strncpy(py_progname, module_path.c_str(), PATH_MAX_);

	PyEval_RestoreThread(p_py_main_threadstate);

	// set the argv for python
	PySys_SetArgv(1, py_argv);
	try
	{
		// load the given module
		PyObject *pModule = PyImport_ImportModule(module_name.c_str());
		if(pModule == 0)
			PyErr_Print();
		else
		{
			init_module(pModule);

			// Clean up module reference
			Py_DECREF(pModule);
		}
	}
	catch(const std::exception& e)
	{
	#ifdef DEBUG
		std::cout << PACKAGE " load_module: error - " << e.what() << std::endl;
	#endif /* DEBUG */
		result = false;
	}
	catch(...)
	{
	#ifdef DEBUG
		std::cout << PACKAGE " load_module: unknown error type" << std::endl;
	#endif /* DEBUG */
		result = false;
	}

	p_py_main_threadstate = PyEval_SaveThread();

	return result;
}

class GnzbPyThreadLock
{
public:

	GnzbPyThreadLock(PyInterpreterState* p_interp_state)
	:   m_thread_state(PyThreadState_New(p_interp_state))
	{
		PyEval_RestoreThread(m_thread_state);
	}

	~GnzbPyThreadLock()
	{
		unlock();
	}

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

static void run_event_handler(std::shared_ptr<GNzb> ptr_gnzb, Py::Callable& handler)
{
	try
	{
		GnzbPyThreadLock py_thread_guard(p_py_main_threadstate->interp);

		// as it turns out allocating the GnzbPyObject on
		// the stack won't work becuase TupleN destruction
		// is causing a delete call on GnzbPyObject, even if
		// it's not heap allocated
		GnzbPyObject *p_py_gnzb = new GnzbPyObject(ptr_gnzb);
		Py::TupleN args(p_py_gnzb->self());
		handler.apply(args);

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
 * plugin_load
 *	  gnzb plugins have this called when they are loaded by the gnzb app
 */   
extern "C" void plugin_load()
{
	// initialize the python library if needed
	if(!Py_IsInitialized())
	{
		// initialize python
		Py_Initialize();
		PyEval_InitThreads();

		// init the gnzb module
		init_module("gnzbhandler");

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
	// clean up the allocated module instance
	finalize_module();

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

/**
 * script_plugin_init
 *	  gnzb script plugins have this called for initialization based on the script file
 */   
extern "C" bool script_plugin_init(const std::string& script_path)
{
	return load_module(script_path);
}

/**
 * Called by gnzb when an NZB is opened
 *
 */
extern "C" void on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(gnzb_added.isCallable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(gnzb_added)};
		handler_thread.detach();
	}
}

/**
 * Called by gnzb when an NZB is finished downloading
 *
 */
extern "C" void on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(gnzb_finished.isCallable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(gnzb_finished)};
		handler_thread.detach();
	}
}

/**
 * Called by gnzb when an NZB is cancelled
 *
 */
extern "C" void on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(gnzb_cancelled.isCallable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(gnzb_cancelled)};
		handler_thread.detach();
	}
}