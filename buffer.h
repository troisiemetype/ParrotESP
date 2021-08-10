/*
 * This program is part an esp32-based controller to pilot Parrot Minidrones
 * Copyright (C) 2021  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARROT_ESP_BUFFER_H
#define PARROT_ESP_BUFFER_H

#include <Arduino.h>

/*
 *	I believe this file is not used anymore, buffers are implemented directly when needed.
 */

// Note : template classes can be defined in two *.cpp + *.h files as usuall, the delacarations and definitions have to be in the same file !

/*
template <class T>
class CBuffer{
public:
//	CBuffer();
	CBuffer(size_t size);
	~CBuffer();

	bool full();
	bool empty();
	size_t room();

	size_t available();
	size_t size();

	size_t write(T value);
	size_t write(T *values, size_t size);

//	T read();
	size_t read(T *destination, size_t size = 1);

//	T peek();
	size_t peek(T *destination, size_t size = 1);

	void flush();
	
	T* start();
	T* end();
	T* next();

private:
	T* wrap(T *ptr);

	size_t _size;	
	T *_buffer;
	T *_bufferEnd;
	T *_begin;
	T *_end;
};
*/
#endif