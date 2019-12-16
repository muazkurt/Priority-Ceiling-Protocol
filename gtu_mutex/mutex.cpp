//
// Created by MSI on 13.12.2019.
//

#include "mutex.h"

std::forward_list<gtu::mutex *> gtu::mutex::locked;
int gtu::mutex::id = 0;

gtu::mutex::mutex() : std::mutex(), ceil(EMPTY), isLocked(false), locker()
{
	++id;
	//Debug fold.
	#ifdef UNIX_SYSTEM
		CPU_ZERO(&cpuset);
		CPU_SET(0, &cpuset);
	#endif
}

gtu::mutex::~mutex()
{
	//Debug fold.
		#if defined(DEBUG) || defined(DEBUG_2)
			std::cerr << "---------------------\n";
			std::cerr << "Still locked:\n";
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority << ')';
				std::cerr << '\n';
			}
			std::cerr << "---------------------\n";
		#endif
}


void gtu::mutex::unlock()
{
	//Debug fold.
		#ifdef DEBUG
			std::cerr << "---------------------------" << std::endl;
			std::cerr << "1: Unlock start" << std::endl;
		#endif
	#ifdef UNIX_SYSTEM
		if(pthread_self() != this->locker)
			throw std::domain_error("HOW DARE YOU! " + std::to_string(id));
		if(!this->isLocked)
			throw std::invalid_argument("DUDE CMON, ITS ALREADY UNLOCKED! " +  std::to_string(id));
	#endif
		#ifdef DEBUG
			std::cerr << "2: Error case finished" << std::endl;
		#endif
	#ifdef UNIX_SYSTEM
		sched_param priority_param;
		priority_param.__sched_priority = candidate.find({pthread_self(), EMPTY})->orj_priority;
		if(pthread_setschedparam(pthread_self(), SCHED_FIFO, &priority_param))
			throw std::system_error(std::error_code(errno, std::system_category()), 
				"Failed to set priority to orj while unlock " + std::to_string(id));
	#endif
		#ifdef DEBUG
			std::cerr << "3: Returned to orj priority." << std::endl;
		#endif
	this->isLocked = false;
	this->locker = EMPTY;
		#ifdef DEBUG
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority << ')';
				std::cerr << '\n';
			}
		#endif
	locked.remove(this);
	locked.sort([](const mutex * first, const mutex * second){return *first < *second;});
		#ifdef DEBUG
			std::cerr << "4: Removed from locked array." << std::endl;
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority<< ')';
				std::cerr << '\n';
			}
		#endif
	#ifdef UNIX_SYSTEM
		if(sched_yield())
			throw std::system_error(std::error_code(errno,std::system_category()), 
				"Impossible in linux" + std::to_string(id));
	#endif
	std::this_thread::yield();
		#ifdef DEBUG
			std::cerr << "5: Now, I'm out." << std::endl;
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority<< ')';
				std::cerr << '\n';
			}
		#endif
}

void gtu::mutex::lock()
{
	//Debug fold.
		#ifdef DEBUG
			std::cerr << "---------------------------" << std::endl;
			std::cerr << "1: lock start, locker: " << pthread_self() << std::endl;
		#endif
	int workon_priority = EMPTY;
	#ifdef UNIX_SYSTEM
		if(locker == pthread_self())
			throw std::invalid_argument("DUDE CMON, YOU HAVE ALREADY LOCKED! " + std::to_string(id));
		auto workon = this->candidate.find({pthread_self(), EMPTY});
	#endif
	if(this->candidate.end() == workon)
		throw std::domain_error("YOU HAVEN'T GOT ASSIGNED! " + std::to_string(id));
		#ifdef DEBUG
			std::cerr << "2: Found candidate: " << workon->the_thread << std::endl;
		#endif
	{
		#ifdef UNIX_SYSTEM
			sched_param sch;
			int policy; 
			if(pthread_getschedparam(workon->the_thread, &policy, &sch))
				throw std::system_error(std::error_code(errno,std::system_category()), 
					"Failed to get priority of the thread that going to lock " + std::to_string(id));
			workon_priority = sch.__sched_priority;
		#endif
	}
		#ifdef DEBUG
			std::cerr << "3: Read sch param" << std::endl;
		#endif

	if(!(locked.empty()) && (workon_priority <= (*locked.begin())->ceil))
		//Benden düşük priorityde olan threadler tarafından locklanan 
		//ve benim ilgilendiğim mutexlerin sahiplerine benim prioritymi
		//vereyim ve kenarda oturayım.
		for(auto bir_keko = locked.begin(); bir_keko != locked.end(); )
		{
			#ifdef DEBUG
				std::cerr << "4 + x: Search for locked mutexes." << std::endl;
			#endif
			if((*bir_keko)->locker != workon->the_thread
			&& (*bir_keko)->candidate.count(*workon) == 1)
			{
				#ifdef DEBUG
					std::cerr << "4 + x + 1: Update child." << std::endl;
				#endif
				#ifdef UNIX_SYSTEM
					sched_param priority;
					int policy;
					if(pthread_getschedparam((*bir_keko)->locker, &policy, &priority))
						throw std::system_error(std::error_code(errno,std::system_category()), 
							"Failed to get the locker thread's priority within loop " + std::to_string(id));
				#endif
				priority.__sched_priority = priority.__sched_priority < workon_priority ?
											priority.__sched_priority :
											workon_priority;
				#ifdef UNIX_SYSTEM
					if (pthread_setschedparam((*bir_keko)->locker, SCHED_FIFO, &priority))
						throw std::system_error(std::error_code(errno,std::system_category()), 
							"Failed to update the locker thread's priority within loop " + std::to_string(id));
					if(sched_yield())
						throw std::system_error(std::error_code(errno,std::system_category()), 
							"Impossible in linux " + std::to_string(id));
				#endif
				std::this_thread::yield();
				bir_keko = locked.begin();
			}
			else
				++bir_keko;
		}
	this->isLocked = true;
		#ifdef DEBUG
			std::cerr << "5: Finally lock" << std::endl;
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority << ')';
				std::cerr << '\n';
			}
		#endif
	this->locker = workon->the_thread;
	locked.push_front(this);
	locked.sort([](const mutex * first, const mutex * second){return *first < *second;});
		#ifdef DEBUG
			std::cerr << "6: Insert yourself the list" << std::endl;
			for (auto it=locked.begin(); it!=locked.end(); ++it)
			{
				std::cerr << (*it)->ceil << ": ";
				for (auto kopek=(*it)->candidate.begin(); kopek!=(*it)->candidate.end(); ++kopek)
					std::cerr << '(' << kopek->the_thread << ", " << kopek->orj_priority << ')';
				std::cerr << '\n';
			}
		#endif
}


void gtu::mutex::register_(std::thread & input, int priority)
{
	#ifdef UNIX_SYSTEM
		//Set to work in a single CPU.
		if(pthread_setaffinity_np(input.native_handle(), sizeof(cpu_set_t), &cpuset))
			throw std::system_error(std::error_code(errno, std::system_category()),
				"Failed to set work on single CPU " + std::to_string(id));
		sched_param priority_param({priority});
	#endif
	thread local(input.native_handle(), priority);
	#ifdef UNIX_SYSTEM
		if (pthread_setschedparam(input.native_handle(), SCHED_FIFO, &priority_param))
		{
			auto local(errno);
			if(local != 0)
				throw std::system_error(std::error_code(local, std::system_category()), 
					"Failed to set the registering thread's priority " + std::to_string(id));
		}
	#endif
	this->ceil = this->ceil > priority ? this->ceil : priority;
	candidate.insert({{local.the_thread, local.orj_priority}});
}


void gtu::mutex::unregister(std::thread & input)
{
	auto item = this->candidate.find(input.native_handle());
	this->candidate.erase(item);
	this->ceil = -1;
	for(auto ilk = this->candidate.begin(); ilk != this->candidate.end(); ++ilk)
		this->ceil = (ilk->orj_priority > this->ceil) ? ilk->orj_priority : this->ceil;
}
