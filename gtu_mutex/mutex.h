//
// Created by MSI on 13.12.2019.
//

#ifndef PRIORITY_CEILING_PROTOCOL_MUTEX_H
	#define PRIORITY_CEILING_PROTOCOL_MUTEX_H
	#include <mutex>
	#include <thread>
	#include <stdexcept>
	#include <forward_list>
	#include <set>
	#ifdef _WIN32
		#define WINDOWS_SYSTEM
		//I DONNO
	#elif defined(unix) || defined(__unix__) || defined(__unix)
		#define UNIX_SYSTEM
		#include <pthread.h>
	#endif

	/**
	 * If debug mod.
	 **/
	#if defined(DEBUG) || defined(DEBUG_2)
		#include <iostream>
	#endif

	#define EMPTY -1
	namespace gtu
	{
		class mutex : public std::mutex
		{
		public:
			mutex();
			~mutex();
			void register_(std::thread &, int);
			void unregister(std::thread & input);

			void lock();
			void unlock();
			
			inline bool operator< (const mutex & other) const
			{return this->ceil < other.ceil;}
		private:
			static int id;
			#ifdef WINDOWS_SYSTEM
				//I DONNO
			#else
				#ifdef UNIX_SYSTEM
					cpu_set_t cpuset;
				#endif
			#endif
			static std::forward_list<mutex *> locked;
			struct thread
			{
				std::thread::native_handle_type the_thread;
				int orj_priority;
				thread(std::thread::native_handle_type _thread = EMPTY, int priority = EMPTY) 
							: the_thread(_thread), orj_priority(priority)
				{/*	(-	.	-	_)*/}
				thread(const thread & other) 
							: thread(other.the_thread, other.orj_priority)
				{/* {0	_	0	_)*/}
			public:
				inline bool operator<(const thread & other) const
    			{return this->the_thread < other.the_thread;}

			};
			int ceil;
			bool isLocked;
			std::set<thread> candidate;
			std::thread::native_handle_type locker;
		};
	}

#endif //PRIORITY_CEILING_PROTOCOL_MUTEX_H
