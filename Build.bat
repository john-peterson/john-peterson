set BUILDTYPE=Build
set BUILDCONFIG=Release
set PLATFORM=Any CPU
call "%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"
devenv /nologo %1 /%BUILDTYPE% "%BUILDCONFIG%|%PLATFORM%"
if %ERRORLEVEL% neq 0 (
	@echo %1 %2 failed
	pause
	exit
)