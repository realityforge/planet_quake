
#ifndef LOCKBOOK_H
#define LOCKBOOK_H

#include <algorithm>
#include <list>
#include <boost/thread/mutex.hpp>

namespace rapido
{
	template<class T>
	class LockBook
	{
		boost::mutex _mutex;
		std::list<T *> _lockedEntities;

		// copying and assignment are not allowed
		LockBook(const LockBook &other);
		LockBook &operator=(const LockBook &other);

	public:
		LockBook() { }

		inline bool Lock(T *entity)
		{
			boost::mutex::scoped_lock lock(_mutex);
			bool free = (std::find(_lockedEntities.begin(), _lockedEntities.end(), entity) == _lockedEntities.end());
			if(free) _lockedEntities.push_back(entity);
			return free;
		}

		inline void Unlock(T *entity)
		{
			boost::mutex::scoped_lock lock(_mutex);
			_lockedEntities.erase(std::find(_lockedEntities.begin(), _lockedEntities.end(), entity));
		}
	};
}

#endif // LOCKBOOK_H

