/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    PUBLIC.H

Abstract:


    Defines the IOCTL codes that will be used by this driver.  The IOCTL code
    contains a command identifier, plus other information about the device,
    the type of access with which the file must have been opened,
    and the type of buffering.

Environment:

    Kernel mode only.

--*/

//
// Device type           -- in the "User Defined" range."
//
#define FILEIO_TYPE 40001
//
// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
//

#define IOCTL_NONPNP_METHOD_NEITHER \
    CTL_CODE( FILEIO_TYPE, 0x903, METHOD_NEITHER , FILE_ANY_ACCESS  )

#define IOCTL_NONPNP_METHOD_INIT_MONITOR \
    CTL_CODE( FILEIO_TYPE, 0x904, METHOD_BUFFERED , FILE_ANY_ACCESS  )

#define IOCTL_NONPNP_METHOD_RUN_MONITOR \
    CTL_CODE( FILEIO_TYPE, 0x905, METHOD_BUFFERED , FILE_ANY_ACCESS  )

#define IOCTL_NONPNP_METHOD_PAUSE_MONITOR \
    CTL_CODE( FILEIO_TYPE, 0x906, METHOD_BUFFERED , FILE_ANY_ACCESS  )

#define IOCTL_NONPNP_METHOD_STOP_MONITOR \
    CTL_CODE( FILEIO_TYPE, 0x907, METHOD_BUFFERED , FILE_ANY_ACCESS  )


#define DRIVER_FUNC_INSTALL     0x01
#define DRIVER_FUNC_REMOVE      0x02

#define DRIVER_NAME       "NONPNP"
#define DEVICE_NAME       "\\\\.\\NONPNP\\nonpnpmonitor.log"
