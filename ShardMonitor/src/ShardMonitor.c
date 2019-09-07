#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow )
{
    PROCESS_INFORMATION pi;
    STARTUPINFOW si;

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0, sizeof(pi));

    if (!CreateProcessW(NULL, L"ServerMonitor.exe -shardmonitor", NULL, NULL, 0, 0, NULL, NULL, &si, &pi))
    {
        MessageBoxW(NULL, L"Error spawning ServerMonitor.exe -shardmonitor", L"Error", MB_OK);
    }
    return 0;
}
