/*
	locker from Intel 2007, update by ChenWenQing 2007
*/
#ifndef _G_MUTEX_H
#define _G_MUTEX_H
#include <pthread.h>
#include <stdexcept>
#include <stdio.h>


class not_copy
{
	not_copy( const not_copy& );
	void operator=( const not_copy& );
public:
	not_copy() {}
};


class Mutex
{
	pthread_mutex_t impl;

	void handle_error( int error_code, const char* what )
	{
		char buf[128];
		sprintf(buf,"%s: ",what);
		char* end = strchr(buf,0);
		size_t n = buf+sizeof(buf)-end;
		strncpy( end, strerror( error_code ), n );
		buf[sizeof(buf)-1] = 0; 
		throw std::runtime_error(buf);
	}

	void inside_construct()
	{
		int error_code = pthread_mutex_init(&impl,NULL);
		if( error_code )
			handle_error(error_code,"mutex: pthread_mutex_init failed");
	}

	void inside_destroy()
	{
		pthread_mutex_destroy(&impl);
	}

public:
	Mutex()
	{
#if _ASSERT
		inside_construct();
#else
		int error_code = pthread_mutex_init(&impl,NULL);
		if( error_code )
			handle_error(error_code,"mutex: pthread_mutex_init failed");
#endif
	};

	~Mutex()
	{
#if _ASSERT
		inside_destroy();
#else
		pthread_mutex_destroy(&impl); 
#endif
	};

	class area_lock;
	friend class area_lock;

	class area_lock : private not_copy
	{
		Mutex* _mutex;

		void inside_acquire( Mutex& m )
		{
			pthread_mutex_lock(&m.impl);
			_mutex = &m;
		}

		bool inside_try_acquire( Mutex& m )
		{
			bool result = pthread_mutex_trylock(&m.impl)==0;
			if( result )	_mutex = &m;
			return result;
		}

		void inside_release()
		{
			pthread_mutex_unlock(&_mutex->impl);
			_mutex = NULL;
		}

	public:
		area_lock() : _mutex(NULL) {};

		area_lock( Mutex& mutex )
		{
			acquire( mutex );
		}

		~area_lock()
		{
			if( _mutex )	release();
		}

		void acquire( Mutex& mutex )
		{
#if _ASSERT
			inside_acquire(mutex);
#else
			_mutex = &mutex;
			pthread_mutex_lock(&mutex.impl);
#endif
		}

		bool try_acquire( Mutex& mutex )
		{
#if _ASSERT
			return inside_try_acquire (mutex);
#else
			bool result = pthread_mutex_trylock(&mutex.impl)==0;
			if( result )
				_mutex = &mutex;
			return result;
#endif
		}

		void release()
		{
#if _ASSERT
			inside_release ();
#else
			pthread_mutex_unlock(&_mutex->impl);
			_mutex = NULL;
#endif
		}
	};
};

#define LOCK	Mutex::area_lock

#endif

