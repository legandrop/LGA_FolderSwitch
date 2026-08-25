@echo off
chcp 65001 >nul
REM Compilacion Windows con Ninja para LGA_FolderSwitch.

set BUILD_TYPE=Debug
set BUILD_DIR=build
set NO_RUN=false

:parse_args
if "%1"=="" goto after_args
if "%1"=="--release" (
    set BUILD_TYPE=Release
    set BUILD_DIR=build-release
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

REM La app corre en la bandeja: si queda una instancia viva, el .exe esta
REM bloqueado y el link falla con "Permission denied". Ademas el QLockFile
REM haria salir en silencio a la instancia nueva.
taskkill /F /IM LGA_FolderSwitch.exe >nul 2>&1

if not exist %BUILD_DIR% mkdir %BUILD_DIR%
cd %BUILD_DIR%

if not exist "CMakeCache.txt" goto :configure
goto :compile

:configure
echo Configurando CMake con Ninja (%BUILD_TYPE%)...
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

if "%NO_RUN%"=="true" exit /b 0

echo Iniciando LGA_FolderSwitch...
start "" "%~dp0%BUILD_DIR%\LGA_FolderSwitch.exe"
exit /b 0
