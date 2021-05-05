#include <windows.h> // ����������� winapi
#include <stdio.h> // ���������� ����� ������
#include <tchar.h> // ���������� �������� �������������� char
#include <strsafe.h> // ���������� �������������� ��������� ������

#define BUFSIZE 512 // ������������ ����� ������

DWORD WINAPI InstanceThread(LPVOID); // �������� �������

int _tmain(VOID) // ������� �������� ������ ���������
{
    BOOL   fConnected = FALSE; // �������� ��������� ��������������
    DWORD  dwThreadId = 0; // id ������
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL; // ���������� ��� ������ � ��� ������
    LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); // ����������� ��� ������

    for (;;) // ����������� ����
    {
        _tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename); // ����� ���������
        hPipe = CreateNamedPipe( // �������� ����������� ������
            lpszPipename,             // ��� ������ 
            PIPE_ACCESS_DUPLEX,       // ����� ������� 
            PIPE_TYPE_MESSAGE |       // ��� ������ ��������� 
            PIPE_READMODE_MESSAGE |   // ����� ������ ��������� 
            PIPE_WAIT,                // ����� ���������� 
            PIPE_UNLIMITED_INSTANCES, // ����. ���-�� ����������  
            BUFSIZE,                  // ������ ��������� ������ 
            BUFSIZE,                  // ������ �������� ������ 
            0,                        // ����� ����-���� ������� 
            NULL);     // ������� ������������ �� ��������� 

        if (hPipe == INVALID_HANDLE_VALUE) // ��������� ������ ��� �������� ������
        {
            _tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError()); // ����� ���������
            return -1; // ������� ��� ������ -1
        }

    	// �������� ����������� �������; ���� �������, �� ������� ������ �� ������� ��������.
    	// ���� ������� ���������� 0, GetLastError ������ ERROR_PIPE_CONNECTED

        fConnected = ConnectNamedPipe(hPipe, NULL) ?
            TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected) // ���� ������ �����������
        {
            printf("Client connected, creating a processing thread.\n"); // ����� ���������

            hThread = CreateThread( // �������� ������ ��� ����� �������
                NULL,              // ������� ������������ 
                0,                 // ������ ����� �� ��������� 
                InstanceThread,    // ��������� ������
                (LPVOID)hPipe,    // �������� ������ 
                0,                 // �������� ������������
                &dwThreadId);      // ���������� id ������ 

            if (hThread == NULL) // ��������� ������ �������� ������
            {
                _tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError()); // ����� ���������
                return -1; // ������� ���� ������ -1
            }
            else 
                CloseHandle(hThread); // ����� ������� ����������
        }
        else // ������ �� ���� �����������, ��������� �����
            CloseHandle(hPipe);
    }

    return 0;
}

DWORD WINAPI InstanceThread(LPVOID lpvParam)
// ��� ������� ��������� ������,
// ������� ��������� ������ � �������� ������� ����� ���������� � �������� �������,
// ���������� �� ��������� �����.
{
    HANDLE hHeap = GetProcessHeap(); // ��������� "����" ��������
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR)); // ���������� ���� �� ����� ��������� ������, � ��������� ������ ��� ���
    TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE * sizeof(TCHAR)); // ���������� ���� �� �������� ��������� ������, � ��������� ������ ��� ���

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0; // �������� ������ ��� ���������
    BOOL fSuccess = FALSE; // ���������� ���������� ������� �����������
    HANDLE hPipe = NULL; // ������������� ���������� ��� ������

    if (lpvParam == NULL) // ���� �������� ������ �� ����������
    {
        printf("\nERROR - Pipe Server Failure:\n"); // ����� ���������
        printf("   InstanceThread got an unexpected NULL value in lpvParam.\n"); // ����� ���������
        printf("   InstanceThread exitting.\n"); // ����� ���������
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply); // ������������ ������
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest); // ������������ ������
        return (DWORD)-1; // ������� ���� ������ -1
    }

    if (pchRequest == NULL) // ���� �� ������� �������� ������ �� ����
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL) // ���� ��� � ������
    {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   InstanceThread got an unexpected NULL heap allocation.\n");
        printf("   InstanceThread exitting.\n");
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    printf("InstanceThread created, receiving and processing messages.\n"); // ����� ���������

    hPipe = (HANDLE)lpvParam; // �������� ������ �������� ������������ ���������� ������� ������

    while (1) // ���� �� ��� ��� ���� �� �������� ������
    {
        fSuccess = ReadFile( // ������ ��������� � ������� �� ������ (��������� ���������� bufsize)
            hPipe,        // ���������� ������
            pchRequest,    // ����� ��� ���������
            BUFSIZE * sizeof(TCHAR), // ������ ������ 
            &cbBytesRead, // ���-�� ���� ������������ ���������
            NULL);        // ����/����� �� ������������� 

        if (!fSuccess || cbBytesRead == 0) // ��������� ������� ��������� ��� ������ ��� ������
        {
            if (GetLastError() == ERROR_BROKEN_PIPE) // ������ ���������� � ��������
            {
                _tprintf(TEXT("InstanceThread: client disconnected.\n")); // ����� ���������
            }
            else // ������ ��� ������
            {
                _tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError()); // ����� ���������
            }
            break; // ����� �� �����
        }
        _tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest); // ����� ���������
    }

    FlushFileBuffers(hPipe); // �������������� ������ ����������� ������
    DisconnectNamedPipe(hPipe); // ������������ �� ������
    CloseHandle(hPipe); // �������� ����������� ������

    HeapFree(hHeap, 0, pchRequest); // ������������ ������
    HeapFree(hHeap, 0, pchReply); // ������������ ������

    printf("InstanceThread exiting.\n"); // ����� ���������
    return 1; // ����������� ���� ��������� 1
}

