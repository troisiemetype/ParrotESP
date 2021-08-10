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

#include "buffer.h"

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

template <class T>
CBuffer<T>::CBuffer(size_t size) :
_size(size),
_buffer(new T[size + 1]),
_bufferEnd(_buffer + size + 1),
_begin(_buffer),
_end(_buffer){

}

template <class T>
CBuffer<T>::~CBuffer(){
	delete[] _buffer;
}

template <class T>
bool CBuffer<T>::full(){
	return wrap(_end + 1) == _begin;
}

template <class T>
bool CBuffer<T>::empty(){
	return _end == _begin;
}

template <class T>
size_t CBuffer<T>::room(){
	return _size - available();
}

template <class T>
size_t CBuffer<T>::available(){
	if(_end > _begin) return _end - _begin;
	return size + 1 - (_begin - _end);
}

template <class T>
size_t CBuffer<T>::size(){
	return _size;
}

template <class T>
size_t CBuffer<T>::write(T value){
	if(full()) return 0;
	*_end = value;
	_end = wrap(_end + 1);
	return 1;
}

template <class T>
size_t CBuffer<T>::write(T *source, size_t size){
	if(full()) return 0;
	size_t toWrite = (size > room()) ? room() : size;
	if((_end > _begin) && (toWrite > (size_t)(_bufferEnd - _end))){
		size_t limit = _bufferEnd - _end;
		memcpy(_end, source, limit);
		_end = _buffer;
		size -= limit;
		source += limit;
	}
	memcpy(_end, source, size);
	_end = wrap(_end + size);
	return toWrite;
}
/*
template <class T>
T CBuffer<T>::read(){
	if(empty()) return
}
*/
template <class T>
size_t CBuffer<T>::read(T *destination, size_t size){
	if(empty()) return 0;
	size_t toRead = (size > available()) ? available() : size;
	if((_begin > _end) && (toRead > (size_t)(_bufferEnd - _begin))){
		size_t limit = _bufferEnd - _begin;
		memcpy(destination, _begin, limit);
		_begin = _buffer;
		size -= limit;
		destination += limit;
	}
	memcpy(destination, _begin, size);
	_begin = wrap(_begin + size);
	return toRead;
}
/*
template <class T>
T CBuffer<T>::peek(){

}
*/
template <class T>
size_t CBuffer<T>::peek(T *destination, size_t size){
	if(empty()) return 0;
	size_t toPeek = (size > available()) ? available() : size;
	T *begin = _begin;
	if((begin > _end) && (toPeek > (size_t)(_bufferEnd - begin))){
		size_t limit = _bufferEnd - begin;
		memcpy(destination, begin, limit);
		begin = _buffer;
		size -= limit;
		destination += limit;
	}
	memcpy(destination, begin, size);
	return toPeek;
}

template <class T>
void CBuffer<T>::flush(){
	_begin = _end = _buffer;
}

template <class T>
T* CBuffer<T>::start(){
	return _begin;
}

template <class T>
T* CBuffer<T>::end(){
	return _end;
}

template <class T>
T* CBuffer<T>::next(){
	return wrap(_end + 1);
}

// Private methods

template <class T>
T* CBuffer<T>::wrap(T *ptr){
	if(ptr == _bufferEnd) ptr = _buffer;
	return ptr;
}
