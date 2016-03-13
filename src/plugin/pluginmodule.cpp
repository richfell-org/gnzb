/*
	gnzb Usenet downloads using NZB index files

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
#include "pluginmodule.h"
#include <stdexcept>
#include <dlfcn.h>

extern "C" int mydlclose(void *h)
{
	int result = dlclose(h);
	return result;
}

PluginModule::PluginModule() throw()
:   m_plugin_load(nullptr),
	m_plugin_unload(nullptr),
	m_ptr_handle(nullptr, mydlclose)
{
}

PluginModule::PluginModule(PluginModule&& that)
:   m_plugin_load(that.m_plugin_load),
	m_plugin_unload(that.m_plugin_unload),
	m_ptr_handle(std::move(that.m_ptr_handle))
{
	that.m_plugin_load = that.m_plugin_unload = nullptr;
}

PluginModule::~PluginModule()
{
	if(m_ptr_handle)
		unload();
}

void PluginModule::load(const std::string& path)
{
	// clear any lingering unchecked error
	dlerror();

	// load the module
	m_path.clear();
	//m_ptr_handle.reset(dlopen(path.c_str(), RTLD_LAZY|RTLD_GLOBAL));
	m_ptr_handle.reset(dlopen(path.c_str(), RTLD_NOW|RTLD_NOLOAD|RTLD_LOCAL));
	if(m_ptr_handle)
		// RTLD_NOLOAD will still bump the reference count even if already loaded
		dlclose(m_ptr_handle.get());
	else
	{
		m_ptr_handle.reset(dlopen(path.c_str(), RTLD_NOW|RTLD_LOCAL));
		if(!m_ptr_handle)
			throw std::runtime_error(dlerror());
	}

	m_path.assign(path);

	// get the plugin load/unload function addresses - if these are not found
	// then the module at the given path is probably not a gnzb DSO plugin
	*(void **)(&m_plugin_load) = dlsym(m_ptr_handle.get(), "plugin_load");
	*(void **)(&m_plugin_unload) = dlsym(m_ptr_handle.get(), "plugin_unload");

	// check for non-null values
	if((m_plugin_load == nullptr) || (m_plugin_unload == nullptr))
	{
		m_path.clear();
		m_ptr_handle.reset();
		m_plugin_load = m_plugin_unload = nullptr;
		throw std::runtime_error(std::string("Invalid plugin") + path + " expected synbols not found");
	}

	// call the plugin initializer
	m_plugin_load();
}

void PluginModule::unload()
{
	if(m_ptr_handle)
	{
		// call the unload function before dlclose
		if(m_plugin_unload != nullptr)
			m_plugin_unload();

		// demote to RTLD_LOCAL 
		//dlopen(m_path.c_str(), RTLD_NOW|RTLD_LOCAL|RTLD_NOLOAD);

		m_path.clear();

		// close the module
		m_ptr_handle.reset();

		// reset load/unload
		m_plugin_load = m_plugin_unload = nullptr;
	}
}

PluginModule& PluginModule::operator =(PluginModule&& that)
{
	// unload if necessary
	if(is_loaded()) unload();

	// move from the passed rvalue instance
	m_path = std::move(that.m_path);
	m_ptr_handle = std::move(that.m_ptr_handle);
	m_plugin_load = that.m_plugin_load;
	m_plugin_unload = that.m_plugin_unload;

	// null out the rvalue's function pointers
	that.m_plugin_load = that.m_plugin_unload = nullptr;
	return *this;
}