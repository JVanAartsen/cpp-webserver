#ifndef def_Queue
#define def_Queue

#include <mutex>
#include <condition_variable>

// templates/generics are a little weird when compiling to a linkable (or avoiding that process altogether, rather)
// https://stackoverflow.com/questions/495021/why-can-templates-only-be-implemented-in-the-header-file
template <typename T>
class Queue {
private:
	T items[10];
	int front;
	int back;
	bool empty;
	bool full;
	std::mutex mtx;
	std::condition_variable hasRequests;
	std::condition_variable hasRoom;
public:
	Queue() {
		front = 0;
		back = 0;
		empty = true;
		full = false;
	}
	~Queue() {
	}
	T Dequeue(void) { // blocks if empty
		std::unique_lock<std::mutex> ulock(mtx);
		while (this->empty) {
			this->hasRequests.wait(ulock);
		}
		int new_front = (this->front+1)%10;

		if (new_front == this->back) {
			this->empty = true;
		} else {
			this->empty = false;
			this->hasRequests.notify_all();
		}
		T sock = this->items[front];
		this->items[front] = NULL;
		this->front = new_front;
		this->full = false;

		ulock.unlock();
		this->hasRoom.notify_one();
		return sock;
	}
	void Enqueue(T t) { // blocks if full
		std::unique_lock<std::mutex> ulock(mtx);

		while (this->full) {
			this->hasRoom.wait(ulock);
		}
		int new_back = (this->back+1)%10;
		if (new_back == this->front) {
			this->full = true;
		} else {
			this->full = false;
			this->hasRoom.notify_all();
		}
		this->items[back] = t;
		this->back = new_back;
		this->empty = false;


		ulock.unlock();
		this->hasRequests.notify_one();
	};
};



#endif
