set ARCHITECTURE=x64

if not exist build (
	mkdir build
)
cd build
cmake -A %ARCHITECTURE% ..
if not errorlevel 1 (
	cmake --build . --config Release
)
cd ..
for /f "delims=" %%r in ('echo %cmdcmdline% ^| find "\build.bat"') do pause
