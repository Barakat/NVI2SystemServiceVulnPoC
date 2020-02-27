NTSTATUS __stdcall sub_1103C(PIO_STACK_LOCATION StackLocation, PIRP Irp, PULONG_PTR Information)
{
  DRIVER_INPUT *DriverInput; // ebx
  PFILE_OBJECT FileObject; // edi
  PDEVICE_OBJECT DeviceObject; // eax
  struct _OBJECT_NAME_INFORMATION *ObjectNameInformation; // ebx
  WCHAR *OuputBuffer; // [esp+10h] [ebp-24h]
  ULONG ReturnLength; // [esp+14h] [ebp-20h]
  NTSTATUS Status; // [esp+18h] [ebp-1Ch]
  CPPEH_RECORD ms_exc; // [esp+1Ch] [ebp-18h]

  Status = 0;
  ReturnLength = 0;
  ms_exc.registration.TryLevel = 0;
  DriverInput = (DRIVER_INPUT *)Irp->AssociatedIrp.SystemBuffer;
  OuputBuffer = (WCHAR *)DriverInput;
  FileObject = DriverInput->FileObject;
  if ( StackLocation->Parameters.DeviceIoControl.InputBufferLength != 4
    || StackLocation->Parameters.DeviceIoControl.OutputBufferLength != 0x414 )
  {
    DbgPrint("Buffer not correct");
    return STATUS_INVALID_PARAMETER;
  }
  if ( FileObject->Type != 5 )
  {
    DbgPrint("File type not supported");
    return STATUS_INVALID_PARAMETER;
  }
  DeviceObject = FileObject->DeviceObject;
  if ( DeviceObject->Type != 3 )
    return STATUS_INVALID_PARAMETER;
  DriverInput->DeviceType = DeviceObject->DeviceType;
  memset(DriverInput->Buffer, 0, 520u);
  memset(DriverInput, 0, 520u);
  memcpy(DriverInput->Buffer, FileObject->FileName.Buffer, FileObject->FileName.Length);
  ObjectNameInformation = (struct _OBJECT_NAME_INFORMATION *)ExAllocatePoolWithTag(PagedPool, 528u, 'drvr');
  ObjectNameInformation->Name.MaximumLength = 520;
  ObjectNameInformation->Name.Length = 260;
  if ( ObQueryNameString(FileObject->DeviceObject, ObjectNameInformation, 520u, &ReturnLength) < 0 )
    DbgPrint("ObQueryNameString failed");
  else
    memcpy(OuputBuffer, ObjectNameInformation->Name.Buffer, ObjectNameInformation->Name.Length);
  ExFreePoolWithTag(ObjectNameInformation, 0);
  *Information = 1044;
  ms_exc.registration.TryLevel = -2;
  return Status;
}