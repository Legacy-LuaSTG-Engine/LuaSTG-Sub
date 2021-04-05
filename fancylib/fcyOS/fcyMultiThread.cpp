#include "fcyMultiThread.h"
#include <Windows.h>

////////////////////////////////////////////////////////////////////////////////

fcyBaseThread::fcyBaseThread(fBool PauseThread) {
    m_hThread = (fHandle) ::CreateThread(0, 0,
                                         (PTHREAD_START_ROUTINE) &ThreadFunction, (void*) this,
                                         PauseThread ? CREATE_SUSPENDED : 0, (DWORD*) &m_hThreadID);
    if (m_hThread == NULL)
        throw fcyWin32Exception("fcyBaseThread::fcyBaseThread", "CreateThread Failed.");
}

fcyBaseThread::~fcyBaseThread() {
    ::CloseHandle((HANDLE) m_hThread);
}

fuInt __stdcall fcyBaseThread::ThreadFunction(void* p) {
    return ((fcyBaseThread*) p)->ThreadJob();
}

fHandle fcyBaseThread::GetHandle() {
    return m_hThread;
}

fBool fcyBaseThread::Resume() {
    // 失败返回-1
    return ::ResumeThread((HANDLE) m_hThread) != DWORD(-1);
}

fBool fcyBaseThread::Suspend() {
    // 失败返回-1
    return ::SuspendThread((HANDLE) m_hThread) != DWORD(-1);
}

fBool fcyBaseThread::Wait(fInt TimeLimited) {
    // 失败返回-1
    return ::WaitForSingleObject((HANDLE) m_hThread, TimeLimited) != DWORD(-1);
}

fBool fcyBaseThread::Terminate(fInt ExitCode) {
    // 失败返回0
    return ::TerminateThread((HANDLE) m_hThread, ExitCode) != FALSE;
}

fuInt fcyBaseThread::GetExitCode() {
    DWORD code = DWORD(-1);
    ::GetExitCodeThread((HANDLE) m_hThread, &code);
    return code;
}

////////////////////////////////////////////////////////////////////////////////

fcyCriticalSection::fcyCriticalSection() {
    InitializeCriticalSection((CRITICAL_SECTION*) &m_Section);
}

fcyCriticalSection::~fcyCriticalSection() {
    DeleteCriticalSection((CRITICAL_SECTION*) &m_Section);
}

fBool fcyCriticalSection::TryLock() {
    return TryEnterCriticalSection((CRITICAL_SECTION*) &m_Section) != FALSE;
}

void fcyCriticalSection::Lock() {
    EnterCriticalSection((CRITICAL_SECTION*) &m_Section);
}

void fcyCriticalSection::UnLock() {
    LeaveCriticalSection((CRITICAL_SECTION*) &m_Section);
}

////////////////////////////////////////////////////////////////////////////////

fcyEvent::fcyEvent(fBool AutoReset, fBool InitalState) {
    m_hEvent = (fHandle) ::CreateEventW(NULL, AutoReset, InitalState, NULL);
    if (m_hEvent == NULL)
        throw fcyWin32Exception("fcyEvent::fcyEvent", "CreateEvent Failed.");
}

fcyEvent::~fcyEvent() {
    ::CloseHandle((HANDLE) m_hEvent);
}

fHandle fcyEvent::GetHandle() {
    return m_hEvent;
}

fBool fcyEvent::Set() {
    return ::SetEvent((HANDLE) m_hEvent) != FALSE;
}

fBool fcyEvent::Reset() {
    return ::ResetEvent((HANDLE) m_hEvent) != FALSE;
}

fBool fcyEvent::Pulse() {
    return ::PulseEvent((HANDLE) m_hEvent) != FALSE;
}

fBool fcyEvent::Wait(fInt TimeLimited) {
    return ::WaitForSingleObject((HANDLE) m_hEvent, TimeLimited) != WAIT_FAILED;
}
