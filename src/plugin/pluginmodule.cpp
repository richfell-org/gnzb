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
:   m_ptr_handle(nullptr, mydlclose)
{
}

PluginModule::PluginModule(PluginModule&& that)
:   m_path(std::move(that.m_path)),
	m_ptr_handle(std::move(that.m_ptr_handle)),
	mp_plugin_desc(that.mp_plugin_desc),
	m_ptr_plugin(std::move(that.m_ptr_plugin))
{
	that.mp_plugin_desc = nullptr;
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
	m_ptr_handle.reset(dlopen(path.c_str(), RTLD_LAZY|RTLD_NOLOAD|RTLD_LOCAL));
	if(m_ptr_handle)
		// RTLD_NOLOAD will still bump the reference count even if already loaded
		dlclose(m_ptr_handle.get());
	else
	{
		m_ptr_handle.reset(dlopen(path.c_str(), RTLD_LAZY|RTLD_LOCAL));
		if(!m_ptr_handle)
			throw std::runtime_error(dlerror());
	}

	// find the plugin descriptor
	mp_plugin_desc = (GNzbPluginDescriptor*)dlsym(m_ptr_handle.get(), "plugin_descriptor");
	if(nullptr == mp_plugin_desc)
	{
		m_ptr_handle.reset();
		throw std::runtime_error(std::string("Invalid plugin") + path + " expected synbols not found");
	}

	// check API version
	if(GNZB_PLUGIN_API_VERSION != mp_plugin_desc->api_version)
	{
		mp_plugin_desc = nullptr;
		m_ptr_handle.reset();
		throw std::runtime_error(path + ": API mismatch");
	}

	m_path.assign(path);

	// call the plugin initializer
	if(nullptr != mp_plugin_desc->module_load)
		mp_plugin_desc->module_load();

	// allocate the plugin instance
	if(nullptr != mp_plugin_desc->allocate)
		m_ptr_plugin.reset(mp_plugin_desc->allocate());
}

void PluginModule::unload()
{
	if(m_ptr_handle)
	{
		// deallocate the plugin instance
		m_ptr_plugin.reset();

		// call the unload function before dlclose
		if(nullptr != mp_plugin_desc->module_unload)
			mp_plugin_desc->module_unload();

		// close the module
		m_ptr_handle.reset();

		// clear now stale data
		mp_plugin_desc = nullptr;
		m_path.clear();
	}
}

PluginModule& PluginModule::operator =(PluginModule&& that)
{
	// move from the passed rvalue instance
	m_path = std::move(that.m_path);
	m_ptr_handle = std::move(that.m_ptr_handle);
	mp_plugin_desc = that.mp_plugin_desc;
	m_ptr_plugin = std::move(that.m_ptr_plugin);

	// null out the rvalue's pointer to the module's plugin descriptor
	that.mp_plugin_desc = nullptr;
	return *this;
}