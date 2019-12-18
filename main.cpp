#include <thread>
#include <mutex>
#include <iostream>
#include <chrono>
#include <cstring>
#include "gtu_mutex/mutex.h"
gtu::mutex m1, m2, m3;

#define SLEEP_TIMER 1
void f()
{
    std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	std::unique_lock<gtu::mutex> a1(m1);
	std::unique_lock<gtu::mutex> a2(m2);
}

void g()
{
	std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	std::unique_lock<gtu::mutex> a2(m2);
	std::unique_lock<gtu::mutex> a1(m1);
}

void h()
{
	std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	std::unique_lock<gtu::mutex> a3(m3);
}

int main()
{
	#ifdef UNIX_SYSTEM
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(0, &cpuset);
		sched_param priority_param;
		pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
	#elif defined(WINDOWS_SYSTEM)
		DWORD_PTR mask_ptr = 1;
		SetProcessAffinityMask(GetCurrentProcess(), mask_ptr);
	#endif
	for(int i = 0; i < 1000; ++i)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		#ifdef UNIX_SYSTEM
			priority_param.__sched_priority = sched_get_priority_max(SCHED_FIFO);
			if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_param)) {
				std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
			}
		#endif
		std::thread a(f), b(g), c(h);
		std::cout << "create\n";
		m1.register_(a, 20);
		std::cout << "reg a1\n";
		m1.register_(b, 30);
		std::cout << "reg b1\n";
		m2.register_(a, 20);
		std::cout << "reg a2\n";
		m2.register_(b, 30);
		std::cout << "reg 2\n";
		m3.register_(c, 10);
		std::cout << "reg 3\n";
		#ifdef UNIX_SYSTEM
			priority_param.__sched_priority = sched_get_priority_min(SCHED_FIFO);
			if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_param)) {
				std::cout << "Failed to setschedparam: " << std::strerror(errno) << '\n';
			}
			sched_yield();
		#endif
		std::this_thread::yield();
		
		a.join();
		std::cout << "dead 1\n";
		b.join();
		std::cout << "dead 2\n";
		c.join();
		std::cout << "dead 3\n";
	}
}