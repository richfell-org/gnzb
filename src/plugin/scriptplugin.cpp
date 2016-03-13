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
#include "scriptplugin.h"
#include <dlfcn.h>

ScriptPlugin::ScriptPlugin()
:   PluginModule()
{
}

ScriptPlugin::ScriptPlugin(ScriptPlugin&& that)
:   PluginModule(std::move(that)),
	m_on_gnzb_added(that.m_on_gnzb_added),
	m_on_gnzb_finished(that.m_on_gnzb_finished),
	m_on_gnzb_cancelled(that.m_on_gnzb_cancelled)
{
	// clear out rvalue object's function pointers
	that.m_on_gnzb_added = that.m_on_gnzb_finished = that.m_on_gnzb_cancelled = nullptr;
}

ScriptPlugin::~ScriptPlugin()
{
}

void ScriptPlugin::load(const std::string& path)
{
	// call base class load
	PluginModule::load(path);

	dlerror();  // clear any stagnant unckeck error

	// A DSO plugin module for scripting must have
	// these entry points defined

	// look for the script init function
	*(void **)(&m_init_for_path) = dlsym(get_dso_handle(), "script_plugin_init");
	if(m_init_for_path == nullptr)
		throw std::runtime_error(dlerror());

	// look for the GNzb added function
	*(void **)(&m_on_gnzb_added) = dlsym(get_dso_handle(), "on_gnzb_added");
	if(m_on_gnzb_added == nullptr)
	{
		m_init_for_path = nullptr;
		throw std::runtime_error(dlerror());
	}

	// look for the GNzb finished function
	*(void **)(&m_on_gnzb_finished) = dlsym(get_dso_handle(), "on_gnzb_finished");
	if(m_on_gnzb_finished == nullptr)
	{
		m_on_gnzb_added = nullptr;
		throw std::runtime_error(dlerror());
	}

	// look for the GNzb cancelled handler
	*(void **)(&m_on_gnzb_cancelled) = dlsym(get_dso_handle(), "on_gnzb_cancelled");
	if(m_on_gnzb_cancelled == nullptr)
	{
		m_on_gnzb_added = m_on_gnzb_finished = nullptr;
		throw std::runtime_error(dlerror());
	}
}

void ScriptPlugin::unload()
{
	PluginModule::unload();
	m_on_gnzb_added = m_on_gnzb_finished = m_on_gnzb_cancelled = nullptr;
}

bool ScriptPlugin::initialize_for_source(const std::string& path)
{
	if(m_init_for_path == nullptr) return false;

	return m_init_for_path(path);
}

void ScriptPlugin::gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_on_gnzb_added == nullptr) return;

	m_on_gnzb_added(ptr_gnzb);
}

void ScriptPlugin::gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_on_gnzb_finished == nullptr) return;

	m_on_gnzb_finished(ptr_gnzb);
}

void ScriptPlugin::gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb)
{
	if(m_on_gnzb_cancelled == nullptr) return;

	m_on_gnzb_cancelled(ptr_gnzb);
}

ScriptPlugin& ScriptPlugin::operator =(ScriptPlugin&& that)
{
	PluginModule::operator =(std::move(that));
	m_init_for_path = that.m_init_for_path;
	m_on_gnzb_added = that.m_on_gnzb_added;
	m_on_gnzb_finished = that.m_on_gnzb_finished;
	m_on_gnzb_cancelled = that.m_on_gnzb_cancelled;

	that.m_init_for_path = nullptr;
	that.m_on_gnzb_added = that.m_on_gnzb_finished = that.m_on_gnzb_cancelled = nullptr;

	return *this;
}