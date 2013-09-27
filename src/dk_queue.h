#ifndef __DK_QUEUE_H__
#define __DK_QUEUE_H__
#include <queue>
#include <exception>
#include "dk_lock.h"



template<class T>
class LockQueue
{
	Mutex	mtx_;

protected:
	std::queue<T> q_;
	
public:
	
	typename std::queue<T>::size_type size() {
		Lock lock(mtx_);
		return q_.size();
	}
	
	bool empty() {
		Lock lock(mtx_);
		return q_.empty();
	}
	
	void push(const T& elem) {
		Lock lock(mtx_);
		q_.push(elem);
	}
	
	T pop() {
		Lock lock(mtx_);
		if (q_.empty()) throw std::exception();
		T elem(q_.front());
		q_.pop();
		return elem;
	}
	
	T& front() {
		Lock lock(mtx_);
		if (q_.empty())	throw std::exception();
		return q_.front();
	}
};

#endif
