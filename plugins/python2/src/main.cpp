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
#include <runtimesettings.h>
#include <plugin/gnzbplugin.h>
#include <dlfcn.h>
#include "pygnzb.h"
#include "util.h"

#include "../config.h"

#include <pythontypes.h>
#include <pythonmodule.h>

#ifdef DEBUG
#   include <iostream>
#endif  /* DEBUG */

#define	 PATH_MAX_ 256

// global python main thread state
static PyThreadState* p_py_main_threadstate;

#ifdef DEBUG
std::ostream& operator <<(std::ostream& out, const PySourceFile& sf)
{
	out
		<< "Path: " << sf.get_path() << std::endl
		<< "File: " << sf.get_file() << std::endl
		<< "Module: " << sf.get_module();
	return out;
}
#endif

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

static void run_event_handler(std::shared_ptr<GNzb> ptr_gnzb, PyWrap::Object& callable)
{
	try
	{
		GnzbPyThreadLock py_thread_guard(p_py_main_threadstate->interp);

		PyGNzb pygnzb(ptr_gnzb);
		Py_DECREF(callable.invoke(pygnzb));

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
 * The gnzb plugin class for python2.  We can assume this is used as
 * as singleton because the gnzb app will allocate one instance when
 * it loads this plugin module.
 */
class Py2Plugin : public GNzbPlugin, public PyWrap::ExtensionModule
{
public:

	Py2Plugin(Py2Plugin const&) = delete;
	~Py2Plugin() {}

public:

	bool init(const std::string& path) override;

	void on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb) override;
	void on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb) override;
	void on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb) override;

	Py2Plugin& operator =(Py2Plugin const&) = delete;

protected:

	Py2Plugin() {}

private:

	PyWrap::Object download_dir()
	{
		return PyWrap::String(RuntimeSettings::locations().base_output_path());
	}

	PyWrap::Object has_moveto_dir()
	{
		return PyWrap::Long((RuntimeSettings::locations().move_completed() ? 1 : 0));
	}

	PyWrap::Object moveto_dir()
	{
		
		return PyWrap::String(RuntimeSettings::locations().moveto_path());
	}

	// the loaded script module
	PyWrap::Module m_script_module;

	// script event handlers (borrowed refs)
	PyWrap::Object m_gnzb_added;
	PyWrap::Object m_gnzb_finished;
	PyWrap::Object m_gnzb_cancelled;
};

bool Py2Plugin::init(const std::string& path)
{
	bool result = false;

	GnzbPyThreadLock thread_guard(p_py_main_threadstate->interp);

	try
	{
		// set up the module methods
		add_method("download_dir", &Py2Plugin::download_dir, "The NZB content download directory");
		add_method("has_moveto_dir", &Py2Plugin::has_moveto_dir, "Indicator for NZB content to be moved after complete download");
		add_method("moveto_dir", &Py2Plugin::moveto_dir, "The directory to which download complete NZB content is moved");

		// register the module and extension objects
		if(init_module("gnzbapp", "The gnzb application module"))
		{
			// static init extention types
			PyGNzb::init_type(*this);

			// parse the path and module info from the given path
			PySourceFile source_info(path);

#ifdef DEBUG
			std::cout << PACKAGE " script info:" << std::endl << source_info << std::endl;
#endif  /* DEBUG */

			// add the path of the script file to the module load path
			PyWrap::String stript_dir(source_info.get_path());
			PyWrap::List sys_path_list = PyWrap::Sys::get_object("path");
			if(sys_path_list.count(stript_dir) == 0)
				sys_path_list.append_item(stript_dir);

			// load the given module
			m_script_module = PyWrap::Module::import(source_info.get_module());

			// the dictionary instanace is "borrowed"
			PyWrap::Dict dict = m_script_module.dict();

			// get the event handlers
			m_gnzb_added = dict.get_item<PyWrap::Object>("nzb_added");
			m_gnzb_finished = dict.get_item<PyWrap::Object>("nzb_finished");
			m_gnzb_cancelled = dict.get_item<PyWrap::Object>("nzb_cancelled");

			// if the script provided on on_load entry point then call it
			PyWrap::Object on_load = dict.get_item<PyWrap::Object>("on_load");
			if(on_load && on_load.is_callable())
				on_load.invoke();

			result = true;
		}
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

	return result;
}

void Py2Plugin::on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_gnzb_added.is_callable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(m_gnzb_added)};
		handler_thread.detach();
	}
}

void Py2Plugin::on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_gnzb_finished.is_callable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(m_gnzb_finished)};
		handler_thread.detach();
	}
}

void Py2Plugin::on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_gnzb_cancelled.is_callable())
	{
		std::thread handler_thread{run_event_handler, ptr_gnzb, std::ref(m_gnzb_cancelled)};
		handler_thread.detach();
	}
}

/**
 * plugin_load
 *	  gnzb plugins have this called when they are loaded by the gnzb app
 */   
extern "C" void plugin_load()
{
	// load the python interpreter SO
	dlopen(PYLIB_NAME, RTLD_NOW|RTLD_GLOBAL|RTLD_NOLOAD);

	// initialize the python library if needed
	if(!Py_IsInitialized())
	{
		// initialize python
		Py_Initialize();
		PyEval_InitThreads();

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
	class ModulePlugin : public Py2Plugin
	{
	public:
		ModulePlugin() {}
		~ModulePlugin() {}
	};

	return new ModulePlugin();
}

GNzbPluginDescriptor plugin_descriptor =
{
	GNZB_PLUGIN_API_VERSION,

	plugin_load,
	plugin_unload,

	allocate_plugin
};

};
