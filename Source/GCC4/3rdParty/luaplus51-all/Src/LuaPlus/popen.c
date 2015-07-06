#ifdef WIN32

/* popen.c
  RunSilent() is by Steven Szelei,
*/

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>

// From some MSDN sample, I think.
static int CreatePipeChild(HANDLE* child, HANDLE* inH, HANDLE* outH, HANDLE* errH, int redirect_stderr_to_stdout, LPCTSTR Command)
{
    SECURITY_ATTRIBUTES lsa;     
    HANDLE ChildIn;
	HANDLE ChildOut;
	HANDLE ChildErr = NULL;
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO             si;
    HANDLE hNul;

	sa.nLength = sizeof(sa);                        // Security descriptor for INHERIT.
    sa.lpSecurityDescriptor = 0;
    sa.bInheritHandle       = 0;

	lsa.nLength=sizeof(SECURITY_ATTRIBUTES);
	lsa.lpSecurityDescriptor=NULL;
	lsa.bInheritHandle=TRUE;
	
	if (!CreatePipe(&ChildIn,inH,&lsa,0))
	{
		// Error.
	}

	if (!CreatePipe(outH,&ChildOut,&lsa,0))
	{
		// Error.
	}

	if (!redirect_stderr_to_stdout)
	{
		if (!CreatePipe(errH,&ChildErr,&lsa,0))
		{
			// Error.
		}
	}

	// Lets Redirect Console StdHandles - easy enough

	// Dup the child handle to get separate handles for stdout and err,
    hNul = CreateFile("NUL",
                      GENERIC_READ | GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL, OPEN_EXISTING,
                      0,
                      NULL);

    if (hNul != NULL)
	{
        // Set up members of STARTUPINFO structure.
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
        si.hStdOutput = ChildOut;
		si.hStdError    = redirect_stderr_to_stdout ? ChildOut : ChildErr;
        si.hStdInput    = ChildIn;
        if (CreateProcess(NULL, (char*)Command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) == TRUE)
        {
            CloseHandle(pi.hThread);        // Thread handle not needed
            //fprintf(stderr, "create process success\n");
            *child = pi.hProcess;  // Return process handle (to get RC)
        } else
			return -1;
        CloseHandle(hNul);                                      // Close error handle,
		CloseHandle(ChildOut);
		if (!redirect_stderr_to_stdout)
			CloseHandle(ChildErr);
		CloseHandle(ChildIn);
    }
    else
	{
		// Error.
	}

    return 0;
}


DWORD RunSilent(const char* cmd)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInfo;
	const char* comspec;
	char* commandBuffer;
	ULONG rc;
	int isCmd;
	
	comspec = getenv("COMSPEC");
	if (!comspec)
		comspec = "cmd";

	commandBuffer = (char*)malloc(strlen(comspec) + 4 + strlen(cmd) + 2 + 1);
	strcpy(commandBuffer, comspec);
	strcat(commandBuffer, " /c ");
	strlwr(commandBuffer);
	isCmd = strstr(commandBuffer, "cmd.exe") != NULL;
	if (isCmd)
		strcat(commandBuffer, "\"");
	strcat(commandBuffer, cmd);
	if (isCmd)
		strcat(commandBuffer, "\"");

	memset(&StartupInfo, 0, sizeof(StartupInfo));
	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE;

	if (!CreateProcess( NULL, commandBuffer, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, 
		NULL, 
		NULL,
		&StartupInfo,
		&ProcessInfo))
	{
		free(commandBuffer);
		return GetLastError();		
	}
	free(commandBuffer);

	WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
	if(!GetExitCodeProcess(ProcessInfo.hProcess, &rc))
		rc = 0;

	CloseHandle(ProcessInfo.hThread);
	CloseHandle(ProcessInfo.hProcess);

	return rc;
	
}

FILE* pt_popen(const char *cmd, const char *mode)
{
	const char* comspec;
	char* commandBuffer;
	char* stderr_to_stdout;
	int redirectStderr = 0;
	HANDLE child;
	HANDLE hIn = INVALID_HANDLE_VALUE, hOut = INVALID_HANDLE_VALUE, hErr = INVALID_HANDLE_VALUE;
	int rc;
	FILE* file;
	int isCmd;

	if (!mode  ||  !*mode  ||  (mode[0] != 'r'  &&  mode[0] != 'w'))
		return NULL;

	comspec = getenv("COMSPEC");
	if (!comspec)
		comspec = "cmd";

	commandBuffer = (char*)malloc(strlen(comspec) + 4 + strlen(cmd) + 2 + 1);
	strcpy(commandBuffer, comspec);
	strcat(commandBuffer, " /c ");
	strlwr(commandBuffer);
	isCmd = strstr(commandBuffer, "cmd.exe") != NULL;
	if (isCmd)
		strcat(commandBuffer, "\"");
	strcat(commandBuffer, cmd);
	if (isCmd)
		strcat(commandBuffer, "\"");

	stderr_to_stdout = strstr(commandBuffer, "2>&1");
	if (stderr_to_stdout)
	{
		stderr_to_stdout[0] = stderr_to_stdout[1] = stderr_to_stdout[2] = stderr_to_stdout[3] = ' ';
		redirectStderr = 1;
	}

	rc = CreatePipeChild(&child, &hIn, &hOut, &hErr, redirectStderr, commandBuffer);
	free(commandBuffer);
	if (rc == -1)
	{
		return NULL;
	}
	
	if (mode[0] == 'r')
	{
		file = _fdopen(_open_osfhandle((long)hOut, _O_RDONLY | _O_TEXT), "rt");
		if (hIn != INVALID_HANDLE_VALUE)
			CloseHandle(hIn);
		if (hErr != INVALID_HANDLE_VALUE)
			CloseHandle(hErr);
	}
	else
	{
		file = _fdopen(_open_osfhandle((long)hIn, _O_WRONLY | _O_TEXT), "wt");
		if (hOut != INVALID_HANDLE_VALUE)
			CloseHandle(hOut);
		if (hErr != INVALID_HANDLE_VALUE)
			CloseHandle(hErr);
	}
	setvbuf(file, NULL, _IONBF, 0);

	CloseHandle(child);

	return file;
}


int pt_pclose(FILE *file)
{
	if (file)
	{
		fclose(file);
		return 0;
	}
	return -1;
}

#endif

