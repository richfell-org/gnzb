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
#include <sched.h>
#include <chrono>
#include <cerrno>
#include <cstring>
#include "../db/nzbdb.h"
#include "articlefetchpool.h"
#include "articlefetchtask.h"

#include <iostream>

namespace NntpFetch {

ArticlePool::ArticlePool(const PrefsNntpServer& nntp_server)
throw(std::system_error)
:   m_prefs_nntp_server(nntp_server),
	m_nntp_server(nntp_server.getUrl().c_str(), nntp_server.getConnectionCount(), nntp_server.getPort().c_str()),
	mTasks(nntp_server.getConnectionCount())
{
	// set any existing username/password info
	if(!nntp_server.getUsername().empty())
		m_nntp_server.set_username(nntp_server.getUsername());
	if(!nntp_server.getPassword().empty())
		m_nntp_server.set_password(nntp_server.getPassword());
}

ArticlePool::~ArticlePool()
{
}

void ArticlePool::start(MsgQueue<NntpFetch::Msg>& msg_queue)
{
	start_fetch_tasks(msg_queue);
}

void ArticlePool::abort()
{
	for(auto& task_tuple : mTasks)
	{
		TaskPtr& ptr_task = getTaskPtr(task_tuple);
		if(ptr_task)
			ptr_task->request_abort();
	}

	bool bSomeAreDownloading;
	do
	{
		bSomeAreDownloading = false;
		std::this_thread::sleep_for(std::chrono::microseconds(100));

		// wait for downloading to stop
		for(auto& task_tuple : mTasks)
		{
			TaskPtr& ptr_task = getTaskPtr(task_tuple);
			if(ptr_task)
				bSomeAreDownloading |= ptr_task->isDownloading();
		}

	} while(bSomeAreDownloading);

	// reset abort for all threads
	for(auto& task_tuple : mTasks)
	{
		TaskPtr& ptr_task = getTaskPtr(task_tuple);
		if(ptr_task)
			ptr_task->reset_abort();
	}
}

void ArticlePool::stop()
{
	// request quit for all threads
	for(auto& task_tuple : mTasks)
	{
		if(getTaskThread(task_tuple).joinable())
			getTaskPtr(task_tuple)->request_quit();
	}

	// wait for threads to terminate
	for(auto& task_tuple : mTasks)
	{
		std::thread& task_thread = getTaskThread(task_tuple);
		if(task_thread.joinable())
			task_thread.join();
	}
}

int ArticlePool::getActiveCount() const
{
	int result = 0;
	for(auto& task_tuple : mTasks)
	{
		if(getTaskPtr(task_tuple)->isDownloading())
			++result;
	}
	return result;
}

bool ArticlePool::hasActive() const
{
	for(auto& task_tuple : mTasks)
	{
		if(getTaskPtr(task_tuple)->isDownloading())
			return true;
	}

	return false;
}

void ArticlePool::start_fetch_tasks(MsgQueue<NntpFetch::Msg>& msg_queue)
{
	const unsigned int cpu_count = std::thread::hardware_concurrency();
	if(cpu_count > 1)
	{
		unsigned int cpu_id = 0;
		cpu_set_t cpu_set;

		for(auto& task_tuple : mTasks)
		{
			// fetch task instance
			TaskPtr& ptr_task = getTaskPtr(task_tuple);
			ptr_task.reset(m_prefs_nntp_server.useSsl()
				? new SslArticleTask(m_nntp_server, msg_queue)
    			: new ArticleTask(m_nntp_server, msg_queue));


			// run task in separate thread
			std::thread& task_thread = getTaskThread(task_tuple);
			task_thread = std::thread{&ArticleTask::run, ptr_task.get()};

			// assign threads to alternating CPUs
			CPU_ZERO(&cpu_set);
			CPU_SET(cpu_id, &cpu_set);
			std::thread::native_handle_type h_thread = task_thread.native_handle();
			int result = pthread_setaffinity_np(h_thread, sizeof(cpu_set_t), &cpu_set);
			if(0 != result)
				std::cerr << "ArticlePool::start_fetch_tasks: " << strerror(result) << std::endl;

			++cpu_id %= cpu_count;
		}
	}
	else
	{
		// allocate task instances and start threads for their "run" methods
		for(auto& task_tuple : mTasks)
		{
			TaskPtr& ptr_task = getTaskPtr(task_tuple);
			ptr_task.reset(m_prefs_nntp_server.useSsl()
				? new SslArticleTask(m_nntp_server, msg_queue)
				: new ArticleTask(m_nntp_server, msg_queue));

			std::thread& task_thread = getTaskThread(task_tuple);
			task_thread = std::thread{&ArticleTask::run, ptr_task.get()};
		}
	}
}

}   // namespace NntpFetch