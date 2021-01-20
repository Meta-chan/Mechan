set TD_DIR=E:/Project/Mechan/td
set VCPKG_DIR=E:/Project/_lib/vcpkg
set IR_DIR=E:/Project/_ir
REM set ARCH=-A x64

if not exist data (
	mkdir data
)
if not exist build (
	mkdir build
)
cd build
cmake %ARCH% -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE:FILEPATH=%VCPKG_DIR%/scripts/buildsystems/vcpkg.cmake -DTd_DIR=%TD_DIR%/lib/cmake/Td -DIr_DIR=%IR_DIR% ..
cmake --build . --config Release
cd ..
for /f "delims=" %%r in ('echo %cmdcmdline% ^| find "\build.bat"') do pause