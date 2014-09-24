@echo off
@del integrityproc.dll>NUL 2>&1

@if not "%XFD_COMPILE_64%" == "T" (
	@set XFD_COMPILE_64=T
	@call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\x86_amd64\vcvarsx86_amd64.bat"
)

@echo on
cl /D UNICODE /D _UNICODE /D OS_ARCH_X64 -I "C:\Program Files\Java\jdk1.6.0_38\include" -I "C:\Program Files\Java\jdk1.6.0_38\include\win32" -I "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\atlmfc\include" /W3 /O1 /LD /EHsc -Feintegrityproc64.dll src_cpp\*.cpp
@echo off

@del *.exp>NUL 2>&1
@del *.lib>NUL 2>&1
@del *.obj>NUL 2>&1
@del *.manifest>NUL 2>&1
