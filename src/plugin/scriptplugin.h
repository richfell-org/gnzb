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
#ifndef __SCRIPT_PLUGIN_HEADER__
#define __SCRIPT_PLUGIN_HEADER__

#include <memory>
#include "pluginmodule.h"

class GNzb;

/**
 *
 *
 */
class ScriptPlugin : public PluginModule
{
// construction/destruction
public:

	ScriptPlugin();
	ScriptPlugin(ScriptPlugin&& that);
	ScriptPlugin(const ScriptPlugin&) = delete;
	~ScriptPlugin();

// operations
public:

	// add custom tasks when loading and unloading
	void load(const std::string& path) override;
	void unload() override;

	// source file based initialization
	bool initialize_for_source(const std::string& path);

	// scripting handlers for gnzb defined events
	void gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb);
	void gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb);
	void gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb);

	ScriptPlugin& operator =(ScriptPlugin&& that);
	ScriptPlugin& operator =(const ScriptPlugin&) = delete;

// implementation
protected:

	// script plugin script-specific init
	bool (*m_init_for_path)(const std::string&){nullptr};

	// pointers to script plugin functions
	void (*m_on_gnzb_added)(const std::shared_ptr<GNzb>&){nullptr};
	void (*m_on_gnzb_finished)(const std::shared_ptr<GNzb>&){nullptr};
	void (*m_on_gnzb_cancelled)(const std::shared_ptr<GNzb>&){nullptr};
};

#endif  /* __SCRIPT_PLUGIN_HEADER__ */