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
#ifndef __MSG_QUEUE_HEADER__
#define __MSG_QUEUE_HEADER__

#include <stdexcept>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>

/*
 *
 */
class MsgQueueError : std::runtime_error
{
public:
	MsgQueueError(const char *m) : std::runtime_error(m) {}
	~MsgQueueError() {}
};

/*
 * 
 */
template<typename T>
class MsgQueue
{
// construction
public:

	MsgQueue() { m_deque.clear(); }

	virtual ~MsgQueue() {}

	bool empty()
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		return m_deque.empty();
	}

	bool full()
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		return m_deque.size() == m_deque.max_size();
	}

	int count()
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		return m_deque.size();
	}

	T& front()
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		if(m_deque.empty())
			throw MsgQueueError("MsgQueue::front called when empty");
		return m_deque.front();
	}

	T& rear()
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		if(m_deque.empty())
			throw MsgQueueError("MsgQueue::rear called when empty");
		return m_deque.rear();
	}

	void remove(T& item_buf)
	{
		std::unique_lock<std::mutex> q_lock(m_queue_mutex);
		while(0 == m_deque.size())
			m_queue_cond.wait(q_lock);
		item_buf = m_deque.front();
		m_deque.pop_front();
	}

	bool try_remove(T& item_buf, int millisecs)
	{
		std::unique_lock<std::mutex> q_lock(m_queue_mutex);
		if(0 >= m_deque.size())
		{
			std::chrono::steady_clock::time_point until
				= std::chrono::steady_clock::now() + std::chrono::milliseconds(millisecs);

			while(0 >= m_deque.size())
			{
				if(std::cv_status::timeout == m_queue_cond.wait_until(q_lock, until))
					return false;
			}
		}
		item_buf = m_deque.front();
		m_deque.pop_front();
		return true;
	}

	void clear()
	{
		std::unique_lock<std::mutex> q_lock(m_queue_mutex);
		m_deque.clear();
	}

	void insert(T& item)
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		unsigned int sz = m_deque.size();
		if(sz != m_deque.max_size())
		{
			m_deque.push_back(item);
			if(0 == sz)
				m_queue_cond.notify_all();
		}
	}

	// using T&& instead of T avoids an ambiguous overload error from compilier
	void insert(T&& item)
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		unsigned int sz = m_deque.size();
		if(sz != m_deque.max_size())
		{
			m_deque.push_back(item);
			if(0 == sz)
				m_queue_cond.notify_all();
		}
	}

	void insert_front(T& item)
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		unsigned int sz = m_deque.size();
		if(sz != m_deque.max_size())
		{
			m_deque.push_front(item);
			if(0 == sz)
				m_queue_cond.notify_all();
		}
	}

	// using T&& instead of T avoids an ambiguous overload error from compilier
	void insert_front(T&& item)
	{
		std::lock_guard<std::mutex> q_lock(m_queue_mutex);
		unsigned int sz = m_deque.size();
		if(sz != m_deque.max_size())
		{
			m_deque.push_front(item);
			if(0 == sz)
				m_queue_cond.notify_all();
		}
	}

// implementation
protected:

	std::deque<T> m_deque;

	std::mutex m_queue_mutex;
	std::condition_variable m_queue_cond;
};

#endif  /* __MSG_QUEUE_HEADER__ */