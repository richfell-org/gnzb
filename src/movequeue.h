#ifndef MOVE_QUEUE_HEADER__
#define MOVE_QUEUE_HEADER__

#include <stdexcept>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <type_traits>

/*
 *
 */
class MoveQueueError : std::runtime_error
{
public:
	MoveQueueError(const char *m) : std::runtime_error(m) {}
	~MoveQueueError() {}
};

/*
 * thread safe queue for move-able types. 
 */
template<
	typename T,
	class = typename std::enable_if<std::is_move_constructible<T>::value>::type,
	class = typename std::enable_if<std::is_move_assignable<T>::value>::type>
class MoveQueue
{
// types
public:

	typedef T	ItemType;

// construction
public:

	MoveQueue() { mDeque.clear(); }

	virtual ~MoveQueue() {}

	/*
	 * Check for an empty queue condition.
	 * 
	 * return true if the queue is empty, false otherwise.
	 */
	bool empty()
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		return mDeque.empty();
	}

	/*
	 * Check for a full queue condition.
	 * 
	 * return true if the queue is full, false otherwise.
	 */
	bool full()
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		return mDeque.size() == mDeque.max_size();
	}

	/*
	 * Get the number of items in the queue.
	 * 
	 * return The number of items in the queue.
	 */
	int count()
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		return mDeque.size();
	}

	/*
	 * Reference the item at the front of the queue.  If the queue is
	 * empty an exception is thrown.
	 * 
	 * return a reference to the item at the front of the queue.
	 * throws MoveQueueError if the queue is empty.
	 */
	T& front()
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		if(mDeque.empty())
			throw MoveQueueError("MsgQueue::front called when empty");
		return mDeque.front();
	}

	/*
	 * Reference the item at the front of the queue.  If the queue is
	 * empty an exception is thrown.
	 * 
	 * return a const reference to the item at the front of the queue.
	 * throws MoveQueueError if the queue is empty.
	 */
	T const& front() const
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		if(mDeque.empty())
			throw MoveQueueError("MsgQueue::front called when empty");
		return mDeque.front();
	}

	/*
	 * Reference the item at the back of the queue.  If the queue is
	 * empty an exception is thrown.
	 * 
	 * return a reference to the item at the back of the queue.
	 * throws MoveQueueError if the queue is empty.
	 */
	T& rear()
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		if(mDeque.empty())
			throw MoveQueueError("MsgQueue::rear called when empty");
		return mDeque.rear();
	}

	/*
	 * Reference the item at the back of the queue.  If the queue is
	 * empty an exception is thrown.
	 * 
	 * return a const reference to the item at the back of the queue.
	 * throws MoveQueueError if the queue is empty.
	 */
	T const& rear() const
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		if(mDeque.empty())
			throw MoveQueueError("MsgQueue::rear called when empty");
		return mDeque.rear();
	}

	/*
	 * Remove the item at the front of the queue.  The item is moved
	 * to the given location and then poped from the queue.
	 * 
	 * itemBuf The instance to which the front item is moved.
	 */
	void remove(T& itemBuf)
	{
		std::unique_lock<std::mutex> qLock(mQueueMutex);
		while(0 == mDeque.size())
			mQueueCond.wait(qLock);
		itemBuf = std::move(mDeque.front());
		mDeque.pop_front();
	}

	/*
	 * Remove the item at the front of the queue if the queue is not
	 * empty.
	 * 
	 * itemBuf The instance to which the front item is moved.
	 * 
	 * return true if an item was removed from the queue and moved to itemBuf,
	 *        false otherwise.
	 */
	bool try_remove(T& itemBuf)
	{
		std::unique_lock<std::mutex> qLock(mQueueMutex);

		// get front item if deque is not empty
		if(!mDeque.empty())
		{
			itemBuf = std::move(mDeque.front());
			mDeque.pop_front();
			return true;
		}

		// else deque was empty
		return false;
	}

	/*
	 * Remove the item at the front of the queue if the queue is not
	 * empty.  If the queue is empty, wait up to milliSecs milliseconds
	 * for an item to be available.
	 * 
	 * itemBuf The instance to which the front item is moved.
	 * 
	 * return true if an item was removed from the queue and moved to itemBuf,
	 *        false otherwise.
	 */
	bool try_remove(T& itemBuf, int milliSecs)
	{
		std::unique_lock<std::mutex> qLock(mQueueMutex);
		if(0 >= mDeque.size())
		{
			std::chrono::system_clock::time_point expire
				= std::chrono::system_clock::now() + std::chrono::milliseconds(milliSecs);
			while(0 >= mDeque.size())
			{
				if(std::cv_status::timeout == mQueueCond.wait_until(qLock, expire))
					return false;
			}
		}
		itemBuf = std::move(mDeque.front());
		mDeque.pop_front();
		return true;
	}

	/*
	 * Remove all items from the queue.
	 */
	void clear()
	{
		std::unique_lock<std::mutex> qLock(mQueueMutex);
		mDeque.clear();
	}

	/*
	 * Copies the given item into queue at the back.
	 */
	void insert(T& item)
	{
		// TODO:  throw on full queue

		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.push_back(item);
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

	/*
	 * Moves the given item into the queue at the back.
	 */
	void insert(T&& item)
	{
		// TODO:  throw on full queue

		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.push_back(std::move(item));
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

	/*
	 * Contructs an item, in-place, at the back of the queue using
	 * the passed arguments for item construction.
	 */
	template<class... Args>
	void emplace(Args&&... args)
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.emplace_back(std::forward<Args>(args)...);
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

	/*
	 * Copies the given item into the queue at the front.
	 */
	void insert_front(T& item)
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.push_front(item);
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

	/*
	 * Moves the given item into the queue at the front.
	 */
	void insertFront(T&& item)
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.push_front(std::move(item));
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

	/*
	 * Contructs an item, in-place, at the front of the queue using
	 * the passed arguments for item construction.
	 */
	template<class... Args>
	void emplace_front(Args&&... args)
	{
		std::lock_guard<std::mutex> qLock(mQueueMutex);
		unsigned int sz = mDeque.size();
		if(sz != mDeque.max_size())
		{
			mDeque.emplace_front(std::forward<Args>(args)...);
			if(0 == sz)
				mQueueCond.notify_all();
		}
	}

// implementation
protected:

	std::deque<T> mDeque;

	std::mutex mQueueMutex;
	std::condition_variable mQueueCond;
};

#endif  /* MOVE_QUEUE_HEADER__ */
