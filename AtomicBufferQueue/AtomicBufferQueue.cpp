// AtomicBufferQueue.cpp : Defines the entry point for the console application.

/*
This is my lock free implementation of a Ring Queue is not suitable for multiple producer consumers.
Single Producer and Consumer Only.
*/

#include "stdafx.h"
#include <iostream>

#include "RingBuffer.h"

int main(int argc, char * argv[])
{
	/*
	Example Usage here
	*/
	RingBuffer<int> r { 20 };
	for (unsigned int i = 0; i < 10; i++) {
		r.push(10);
	}
	int temp = 1;
	for (unsigned int i = 0; i < 10; i++) {
		r.push(temp);
	}

	for (int i = 0; i < 20; i++) {
		std::shared_ptr<int>  val = r.pop();
		if ( val == nullptr) { std::cout << "HERE" << std::endl; }
		else {
			std::cout << "Index: " << i << " element: " << *val << std::endl;
		}
	}
	return 0;
}

