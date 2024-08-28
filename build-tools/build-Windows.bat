set VISUAL_STUDIO="Visual Studio 17 2022"

set BUILD_DIR=%CD%\..\build.Windows
set BUILD_TYPE="Release"
set OUTPUT_DIR=%BUILD_DIR%\output\
set HEADER_OUTPUT_DIR=%OUTPUT_DIR%\include\

del /s /f /q %BUILD_DIR%
mkdir %BUILD_DIR%
mkdir %OUTPUT_DIR%
cd %BUILD_DIR%

cmake ..\build-tools ^
    -DCMAKE_INSTALL_PREFIX=. ^
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
    -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=%OUTPUT_DIR% ^
REM -G %VISUAL_STUDIO%

cmake --build . --config %BUILD_TYPE%

mkdir %HEADER_OUTPUT_DIR%
copy %BUILD_DIR%\..\include\EngineXX.h %HEADER_OUTPUT_DIR%
copy %BUILD_DIR%\..\include\EngineXXLog.h %HEADER_OUTPUT_DIR%
copy %BUILD_DIR%\..\include\EngineXXLua.h %HEADER_OUTPUT_DIR%
copy %BUILD_DIR%\..\include\EngineXXNet.h %HEADER_OUTPUT_DIR%
