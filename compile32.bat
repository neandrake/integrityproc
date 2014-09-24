@echo off
@del integrityproc.dll>NUL 2>&1

@if not "%XFD_COMPILE_32%" == "T" (
	@set XFD_COMPILE_32=T
	@call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\bin\vcvars32.bat"
)

@echo on
cl /D UNICODE /D _UNICODE /D _CRT_NON_CONFORMING_SWPRINTFS /D OS_ARCH_X86 -I "C:\Program Files (x86)\Java\jdk1.6.0_38\include" -I "C:\Program Files (x86)\Java\jdk1.6.0_38\include\win32" -I "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\atlmfc\include" /W3 /O1 /LD /EHsc -Feintegrityproc.dll src_cpp\*.cpp
@echo off

@del *.exp>NUL 2>&1
@del *.lib>NUL 2>&1
@del *.obj>NUL 2>&1
@del *.manifest>NUL 2>&1
