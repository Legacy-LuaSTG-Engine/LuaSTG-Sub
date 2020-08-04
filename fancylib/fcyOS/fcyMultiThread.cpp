#include "fcyMultiThread.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

fcyBaseThread::fcyBaseThread(fBool PauseThread)
{
	m_hThread = CreateThread(0, 0, &execute, (void*)this, PauseThread ? CREATE_SUSPENDED : 0, &m_hThreadID);
	if(m_hThread == NULL)
		throw fcyWin32Exception("fcyBaseThread::fcyBaseThread", "CreateThread Failed.");
}

fcyBaseThread::~fcyBaseThread()
{
	CloseHandle(m_hThread);
}

DWORD WINAPI fcyBaseThread::execute(void* p)
{
	return ((fcyBaseThread*)p)->ThreadJob();
}

HANDLE fcyBaseThread::GetHandle()
{
	return m_hThread;
}

fBool fcyBaseThread::Resume()
{
	// Ê§°Ü·µ»Ø-1
	return ResumeThread(m_hThread) != (DWORD)-1;
}

fBool fcyBaseThread::Suspend()
{
	// Ê§°Ü·µ»Ø-1
	return SuspendThread(m_hThread) != (DWORD)-1;
}

fBool fcyBaseThread::Wait(fInt TimeLimited)
{
	// Ê§°Ü·µ»Ø-1
	return WaitForSingleObject(m_hThread, TimeLimited) != (DWORD)-1;
}

fBool fcyBaseThread::Terminate(fInt ExitCode)
{
	// Ê§°Ü·µ»Ø0
	return TerminateThread(m_hThread, ExitCode) != FALSE;
}

fuInt fcyBaseThread::GetExitCode()
{
	DWORD code = (DWORD)-1;
	GetExitCodeThread(m_hThread, &code);
	return code;
}

////////////////////////////////////////////////////////////////////////////////

fcyCriticalSection::fcyCriticalSection()
{
	InitializeCriticalSection(&m_Section);
}

fcyCriticalSection::~fcyCriticalSection()
{
	DeleteCriticalSection(&m_Section);
}

fBool fcyCriticalSection::TryLock()
{
	return TryEnterCriticalSection(&m_Section) != FALSE;
}

void fcyCriticalSection::Lock()
{
	EnterCriticalSection(&m_Section);
}

void fcyCriticalSection::UnLock()
{
	LeaveCriticalSection(&m_Section);
}

////////////////////////////////////////////////////////////////////////////////

fcyEvent::fcyEvent(fBool AutoReset, fBool InitalState)
{
	m_hEvent = CreateEvent(NULL,AutoReset,InitalState,NULL);
	if(m_hEvent == NULL)
		throw fcyWin32Exception("fcyEvent::fcyEvent", "CreateEvent Failed.");
}

fcyEvent::~fcyEvent()
{
	CloseHandle(m_hEvent);
}

HANDLE fcyEvent::GetHandle()
{
	return m_hEvent;
}

fBool fcyEvent::Set()
{
	return SetEvent(m_hEvent) != FALSE;
}

fBool fcyEvent::Reset()
{
	return ResetEvent(m_hEvent) != FALSE;
}

fBool fcyEvent::Pulse()
{
	return PulseEvent(m_hEvent) != FALSE;
}

fBool fcyEvent::Wait(fInt TimeLimited)
{
	return WaitForSingleObject(m_hEvent, TimeLimited) != (DWORD)-1;
}
