/* 
 * Copyright (C) 2001-2005 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SINGLETON_H
#define SINGLETON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

template<typename T>
class Singleton {
public:
	Singleton() { }
	virtual ~Singleton() {}; // if( !instance ) delete instance; }

	static T* me() {
//		dcassert(instance);
		if( !instance )		instance = new T();
		return instance;
	}
	
	static void create() {
		if(instance)	delete instance;
		instance = new T();
	}

	static void cance() {
		if(instance) {
			delete instance;
			instance = 0;
		}
	}

protected:
	static T* instance;
private:
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

};

template<class T> T* Singleton<T>::instance = 0;
#endif // SINGLETON_H

