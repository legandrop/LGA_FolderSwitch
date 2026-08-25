@echo off
chcp 65001 >nul
REM Compilacion Windows con Ninja para LGA_FolderSwitch.

set BUILD_TYPE=Debug
set NO_RUN=false

:parse_args
if "%1"=="" goto after_args
if "%1"=="--release" (
    set BUILD_TYPE=Release
    shift
    goto parse_args
)
if "%1"=="--no-run" (
    set NO_RUN=true
    shift
    goto parse_args
)
shift
goto parse_args

:after_args
cd /d "%~dp0"

set PATH=C:\Qt\6.5.3\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;%PATH%

if not exist build mkdir build
cd build

if not exist "CMakeCache.txt" goto :configure
goto :compile

:configure
echo Configurando CMake con Ninja...
cmake .. -G "Ninja" -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/mingw_64" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
if %ERRORLEVEL% neq 0 (
    echo Error en configuracion CMake
    cd ..
    exit /b 1
)

:compile
echo Compilando con Ninja...
ninja
if %ERRORLEVEL% neq 0 (
    echo Error en compilacion
    cd ..
    exit /b 1
)

cd ..
echo Compilacion completada.

if "%NO_RUN%"=="true" (
    exit /b 0
)
exit /b 0
