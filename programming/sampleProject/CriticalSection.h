
#ifndef CriticalSection_h_
#define CriticalSection_h_

#include "TwsApiL0.h"
//This class was originally declared in TwsApi. Here I redeclared in a new  place because many places will use it.

#ifdef WIN32
	#include <windows.h>	// for mutex
	#include <process.h>	// for Sleep
	#define CurrentThreadId GetCurrentThreadId
#else
	#include <pthread.h>	// for mutex
	#define Sleep( m ) usleep( m*1000 )
	#define CurrentThreadId pthread_self
#endif

//Note I changed all CriticalSection (copied from TwsApiL0.cpp) to Critical_Section below. 
struct Critical_Section
{
#ifdef WIN32
public:
	CRITICAL_SECTION	m_CriticalSection;

	Critical_Section		( void ) { ::InitializeCriticalSection	(&m_CriticalSection); }
	~Critical_Section	( void ) { ::DeleteCriticalSection		(&m_CriticalSection); }

#ifdef _DEBUG
	// Put in a seperate method to allow breakpoint in debugger
	// While debugging, shouldn't it always have a breakpoint set
	// The throw(0) on purpose for the ENTERCRITICALSECTION() macro
	void DeadlockEncountered( void ) { throw(0); }
	void EnterCriticalSection   ( void ) { try{ ::EnterCriticalSection(&m_CriticalSection); } catch(...) { DeadlockEncountered(); } }
#else
	void EnterCriticalSection   ( void ) { ::EnterCriticalSection(&m_CriticalSection); }
#endif

	void LeaveCriticalSection   ( void ) { ::LeaveCriticalSection(&m_CriticalSection); }

	bool TryEnterCriticalSection( void ) { return ::TryEnterCriticalSection(&m_CriticalSection) != 0; }
#else
	pthread_mutex_t	m_mutex;

	Critical_Section		( void ) :m_mutex (PTHREAD_MUTEX_INITIALIZER) {}

	~Critical_Section	( void ) { ::pthread_mutex_destroy( &m_mutex ); }

	bool EnterCriticalSection   ( void ) { ::pthread_mutex_lock  ( &m_mutex ); }
	void LeaveCriticalSection   ( void ) { ::pthread_mutex_unlock( &m_mutex ); }

	bool TryEnterCriticalSection( void ) { return ::pthread_mutex_trylock( &m_mutex ) == 0; }
#endif
};


#endif 