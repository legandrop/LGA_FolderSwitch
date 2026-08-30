@echo off
setlocal
cd /d "%~dp0"

set "NO_RUN=false"
set "BUILD_DIR=build-release"
set "QT_DIR=C:\Qt\6.5.3\mingw_64"
set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"

:parse_args
if "%~1"=="" goto after_args
if /I "%~1"=="--no-run" set "NO_RUN=true"
shift
goto parse_args

:after_args
echo Implementando LGA FolderSwitch...

taskkill /F /IM LGA_FolderSwitch.exe >nul 2>&1

echo Compilando (modo Release) via compilar.bat...
call "%~dp0compilar.bat" --release --no-run
if %ERRORLEVEL% neq 0 (
    echo Error en la compilacion.
    exit /b 1
)

if not exist "%BUILD_DIR%\LGA_FolderSwitch.exe" (
    echo ERROR: no se genero %BUILD_DIR%\LGA_FolderSwitch.exe
    exit /b 1
)

echo Limpiando carpeta de deploy anterior...
if exist deploy rmdir /S /Q deploy
mkdir deploy

echo Copiando ejecutable...
copy /Y "%BUILD_DIR%\LGA_FolderSwitch.exe" deploy\ >nul

echo Copiando runtime Qt y MinGW (dependencias directas del exe)...
copy /Y "%QT_DIR%\bin\Qt6Core.dll" deploy\ >nul
copy /Y "%QT_DIR%\bin\Qt6Gui.dll" deploy\ >nul
copy /Y "%QT_DIR%\bin\Qt6Widgets.dll" deploy\ >nul
copy /Y "%QT_DIR%\bin\Qt6Network.dll" deploy\ >nul
copy /Y "%MINGW_BIN%\libgcc_s_seh-1.dll" deploy\ >nul
copy /Y "%MINGW_BIN%\libstdc++-6.dll" deploy\ >nul
copy /Y "%MINGW_BIN%\libwinpthread-1.dll" deploy\ >nul

if not exist deploy\platforms mkdir deploy\platforms
copy /Y "%QT_DIR%\plugins\platforms\qwindows.dll" deploy\platforms\ >nul

REM Plugin TLS: sin el, QNetworkAccessManager no puede hacer HTTPS (falla en
REM silencio en el deploy portable, aunque el mismo exe ande bien corriendo
REM desde el arbol de Qt). Lo necesita el chequeo de updates.
if not exist deploy\tls mkdir deploy\tls
copy /Y "%QT_DIR%\plugins\tls\qschannelbackend.dll" deploy\tls\ >nul

set PATH=%PATH%;%QT_DIR%\bin;%MINGW_BIN%

REM windeployqt resuelve la clausura transitiva completa (ICU, ANGLE, etc. si
REM aplicaran). En este entorno, windeployqt detecta mal debug/release en los
REM plugins de C:\Qt\6.5.3\mingw_64\plugins (los reporta como "debug" pese a
REM ser release) y aborta con "Unable to find the platform plugin" ANTES de
REM copiar nada. Por eso las dependencias criticas (arriba) se copian a mano
REM primero, y windeployqt corre despues solo como complemento best-effort:
REM si falla, no es fatal, porque deploy\ ya tiene lo necesario para arrancar.
echo Ejecutando windeployqt como complemento (best-effort)...
set "WINDEPLOYQT=%QT_DIR%\bin\windeployqt.exe"
if not exist "%WINDEPLOYQT%" (
    for /f "delims=" %%W in ('where windeployqt.exe 2^>nul') do set "WINDEPLOYQT=%%W"
)
if exist "%WINDEPLOYQT%" (
    "%WINDEPLOYQT%" --release --compiler-runtime --no-translations --no-opengl-sw --no-system-d3d-compiler --dir deploy deploy\LGA_FolderSwitch.exe
    if errorlevel 1 echo ADVERTENCIA: windeployqt devolvio error ^(no fatal, ver comentario arriba^).
) else (
    echo ADVERTENCIA: no se encontro windeployqt.exe. Se continua solo con las copias manuales.
)

echo Verificando dependencias criticas...
set DEPS_MISSING=false
if not exist "deploy\Qt6Core.dll" set DEPS_MISSING=true
if not exist "deploy\Qt6Gui.dll" set DEPS_MISSING=true
if not exist "deploy\Qt6Widgets.dll" set DEPS_MISSING=true
if not exist "deploy\Qt6Network.dll" set DEPS_MISSING=true
if not exist "deploy\libgcc_s_seh-1.dll" set DEPS_MISSING=true
if not exist "deploy\libstdc++-6.dll" set DEPS_MISSING=true
if not exist "deploy\libwinpthread-1.dll" set DEPS_MISSING=true
if not exist "deploy\platforms\qwindows.dll" set DEPS_MISSING=true
if not exist "deploy\tls\qschannelbackend.dll" set DEPS_MISSING=true

if "%DEPS_MISSING%"=="true" (
    echo ERROR: faltan dependencias criticas de Qt en deploy\. Verifica la instalacion de Qt 6.5.3 mingw_64.
    exit /b 1
)
echo Dependencias criticas verificadas.

echo.
echo Implementacion completada. App portable en carpeta 'deploy'.
echo.
if /I "%NO_RUN%"=="true" goto skip_run
start deploy\LGA_FolderSwitch.exe
goto end_run

:skip_run
echo Omitiendo ejecucion de LGA_FolderSwitch (--no-run).

:end_run
endlocal
exit /b 0
