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
#ifndef __PLUGIN_MODULE_HEADER__
#define __PLUGIN_MODULE_HEADER__

#include <memory>
#include <string>

/**
 * 
 * 
 */
class PluginModule
{
// construction/destruction
public:

	PluginModule() throw();
	PluginModule(PluginModule&& that);
	PluginModule(const PluginModule&) = delete;
	virtual ~PluginModule();

// attributes
public:

	// the path of the loaded module
	const std::string& path() const { return m_path; }

// operations
public:

	bool is_loaded() const { return bool(m_ptr_handle); }

	// load a plugin module
	// throws std::runtime_error if module load fails
	virtual void load(const std::string& path);

	// unload the plugin module
	virtual void unload();

	PluginModule& operator =(PluginModule&&);
	PluginModule& operator =(const PluginModule&) = delete;

// implementation
protected:

	void *get_dso_handle() const { return m_ptr_handle.get(); }

private:

	// plugin load/unload entry points
	void (*m_plugin_load)();
	void (*m_plugin_unload)();

	// module path
	std::string m_path;

	// handle to module
	std::unique_ptr<void, int(*)(void*)> m_ptr_handle;
};

#endif  /* __PLUGIN_MODULE_HEADER__ */