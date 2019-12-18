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
	#include <string>
	#ifdef _WIN32
		#define WINDOWS_SYSTEM
		#include <Windows.h>
		using thread_object = DWORD;
	#elif defined(unix) || defined(__unix__) || defined(__unix)
		#define UNIX_SYSTEM
		#include <pthread.h>
		using thread_object = std::thread::native_handle_type;
	#endif
	/**
	 * If debug mod.
	 **/
	#if defined(DEBUG) || defined(DEBUG_2)
		#include <iostream>
	#endif

	#define EMPTY 0
	namespace gtu
	{
		class mutex : public std::mutex
		{
		public:
			/**
			 * Creates a mutex with an id.
			 */
			mutex();
			/**
			 * Deletes the mutex data. If Debug mode is selected, then prints the last state of the locked threads.
			 */
			~mutex();
			/**
			 * Creates an entry for given thread input with the priority.
			 * Possibly throws some exceptions.
			 * Sets the running cpu as declared one cpu. Adds the thread descriptor to the candidate list.
			 */
			void register_(std::thread &, int);
			/**
			 * Deletes all candidates of the mutex.
			 */
			inline void unregister() {this->candidate.clear();}

			/**
			 * Tries to lock wrt. Priority Ceiling Protocol.
			 * If a thread locks the mutex or the locker threads one of interested mutex is locked by another thread
			 * the higher priority of them will be the next priority of other. Then gives the CPU to the other thread.
			 * If nothing above happens, locks mutex and goes to the way.
			 */
			void lock();
			/**
			 * The thread unlocks the mutex, returns their orj priority and releases the cpu.
			 */
			void unlock();

			/**
			 * Compares the mutex priority with other.
			 * @param other a mutex to compare.
			 * @return if this mutex' priority is less then other'
			 */
			inline bool operator< (const mutex & other) const
			{return this->ceil < other.ceil;}
		private:
			static int id;
			#ifdef UNIX_SYSTEM
				cpu_set_t cpuset;
			#endif
			/**
			 * All currently locked mutexes.
			 */
			static std::forward_list<mutex *> locked;
			/**
			 * A struct to hold necessary informations about candidate threads.
			 */
			struct thread
			{
				/**
				 * Depends on the OS, it will be a Id of thread or the native handler class object.
				 */
				thread_object the_thread;
				/**
				 * Orj priority that given while registering the thread to a mutex.
				 */
				int orj_priority;
				thread(thread_object _thread = EMPTY, int priority = EMPTY)
							: the_thread(_thread), orj_priority(priority)
				{/*	(-	.	-	_)*/}
				thread(const thread & other) 
							: thread(other.the_thread, other.orj_priority)
				{/* {0	_	0	_)*/}
			public:
				inline bool operator<(const thread & other) const
    			{return this->the_thread < other.the_thread;}

			};
			/**
			 * Highest candidate' priority.
			 */
			int ceil;
			/**
			 * Information about if the mutex is locked or not.
			 */
			bool isLocked;
			/**
			 * All registered threads to the mutex.
			 */
			std::set<thread> candidate;
			/**
			 * Locker objects ref.
			 */
			thread_object locker;
		};
	}

#endif //PRIORITY_CEILING_PROTOCOL_MUTEX_H
