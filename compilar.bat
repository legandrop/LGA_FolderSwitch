@echo off
chcp 65001 >nul
REM Motor de compilacion Windows con Ninja para LGA_FolderSwitch.
REM Estructura tomada de LGA_LinkRedirector\compilar.bat.

set BUILD_TYPE=Debug
set BUILD_DIR=build
set NO_RUN=false
set FORCE_CLEAN=false
set PARALLEL_CORES=%NUMBER_OF_PROCESSORS%

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
if "%1"=="--force-clean" (
    set FORCE_CLEAN=true
    shift
    goto parse_args
)
shift
goto parse_args

:after_args
cd /d "%~dp0"

REM La app vive en la bandeja: una instancia viva bloquea el .exe y el link falla
REM con "Permission denied". Ademas el QLockFile haria salir en silencio a la nueva.
taskkill /F /IM LGA_FolderSwitch.exe >nul 2>&1
ping -n 2 127.0.0.1 >nul

REM Rutas de las toolchains. El bloque de dependencias las usa en vez de repetir
REM el literal.
set "QT_DIR=C:\Qt\6.5.3\mingw_64"
set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"

set PATH=%PATH%;%QT_DIR%\bin;%MINGW_BIN%;C:\Qt\Tools\Ninja

if "%FORCE_CLEAN%"=="true" (
    echo Limpiando build anterior...
    rmdir /s /q %BUILD_DIR% 2>nul
)

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
echo Compilando con %PARALLEL_CORES% nucleos usando Ninja...
ninja -j%PARALLEL_CORES%
if %ERRORLEVEL% neq 0 (
    echo Error en compilacion
    cd ..
    exit /b 1
)

cd ..

REM ============================================================
REM  DEPENDENCIAS DE RUNTIME: verificar -> copiar -> windeployqt -> verificar
REM
REM  Sin esto el .exe depende del PATH para encontrar Qt, y eso rompe al arrancar
REM  con Windows: la Run key lo lanza sin el PATH del script y muere con
REM  "Qt6Widgets.dll was not found" antes de escribir una sola linea de log.
REM  Peor: cuando SI habia algo en el PATH, cargaba Qt 6.8.2 y el runtime de
REM  MinGW que trae Git, o sea ni la version ni el compilador con los que se
REM  compilo. Con las DLL al lado del exe, Windows resuelve por directorio de la
REM  aplicacion y el PATH deja de importar.
REM ============================================================

REM Lista canonica, relativa al arbol de build. Derivada de find_package(Qt6
REM COMPONENTS Core Gui Widgets Svg) en CMakeLists.txt.
set "DEP_LIST=libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll Qt6Core.dll Qt6Gui.dll Qt6Widgets.dll Qt6Svg.dll platforms\qwindows.dll"

echo Verificando dependencias de runtime...
set DEPS_MISSING=false
for %%D in (%DEP_LIST%) do call :check_dep "%%D"

if "%DEPS_MISSING%"=="true" call :copy_missing_deps
if "%DEPS_MISSING%"=="true" (
    set DEPS_MISSING=false
    for %%D in (%DEP_LIST%) do call :check_dep "%%D"
)

REM Ultimo recurso: windeployqt, el unico que resuelve la clausura transitiva.
if "%DEPS_MISSING%"=="true" call :run_windeployqt
if "%DEPS_MISSING%"=="true" (
    set DEPS_MISSING=false
    for %%D in (%DEP_LIST%) do call :check_dep "%%D"
)

if "%DEPS_MISSING%"=="true" (
    echo.
    echo ERROR: faltan dependencias de runtime y no se pudieron reparar.
    echo        Verifica que Qt 6.5.3 mingw_64 este instalado en "%QT_DIR%".
    exit /b 1
)
echo Dependencias de runtime verificadas.

if not exist "%BUILD_DIR%\LGA_FolderSwitch.exe" (
    echo ERROR: CMake no genero %BUILD_DIR%\LGA_FolderSwitch.exe.
    exit /b 1
)

echo Compilacion completada a las %TIME%

if "%NO_RUN%"=="true" (
    echo Ejecucion omitida ^(--no-run^).
    exit /b 0
)

echo Iniciando LGA_FolderSwitch...
start "" "%~dp0%BUILD_DIR%\LGA_FolderSwitch.exe"
exit /b 0

REM ============================================================
REM  Subrutinas
REM
REM  Van en subrutinas y no inline a proposito: cmd.exe expande los %VAR% de un
REM  bloque `if (...)` al PARSEARLO, no al ejecutarlo, asi que una variable que
REM  se escribe y se lee dentro del mismo bloque lee siempre el valor viejo.
REM ============================================================

:check_dep
if not exist "%BUILD_DIR%\%~1" (
    echo    [falta] %BUILD_DIR%\%~1
    set DEPS_MISSING=true
)
goto :eof

:copy_missing_deps
echo.
echo Faltan dependencias de runtime. Copiandolas...

REM Sin `2>nul`: una copia que falla tiene que verse.
call :copy_dep "%MINGW_BIN%" "" libgcc_s_seh-1.dll
call :copy_dep "%MINGW_BIN%" "" libstdc++-6.dll
call :copy_dep "%MINGW_BIN%" "" libwinpthread-1.dll
call :copy_dep "%QT_DIR%\bin" "" Qt6Core.dll
call :copy_dep "%QT_DIR%\bin" "" Qt6Gui.dll
call :copy_dep "%QT_DIR%\bin" "" Qt6Widgets.dll
call :copy_dep "%QT_DIR%\bin" "" Qt6Svg.dll
call :copy_dep "%QT_DIR%\plugins\platforms" "platforms" qwindows.dll
goto :eof

:run_windeployqt
set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"
if not exist "%WINDEPLOYQT%" (
    for /f "delims=" %%W in ('where windeployqt.exe 2^>nul') do set "WINDEPLOYQT=%%W"
)
if not exist "%WINDEPLOYQT%" (
    echo ADVERTENCIA: no se encontro windeployqt.exe.
    goto :eof
)
echo Todavia faltan dependencias: probando con windeployqt...
"%WINDEPLOYQT%" --release --compiler-runtime --no-translations --no-opengl-sw --no-system-d3d-compiler --dir %BUILD_DIR% %BUILD_DIR%\LGA_FolderSwitch.exe
if errorlevel 1 echo ADVERTENCIA: windeployqt devolvio error.
goto :eof

:copy_dep
REM %1 = directorio origen, %2 = subdirectorio dentro del build [puede ir vacio],
REM %3 = nombre del archivo.
set "DEP_DEST=%BUILD_DIR%"
if not "%~2"=="" set "DEP_DEST=%BUILD_DIR%\%~2"
if exist "%DEP_DEST%\%~3" goto :eof
if not exist "%DEP_DEST%" mkdir "%DEP_DEST%"
if not exist "%~1\%~3" (
    echo    ERROR: no existe el origen "%~1\%~3"
    goto :eof
)
copy /Y "%~1\%~3" "%DEP_DEST%\" >nul
if errorlevel 1 (
    echo    ERROR: fallo la copia de "%~1\%~3"
) else (
    echo    [ok] %~3
)
goto :eof
