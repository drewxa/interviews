/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    testapp.c

Abstract:

    Purpose of this app to test the NONPNP sample driver. The app
    makes four different ioctl calls to test all the buffer types, write
    some random buffer content to a file created by the driver in \SystemRoot\Temp
    directory, and reads the same file and matches the content.
    If -l option is specified, it does the write and read operation in a loop
    until the app is terminated by pressing ^C.

    Make sure you have the \SystemRoot\Temp directory exists before you run the test.

Environment:

    Win32 console application.

--*/

        
#include <DriverSpecs.h>
_Analysis_mode_(_Analysis_code_type_user_code_)  

#include <windows.h>

#pragma warning(disable:4201)  // nameless struct/union
#include <winioctl.h>
#pragma warning(default:4201)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <strsafe.h>
#include "public.h"


BOOLEAN
ManageDriver(
    IN LPCTSTR  DriverName,
    IN LPCTSTR  ServiceName,
    IN USHORT   Function
    );

HMODULE
LoadWdfCoInstaller(
    VOID
    );

VOID
UnloadWdfCoInstaller(
    HMODULE Library
    );

BOOLEAN
SetupDriverName(
    _Inout_updates_all_(BufferLength) PCHAR DriverLocation,
    _In_ ULONG BufferLength
    );

BOOL 
SendCommand(
	HANDLE hDevice,
	DWORD command,
	STRSAFE_LPCWSTR pwcSrc
	);

// for example, WDF 1.9 is "01009". the size 6 includes the ending NULL marker
//
#define MAX_VERSION_SIZE 6

CHAR G_coInstallerVersion[MAX_VERSION_SIZE] = {0};
BOOLEAN  G_fLoop = FALSE;
BOOL G_versionSpecified = FALSE;



//-----------------------------------------------------------------------------
// 4127 -- Conditional Expression is Constant warning
//-----------------------------------------------------------------------------
#define WHILE(constant) \
__pragma(warning(disable: 4127)) while(constant); __pragma(warning(default: 4127))


#define USAGE  \
"Usage: nonpnpapp <-V version> <-l> \n" \
       " -V version  {if no version is specified the version specified in the build environment will be used.}\n" \
       "    The version is the version of the KMDF coinstaller to use \n"  \
       "    The format of version  is MMmmm where MM -- major #, mmm - serial# \n" \
       " -l  { option to continuously read & write to the file} \n"

BOOL
ValidateCoinstallerVersion(
    _In_ PSTR Version
    )
{   BOOL ok = FALSE;
    INT i;

    for(i= 0; i<MAX_VERSION_SIZE ;i++){
        if( ! IsCharAlphaNumericA(Version[i])) {
            break;
        }
    }
    if (i == (MAX_VERSION_SIZE -sizeof(CHAR))) {
        ok = TRUE;
    }
    return ok;
}

LONG
Parse(
    _In_ int argc,
    _In_reads_(argc) char *argv[]
    )
/*++
Routine Description:

    Called by main() to parse command line parms

Arguments:

    argc and argv that was passed to main()

Return Value:

    Sets global flags as per user function request

--*/
{
    int i;
    BOOL ok;
    LONG error = ERROR_SUCCESS;

    for (i=0; i<argc; i++) {
        if (argv[i][0] == '-' ||
            argv[i][0] == '/') {
            switch(argv[i][1]) {
            case 'V':
            case 'v':
                if (( (i+1 < argc ) &&
                      ( argv[i+1][0] != '-' && argv[i+1][0] != '/'))) {
                    //
                    // use version in commandline
                    //
                    i++;
                    ok = ValidateCoinstallerVersion(argv[i]);
                    if (!ok) {
                        printf("Not a valid format for coinstaller version\n"
                               "It should be characters between A-Z, a-z , 0-9\n"
                               "The version format is MMmmm where MM -- major #, mmm - serial#");
                        error = ERROR_INVALID_PARAMETER;
                        break;
                    }
                    if (FAILED( StringCchCopy(G_coInstallerVersion,
                                              MAX_VERSION_SIZE,
                                              argv[i]) )) {
                        break;
                    }
                    G_versionSpecified = TRUE;

                }
                else{
                    printf(USAGE);
                    error = ERROR_INVALID_PARAMETER;
                }
                break;
            case 'l':
            case 'L':
                G_fLoop = TRUE;
                break;
            default:
                printf(USAGE);
                error = ERROR_INVALID_PARAMETER;

            }
        }
    }
    return error;
}

PCHAR
GetCoinstallerVersion(
    VOID
    )
{
    if (FAILED( StringCchPrintf(G_coInstallerVersion,
                                MAX_VERSION_SIZE,
                                "%02d%03d",    // for example, "01009"
                                KMDF_VERSION_MAJOR,
                                KMDF_VERSION_MINOR)))
    {
        printf("StringCchCopy failed with error \n");
    }

    return (PCHAR)&G_coInstallerVersion;
}

VOID __cdecl
main(
    _In_ ULONG argc,
    _In_reads_(argc) PCHAR argv[]
    )
{
    HANDLE   hDevice;
    DWORD    errNum = 0;
    CHAR     driverLocation [MAX_PATH];
    BOOL     ok;
    HMODULE  library = NULL;
	LONG     error;
	PCHAR    coinstallerVersion;
	WCHAR    proc_name[1024];

    //
    // Parse command line args
    //   -l     -- loop option
    //
    if ( argc > 1 ) {// give usage if invoked with no parms
        error = Parse(argc, argv);
        if (error != ERROR_SUCCESS) {
            return;
        }
    }

    if (!G_versionSpecified ) {
        coinstallerVersion = GetCoinstallerVersion();

        //
        // if no version is specified or an invalid one is specified use default version
        //
        printf("No version specified. Using default version:%s\n",
               coinstallerVersion);

    } else {
        coinstallerVersion = (PCHAR)&G_coInstallerVersion;
    }

    //
    // open the device
    //
    hDevice = CreateFile(DEVICE_NAME,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

    if(hDevice == INVALID_HANDLE_VALUE) {

        errNum = GetLastError();

        if (!(errNum == ERROR_FILE_NOT_FOUND ||
                errNum == ERROR_PATH_NOT_FOUND)) {

            printf("CreateFile failed!  ERROR_FILE_NOT_FOUND = %d\n",
                   errNum);
            return ;
        }

        //
        // Load WdfCoInstaller.dll.
        //
        library = LoadWdfCoInstaller();

        if (library == NULL) {
            printf("The WdfCoInstaller%s.dll library needs to be "
                   "in same directory as nonpnpapp.exe\n", coinstallerVersion);
            return;
        }

        //
        // The driver is not started yet so let us the install the driver.
        // First setup full path to driver name.
        //
        ok = SetupDriverName( driverLocation, MAX_PATH );

        if (!ok) {
            return ;
        }

        ok = ManageDriver( DRIVER_NAME,
                           driverLocation,
                           DRIVER_FUNC_INSTALL );

        if (!ok) {

            printf("Unable to install driver. \n");

            //
            // Error - remove driver.
            //
            ManageDriver( DRIVER_NAME,
                          driverLocation,
                          DRIVER_FUNC_REMOVE );
            return;
        }

        hDevice = CreateFile( DEVICE_NAME,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL );

        if (hDevice == INVALID_HANDLE_VALUE) {
            printf ( "Error: CreatFile Failed : %d\n", GetLastError());
            return;
        }
    }
	
	memset(proc_name, 0, sizeof(WCHAR) * 1024);
	printf("Proc name:\n");
	wscanf(L"%1023s", proc_name);

	if (SendCommand(hDevice, (DWORD)IOCTL_NONPNP_METHOD_INIT_MONITOR, proc_name))
	{
		printf("Monitor has been initialized. To run monitor press any key\n");
		system("pause");
		if (SendCommand(hDevice, (DWORD)IOCTL_NONPNP_METHOD_RUN_MONITOR, NULL))
		{
			printf("Monitor has been activated!\n");

			printf("Press any key to stop monitor. waiting.....\n");
			system("pause");
			if (SendCommand(hDevice, (DWORD)IOCTL_NONPNP_METHOD_STOP_MONITOR, NULL))
			{
				printf("Monitor has been stopped. Bye!\n");
			}
		}
	}

    CloseHandle ( hDevice );

    //
    // Unload the driver.  Ignore any errors.
    //
    ManageDriver( DRIVER_NAME,
                  driverLocation,
                  DRIVER_FUNC_REMOVE );

    //
    // Unload WdfCoInstaller.dll
    //
    if ( library ) {
        UnloadWdfCoInstaller( library );
    }

	printf("Bye, bye!\n");
	system("pause");
    return;
}

BOOL SendCommand(
	HANDLE hDevice,
	DWORD command,
	STRSAFE_LPCWSTR pwcSrc
	)
{
	char OutputBuffer[100];
	wchar_t InputBuffer[100];
	BOOL bRc;
	ULONG bytesReturned;

	memset(InputBuffer, 0, sizeof(InputBuffer));
	if (FAILED(StringCchCopyW(InputBuffer, sizeof(InputBuffer), pwcSrc ? pwcSrc : L"")))
	{
        return FALSE;
    }
	
    memset(OutputBuffer, 0, sizeof(OutputBuffer));

	bRc = DeviceIoControl(hDevice,
						  command,
						  InputBuffer,
						  ((DWORD)wcslen(InputBuffer) * 2) + 1,
						  OutputBuffer,
						  sizeof(OutputBuffer),
						  &bytesReturned,
						  NULL
						  );

	if (!bRc)
	{
		printf("Error in DeviceIoControl : %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}