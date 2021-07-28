#ifndef PARROT_ESP_BUFFER_H
#define PARROT_ESP_BUFFER_H

#include <Arduino.h>
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