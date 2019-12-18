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
	//std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	try
	{
		std::unique_lock<gtu::mutex> a1(m1);
		std::unique_lock<gtu::mutex> a2(m2);
	}
	catch (std::exception & a )
	{
		std::cerr << a.what() << std::endl;
	}
}

void g()
{
	//std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	try
	{
		std::unique_lock<gtu::mutex> a2(m2);
		std::unique_lock<gtu::mutex> a1(m1);
	}
	catch (std::exception & a )
	{
		std::cerr << a.what() << std::endl;
	}
}

void h()
{
	//std::this_thread::sleep_for(std::chrono::seconds(SLEEP_TIMER));
	try
	{
		std::unique_lock<gtu::mutex> a3(m3);
	}
	catch (std::exception & a )
	{
		std::cerr << a.what() << std::endl;
	}
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
	try
	{
		for(int i = 0; i < 100000; ++i)
		{
			std::this_thread::sleep_for(std::chrono::nanoseconds(10));
			#ifdef UNIX_SYSTEM
			priority_param.__sched_priority = sched_get_priority_max(SCHED_FIFO);
			if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_param)) {
				std::cerr << "Failed to setschedparam: " << std::strerror(errno) << '\n';
			}
			#endif
			std::thread a(f), b(g), c(h);
			m1.register_(a, 20);
			m1.register_(b, 30);
			m2.register_(a, 20);
			m2.register_(b, 30);
			m3.register_(c, 10);
			#ifdef UNIX_SYSTEM
				priority_param.__sched_priority = sched_get_priority_min(SCHED_FIFO);
				if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_param)) {
					std::cerr << "Failed to setschedparam: " << std::strerror(errno) << '\n';
				}
				sched_yield();
			#endif
			std::this_thread::yield();
			a.join();
			b.join();
			c.join();
			m1.unregister();
			m2.unregister();
			m3.unregister();
			//int q;
			//std::cin >> q;
		}
	}
	catch (std::exception & a )
	{
		std::cerr << a.what() << std::endl;
	}

}