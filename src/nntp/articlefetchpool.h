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
#ifndef __NNTP_ARTICLE_FETCH_POOL_HEADER__
#define __NNTP_ARTICLE_FETCH_POOL_HEADER__

#include <vector>
#include <tuple>
#include <memory>
#include <thread>
#include <libusenet/nntpclient.h>
#include "../movequeue.h"
#include "../db/preferences.h"
#include "fetchmsg.h"

namespace NntpFetch {

class ArticleTask;

using TaskPtr = std::unique_ptr<ArticleTask>;
using TaskTuple = std::tuple<TaskPtr, std::thread>;
using TaskVector = std::vector<TaskTuple>;

/*
 * 
 */
class ArticlePool
{
public:

	ArticlePool(const PrefsNntpServer& nntp_server)
		throw(std::system_error);
	~ArticlePool();

// attributes
public:

	const PrefsNntpServer& getPrefsNntpServer() const { return m_prefs_nntp_server; }
	const NntpClient::ServerAddr& getNntpServer() const { return m_nntp_server; }

// operations
public:

	void start(MoveQueue<NntpFetch::Msg>& msg_queue);
	void abort();
	void stop();

	int getActiveCount() const;
	bool hasActive() const;

// implementation
protected:

	void start_fetch_tasks(MoveQueue<NntpFetch::Msg>& msg_queue);

	// NNTP server data structures
	PrefsNntpServer m_prefs_nntp_server;
	NntpClient::ServerAddr m_nntp_server;

	// fetch task thread vector
	TaskVector mTasks;
	static TaskPtr& getTaskPtr(TaskTuple& tuple);
	static const TaskPtr& getTaskPtr(const TaskTuple& tuple);
	static std::thread& getTaskThread(TaskTuple& tuple);
};

inline TaskPtr&
ArticlePool::getTaskPtr(TaskTuple& tuple)
{ return std::get<0>(tuple); }

inline const TaskPtr&
ArticlePool::getTaskPtr(const TaskTuple& tuple)
{ return std::get<0>(tuple); }

inline std::thread&
ArticlePool::getTaskThread(TaskTuple& tuple)
{ return std::get<1>(tuple); }

}   // namespace NntpFetch

#endif  /* __NNTP_ARTICLE_FETCH_POOL_HEADER__ */