integrityproc
=============

Windows utility for launching a process with 'un-elevated' privileges

A DLL with JNI wrapper which allows running a process using a clone of the explorer.exe process.

When running a process as administrator/UAC the process' privileges are elevated which causes any 
spawned processes to inherit the elevated privileges. This can result in strange behavior when not desired.

The simple use case for using this would be an installer (which typically requires elevated privilege) that
launches its application when finished installing.

For example, network drives are generally mapped by the user account, and starting on Windows 8 any process
running as administrator/UAC will not be able to see these drives.

* https://connect.microsoft.com/VisualStudio/feedback/details/767400/folderbrowserdialog-does-not-show-network-drives-on-windows-8

Resources:
 * http://www.codeproject.com/Tips/23090/Creating-a-Process-with-Medium (see Elmue's comments)
 * http://msdn.microsoft.com/en-us/library/bb625964.aspx
