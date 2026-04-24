#include <windows.h>

namespace {
    SERVICE_STATUS g_status{};
    SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
    constexpr wchar_t kServiceName[] = L"Tema3HelloService";

    void LogInfoEvent(const wchar_t* text) {
        HANDLE source = RegisterEventSourceW(nullptr, kServiceName);
        if (!source) {
            return;
        }

        LPCWSTR payload[] = { text };
        ReportEventW(
            source,
            EVENTLOG_INFORMATION_TYPE,
            0,
            1000,
            nullptr,
            1,
            0,
            payload,
            nullptr
        );
        DeregisterEventSource(source);
    }

    void UpdateServiceState(DWORD state, DWORD win32Code = NO_ERROR, DWORD waitHint = 0) {
        g_status.dwCurrentState = state;
        g_status.dwWin32ExitCode = win32Code;
        g_status.dwWaitHint = waitHint;
        SetServiceStatus(g_statusHandle, &g_status);
    }
}

void WINAPI HandleControlCode(DWORD controlCode) {
    if (controlCode == SERVICE_CONTROL_STOP) {
        UpdateServiceState(SERVICE_STOP_PENDING, NO_ERROR, 1500);
        UpdateServiceState(SERVICE_STOPPED);
    }
}

void WINAPI RunService(DWORD, LPWSTR*) {
    g_statusHandle = RegisterServiceCtrlHandlerW(kServiceName, HandleControlCode);
    if (!g_statusHandle) {
        return;
    }

    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_status.dwCurrentState = SERVICE_START_PENDING;
    g_status.dwWin32ExitCode = NO_ERROR;
    g_status.dwServiceSpecificExitCode = 0;
    g_status.dwCheckPoint = 0;
    g_status.dwWaitHint = 0;

    UpdateServiceState(SERVICE_RUNNING);
    LogInfoEvent(L"Hello World!");

    while (g_status.dwCurrentState == SERVICE_RUNNING) {
        Sleep(500);
    }
}

int wmain() {
    SERVICE_TABLE_ENTRYW dispatchTable[] = {
        { const_cast<LPWSTR>(kServiceName), RunService },
        { nullptr, nullptr }
    };

    StartServiceCtrlDispatcherW(dispatchTable);
    return 0;
}
