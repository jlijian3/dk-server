/*
	单向队列 by 陈文清
*/
#ifndef _CWQ_QUEUE_H
#define _CWQ_QUEUE_H
#include <deque>
#include <exception>
#include "mutex.h"


template<class T>
class Queue
{
protected:
	std::deque<T> _d;
	
public:

	typename std::deque<T>::size_type size() const {
		return _d.size();
	}
	
	bool empty() const {
		return _d.empty();
	}
	
	void push(const T& elem) {
		_d.push_back(elem);
	}
	
	T pop() {
		if (_d.empty()) throw std::exception();
		T elem(_d.front());
		_d.pop_front();
		return elem;
	}
	
	T& front() {
		if (_d.empty())	throw std::exception();
		return _d.front();
	}
};


template<class T>
class LockQueue
{
	Mutex	_m;

protected:
	std::deque<T> _d;
	
public:
	
	typename std::deque<T>::size_type size() {
		LOCK lock(_m);
		return _d.size();
	}
	
	bool empty() {
		LOCK lock(_m);
		return _d.empty();
	}
	
	void push(const T& elem) {
		LOCK lock(_m);
		_d.push_back(elem);
	}
	
	T pop() {
		LOCK lock(_m);
		if (_d.empty()) throw std::exception();
		T elem(_d.front());
		_d.pop_front();
		return elem;
	}
	
	T& front() {
		LOCK lock(_m);
		if (_d.empty())	throw std::exception();
		return _d.front();
	}

	inline void push_lock(const T& elem) {
		return push(elem);
	}

	inline T pop_lock() {
		return pop();
	}

};

#endif
