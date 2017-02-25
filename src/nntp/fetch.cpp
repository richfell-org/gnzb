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
#include <glibmm/ustring.h>
#include "fetch.h"
#include "articlefetchpool.h"
#include "../movequeue.h"
#include "../db/preferences.h"
#include "fetchmsg.h"

namespace NntpFetch {

// article fetch message queue
MoveQueue<Msg> segment_queue;

void push_message(GNzb *p_gnzb, int i_file)
{
	//segment_queue.insert(Msg(p_gnzb, i_file));
	segment_queue.emplace(p_gnzb, i_file);
}

void push_message(GNzb *p_gnzb, int i_file, int i_segment)
{
	//segment_queue.insert(Msg(p_gnzb, i_file, i_segment));
	segment_queue.emplace(p_gnzb, i_file, i_segment);
}

void clear_message_queue()
{
	segment_queue.clear();
}

bool is_message_queue_empty()
{
	return segment_queue.empty();
}

// The list of article download pools, there will
// be one entry for each enable NNTP server defined
// through the app's preferences interface
std::list<ArticlePool*> fetch_pools;

std::list<ArticlePool*>& get_fetch_pools()
{
	return fetch_pools;
}

// find a pool in the list by NNTP server
std::list<ArticlePool*>::iterator find_pool(const PrefsNntpServer& nntpServer)
{
	std::list<ArticlePool*>::iterator iter = fetch_pools.begin();
	for(; iter != fetch_pools.end(); ++iter)
	{
		if(nntpServer.getDbId() == (*iter)->getPrefsNntpServer().getDbId())
			break;
	}
	return iter;
}

int get_max_connections()
{
	int result = 0;
	for(auto& p_pool : fetch_pools)
		result += p_pool->getPrefsNntpServer().getConnectionCount();
	return result;
}

int get_active_connection_count()
{
	int result = 0;
	for(auto& fetch_pool : fetch_pools)
		result += fetch_pool->getActiveCount();
	return result;
}

bool has_active_connection()
{
	for(auto& fetch_pool : fetch_pools)
	{
		if(fetch_pool->hasActive())
			return true;
	}

	return false;
}

//
// Remove any active NNTP artice fetch pools which are not found or
// are not marked enabled in the given list of servers  
//
void purge_server_pools(const std::vector<PrefsNntpServer>& valid_servers)
{
	std::list<ArticlePool*>::iterator iter = fetch_pools.begin();
	for(; iter != fetch_pools.end(); )
	{
		bool remove_task = true;
		for(auto& valid_server : valid_servers)
		{
			if(valid_server.getDbId() == (*iter)->getPrefsNntpServer().getDbId())
			{
				if(valid_server.isEnabled())
					remove_task = false;
				break;
			}
		}

		if(!remove_task)
			++iter;
		else
		{
			(*iter)->stop();
			delete (*iter);
			iter = fetch_pools.erase(iter);
		}
	}
}

// update the existing pools to match what's in the given NNTP server list
// this method will ignore disabled NNTP servers, use the purge_server_pools
// function to remove existing pools for NNTP servers that are deleted or
// un-checked (disabled) via a run of the preferences dialog
void update_server_pools(const std::vector<PrefsNntpServer>& valid_servers)
{
	for(auto& prefs_server : valid_servers)
	{
		// need to ignore disabled servers
		if(!prefs_server.isEnabled()) continue;

		// look for an existing pool instance for the NNTP server
		std::list<ArticlePool*>::iterator iter_server = find_pool(prefs_server);

		// if found, remove the current instance
		if(iter_server != fetch_pools.end())
		{
			(*iter_server)->stop();
			delete (*iter_server);
			fetch_pools.erase(iter_server);
		}

		// create a new fetch pool instance
		try
		{
			ArticlePool *p_pool = new ArticlePool(prefs_server);
			p_pool->start(segment_queue);
			fetch_pools.push_back(p_pool);
		}
		catch(const std::exception& e)
		{
			Glib::ustring msg = Glib::ustring::compose("Error for NNTP server: %1", prefs_server.getName());
			Glib::ustring secMsg(e.what());
			//show_error(???, msg, secMsg);
		}
	}
}

// abort the current download and rest of the queue but
// keep the thask thread running its task loop
void abort_download()
{
	for(auto& p_pool : fetch_pools)
		p_pool->abort();
}

// terminate all fetch threads for each fetch pool, this should
// really only be called at app shutdown but if followed by a
// call to update_server_pools it should be OK to continue running
void stop_server_pools()
{
	for(auto& p_pool : fetch_pools)
	{
		p_pool->stop();
		delete p_pool;
	}

	fetch_pools.clear();
}

}   // namespace NntpFetch