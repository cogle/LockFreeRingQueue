#pragma once

#include <atomic>
#include <memory>

#include <iostream>
template <typename T> class RingBuffer {
public:

	explicit RingBuffer(std::size_t capacity) : _bufferCapacity(capacity), _ringBufferArray(new T[capacity + 1]) {

	}

	~RingBuffer() {

	}

	/*
	Returns True if the push to buffer was successful.
	If the push is unsuccessful returns False.
	This design choice allows the user to be notified of whether or not
	the transaction was completed. If it was not able to complete then the
	user can sleep/yield or perform other work in the calling thread.
	*/
	
	template<typename U>
	bool push(U && val)
	{
		return tryPush(std::forward<U>(val));
	}
	

	/*
	Returns a shared_ptr to the value that is popped off the thread. If there is nothing
	in the queue or the queue is full then the user is returned a nullptr, and can proceed accordingly.
	*/
	std::shared_ptr<T> pop() {
		std::shared_ptr<T> ret  = tryPop();
		return ret;
	}

	std::size_t maxCapacity() {
		return _bufferCapacity;
	}

	std::size_t curCapacity() {
		return _curCapacity.load(std::memory_order_acquire);
	}

private:

	//Private Struct to ensure allocated memory is freed.
	struct RingBufferFree {
		void operator()(T * ring_buffer_memory) { delete[] ring_buffer_memory; }
	};

	//Private Member Functions
	template<typename U>
	bool tryPush(U && val) {
		const std::size_t currentWrite = _writePosition.load(std::memory_order_acquire);
		const std::size_t currentRead = _readPosition.load(std::memory_order_acquire);
		const std::size_t nextWrite = incrementIndex(currentWrite);

		if (nextWrite == currentRead) { return false; }

		_ringBufferArray[currentWrite] = std::move(val);
		_writePosition.store(nextWrite, std::memory_order_release);

		return true;
	}

	std::shared_ptr<T> tryPop() {
		std::shared_ptr<T> val(nullptr);
		const std::size_t currentWrite = _writePosition.load(std::memory_order_acquire);
		const std::size_t currentRead = _readPosition.load(std::memory_order_acquire);

		if (currentRead == currentWrite) { return val; }

		val = std::make_shared<T>(_ringBufferArray[currentRead]);

		const std::size_t nextRead = incrementIndex(currentRead);
		_readPosition.store(nextRead, std::memory_order_release);
		return val;
	}

	std::size_t incrementIndex(std::size_t index) {
		return (index + 1) % _bufferCapacity;
	}

	//Private Member Variables
	std::atomic<std::size_t> _curCapacity = 0;
	std::atomic<std::size_t> _readPosition = 0;
	std::atomic<std::size_t> _writePosition = 0;

	std::size_t _bufferCapacity;
	std::unique_ptr<T[], RingBufferFree> _ringBufferArray;
};