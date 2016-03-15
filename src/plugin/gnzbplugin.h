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
#ifndef __GNZB_PLUGIN_HEADER__
#define __GNZB_PLUGIN_HEADER__

#include <memory>
#include <string>

#define GNZB_PLUGIN_API_VERSION		2

class GNzb;

/**
 *
 *
 */
class GNzbPlugin
{
// construction/destruction
public:

	GNzbPlugin() {}
	virtual ~GNzbPlugin() {}

// operations
public:

	virtual bool init(const std::string& path) = 0;
	virtual void on_gnzb_added(const std::shared_ptr<GNzb>& ptr_gnzb) = 0;
	virtual void on_gnzb_finished(const std::shared_ptr<GNzb>& ptr_gnzb) = 0;
	virtual void on_gnzb_cancelled(const std::shared_ptr<GNzb>& ptr_gnzb) = 0;
};

extern "C"
{

// module load/unload entry points 
typedef void (*ModuleLoad)();
typedef void (*ModuleUnload)();

// GNzbPlugin instance allocator
typedef GNzbPlugin *(*PluginAllocator)();

typedef struct GNzbPluginDescriptor
{
	int api_version;
	ModuleLoad module_load;
	ModuleUnload module_unload;
	PluginAllocator allocate;
} GNzbPluginDescriptor;

};

#endif  /* __GNZB_PLUGIN_HEADER__ */