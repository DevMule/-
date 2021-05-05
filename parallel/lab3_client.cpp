#include <windows.h> // ���������� winapi
#include <synchapi.h> // ���������� ������������� � winapi
#include <stdio.h> // ���������� ����� ������
#include <conio.h> // ������� ��� ����������� ����� ������
#include <tchar.h> // ���������� �������� �������������� char

#define BUFSIZE 512 // ������������ ����� ������

int _tmain(int argc, TCHAR* argv[]) // ������� �������� ������ ���������
{
    HANDLE hPipe; // ���������� ��� ������
    LPCWSTR lpvMessage = TEXT("Default message from client."); // ���������� ���������
    TCHAR  chBuf[BUFSIZE]; // ����� ��� ������ �������
    BOOL   fSuccess = FALSE; // ���������� ��������� ���������� ��������
    DWORD  cbRead, cbToWrite, cbWritten, dwMode; // ���������� ��� �������� ���-�� ����
    LPCWSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe"); // ����������� ��� ������

    if (argc > 1) // ���� � ��������� �������� �����, �� ���������� ���, � �� �� ���������
        lpvMessage = argv[1];

    while (true) // ���� ������������ ���������� 
    {
        hPipe = CreateFile( // ������� �����
            lpszPipename,   // ��� ������  
            GENERIC_READ |  // ������ �� ������ � ������ 
            GENERIC_WRITE,
            0,              // ����� ����������� 
            NULL,           // �������� ������������ �� ���������
            OPEN_EXISTING,  // �������� ������������� ������ 
            0,              // �������� �� ���������
            NULL);          // ��� �����-�������

		// Break if the pipe handle is valid. 

        if (hPipe != INVALID_HANDLE_VALUE) // ����� �� ����� ���� ���������� �����������
            break;

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY) // ��������� ������, ����� ����� ��� �����
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());// ����� ���������
            return -1; // ������� ��� ������ -1
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipe(lpszPipename, 20000)) // ���� ����� �� �������� � ������� 20 ������, ��������� ��� -1
        {
            printf("Could not open pipe: 20 second wait timed out.");// ����� ���������
            return -1; // ������� ��� ������ -1
        }
    }

    dwMode = PIPE_READMODE_MESSAGE; // ����� ����� ������
    fSuccess = SetNamedPipeHandleState( // ������ ����� ������ �� ������
        hPipe,    // ���������� ������
        &dwMode,  // ����� ����� ��� ������
        NULL,     // ������������ ���-�� ����
        NULL);    // ������������ ����� ��������

	if (!fSuccess) // ��������� ������, ���� ������� ����� �� �������
    {
        _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());// ����� ���������
        return -1; // ������� ��� ������ -1
    }


	for(int i = 0; i < 10; i++) // 10 ��� ���� �� ������ ���������
	{
		cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR); // ���-�� ���� ��� ��������
		_tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);// ����� ���������

		fSuccess = WriteFile( // ����� � �����
			hPipe,                  // ���������� ������
			lpvMessage,             // ��������� 
			cbToWrite,              // ����� ��������� 
			&cbWritten,             // ���-�� ���������� ���� 
			NULL);                  // ����/����� �� ������������� 

		if (!fSuccess) // ��������� ������ ��� ������ � �����
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());// ����� ���������
			return -1;// ������� ��� ������ -1
		}

		printf("Message sent to server\n");// ����� ���������

        Sleep(1000); // �������� ������ �� 2 �������
	}
    

    printf("\n<End of message, press ENTER to terminate connection and exit>");// ����� ���������
    _getch(); // �������� ������� ����� ������

    CloseHandle(hPipe); // ��������� ���������� ������

    return 0;
}