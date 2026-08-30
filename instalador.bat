@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"
set "SCRIPT_DIR=%~dp0"
set "INSTALLER_DIR=%SCRIPT_DIR%installer"
REM El repo es PUBLICO y la release se publica en el MISMO repo del codigo:
REM legandrop/LGA_FolderSwitch. El tag del codigo y el de la release son el
REM mismo tag: se crea una sola vez, se pushea a origin, y se publica ahi.
set "PUBLIC_RELEASE_REPO=legandrop/LGA_FolderSwitch"
set "GITHUB_READY=true"
set "GH_CMD="
set "GITHUB_LOCAL_ONLY_CONFIRMED=false"

echo.
echo ============================================================
echo  Preflight checks de Git/GitHub
echo ============================================================
echo.

where git >nul 2>nul
if !errorlevel! EQU 0 (
    echo OK: Git disponible en PATH.
) else (
    echo ERROR: Git no esta disponible en PATH.
    echo Abortando...
    pause
    exit /b 1
)

git rev-parse --show-toplevel >nul 2>nul
if !errorlevel! EQU 0 (
    echo OK: Repositorio Git valido.
) else (
    echo ERROR: Esta carpeta no es un repositorio Git valido.
    echo Abortando...
    pause
    exit /b 1
)

for /f "tokens=*" %%B in ('git rev-parse --abbrev-ref HEAD 2^>nul') do set "CURRENT_BRANCH=%%B"
if "!CURRENT_BRANCH!"=="" (
    echo ERROR: No se pudo detectar el branch activo de Git.
    echo Abortando...
    pause
    exit /b 1
) else (
    echo OK: Branch activo detectado: !CURRENT_BRANCH!
)

if /i "!CURRENT_BRANCH!" NEQ "main" (
    echo ERROR: El branch activo es '!CURRENT_BRANCH!' y este instalador debe generarse desde 'main'.
    echo Abortando...
    pause
    exit /b 1
)

set "HAS_PREEXISTING_CHANGES=false"
for /f "tokens=*" %%S in ('git status --porcelain 2^>nul') do (
    set "HAS_PREEXISTING_CHANGES=true"
)

if "!HAS_PREEXISTING_CHANGES!"=="true" (
    echo.
    echo ERROR: Hay cambios sin commitear antes de crear el instalador:
    git status --short
    echo.
    echo Abortando para evitar mezclar cambios previos con el commit del installer.
    pause
    exit /b 1
) else (
    echo OK: El repositorio esta limpio antes de crear el instalador.
)

where gh >nul 2>nul
if !errorlevel! EQU 0 (
    set "GH_CMD=gh"
) else (
    if exist "C:\Program Files\GitHub CLI\gh.exe" (
        set "GH_CMD=C:\Program Files\GitHub CLI\gh.exe"
    )
)

if "!GH_CMD!"=="" (
    echo AVISO: GitHub CLI [gh] no esta instalado.
    set "GITHUB_READY=false"
) else (
    echo OK: GitHub CLI encontrado.
    "!GH_CMD!" auth status >nul 2>nul
    if !errorlevel! NEQ 0 (
        echo AVISO: GitHub CLI no esta autenticado.
        set "GITHUB_READY=false"
    ) else (
        echo OK: GitHub CLI autenticado.
        git ls-remote --exit-code origin HEAD >nul 2>nul
        if !errorlevel! NEQ 0 (
            echo AVISO: No se pudo acceder al remoto origin.
            set "GITHUB_READY=false"
        ) else (
            echo OK: Acceso al remoto origin confirmado.
            "!GH_CMD!" repo view "%PUBLIC_RELEASE_REPO%" --json name -q .name >nul 2>nul
            if !errorlevel! NEQ 0 (
                echo AVISO: No se pudo acceder al repo publico de releases %PUBLIC_RELEASE_REPO%.
                set "GITHUB_READY=false"
            ) else (
                echo OK: Acceso al repo publico de releases confirmado.
            )
        )
    )
)

if /i "!GITHUB_READY!" NEQ "true" (
    echo.
    echo Los chequeos de GitHub fallaron.
    echo Se podra generar el instalador local, pero no publicar la release desde este .bat.
    echo.
    choice /C YN /M "Continuar solo con la generacion local del instalador?"
    if !errorlevel! NEQ 1 (
        echo Operacion cancelada por el usuario.
        pause
        exit /b 1
    )
    echo OK: Se continuara solo con la generacion local del instalador.
    set "GITHUB_LOCAL_ONLY_CONFIRMED=true"
)

echo Creando instalador de LGA FolderSwitch...

REM Buscar Inno Setup. Va ANTES del build: si falta, enterarse en dos segundos y
REM no despues de una compilacion completa.
set ISCC=""
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" set ISCC="C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if exist "C:\Program Files\Inno Setup 6\ISCC.exe" set ISCC="C:\Program Files\Inno Setup 6\ISCC.exe"

if %ISCC%=="" (
    echo ERROR: No se encontro Inno Setup 6.
    echo Descargar desde: https://jrsoftware.org/isdl.php
    exit /b 1
)

REM ============================================================================
REM VERSION: la fuente unica es CMakeLists.txt (`project(LGA_FolderSwitch
REM VERSION x.y ...)`). No hay archivo VERSION ni sync_version.bat: se extrae
REM directo de la linea `project(...)` antes de compilar.
REM ============================================================================
echo Extrayendo version desde CMakeLists.txt...
set "VERSION="
for /f "tokens=1-4" %%A in ('findstr /B /C:"project(LGA_FolderSwitch" CMakeLists.txt') do set "VERSION=%%C"

if "%VERSION%"=="" (
    echo ERROR: No se pudo extraer la version desde CMakeLists.txt.
    echo        Se esperaba una linea del tipo: project^(LGA_FolderSwitch VERSION x.y LANGUAGES CXX^)
    exit /b 1
)
echo OK: Version detectada: %VERSION%

REM Cerrar proceso activo para evitar bloqueos durante deploy/installer
taskkill /F /IM LGA_FolderSwitch.exe 2>nul

REM Ejecutar deploy automaticamente (sin abrir la app al finalizar)
echo Ejecutando deploy.bat --no-run...
call "%~dp0deploy.bat" --no-run
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo deploy.bat. Abortando creacion de instalador.
    exit /b 1
)

REM ============================================================================
REM GUARD: el binario compilado tiene que llevar la version que se va a publicar.
REM
REM Se busca una cadena dentro del binario porque el .exe no lleva recurso
REM VERSIONINFO (FileVersion/ProductVersion vienen vacios) y la app no tiene un
REM flag `--version`: el string compilado es la unica evidencia disponible.
REM
REM Se busca el MARCADOR LGA_FOLDERSWITCH_BUILD_VERSION y NO el numero suelto,
REM para no confundirlo con otros literales 0.NNN que pueda traer el binario.
REM ============================================================================
echo Verificando que el binario reporte la version %VERSION%...
findstr /C:"LGA_FOLDERSWITCH_BUILD_VERSION=%VERSION%" "%~dp0deploy\LGA_FolderSwitch.exe" >nul 2>nul
if errorlevel 1 (
    echo ERROR: deploy\LGA_FolderSwitch.exe NO contiene la version %VERSION%.
    echo        El binario se compilo con otra version, asi que el instalador
    echo        publicaria un numero que la app no reporta.
    echo        Revisar que CMakeLists.txt y el binario compilado coincidan.
    exit /b 1
)
echo OK: El binario reporta la version %VERSION%.

if /i "!GITHUB_READY!"=="true" (
    git rev-parse -q --verify "refs/tags/v%VERSION%" >nul 2>nul
    if !errorlevel! EQU 0 (
        echo AVISO: El tag local v%VERSION% ya existe.
        set "GITHUB_READY=false"
    ) else (
        git ls-remote --exit-code --tags origin "refs/tags/v%VERSION%" >nul 2>nul
        if !errorlevel! EQU 0 (
            echo AVISO: El tag remoto v%VERSION% ya existe.
            set "GITHUB_READY=false"
        ) else (
            "!GH_CMD!" release view "v%VERSION%" --repo "%PUBLIC_RELEASE_REPO%" >nul 2>nul
            if !errorlevel! EQU 0 (
                echo AVISO: La release v%VERSION% ya existe en %PUBLIC_RELEASE_REPO%.
                set "GITHUB_READY=false"
            ) else (
                echo OK: El tag v%VERSION% esta disponible.
            )
        )
    )
)

if /i "!GITHUB_READY!" NEQ "true" if /i "!GITHUB_LOCAL_ONLY_CONFIRMED!" NEQ "true" (
    echo.
    echo No se podra publicar la release v%VERSION% desde este .bat.
    echo Se podra generar el instalador local igualmente.
    echo.
    choice /C YN /M "Continuar solo con la generacion local del instalador?"
    if !errorlevel! NEQ 1 (
        echo Operacion cancelada por el usuario.
        pause
        exit /b 1
    )
    echo OK: Se continuara solo con la generacion local del instalador.
    set "GITHUB_LOCAL_ONLY_CONFIRMED=true"
)

if not exist "%INSTALLER_DIR%" mkdir "%INSTALLER_DIR%"

set "OUTPUT_EXE=%INSTALLER_DIR%\LGA_FolderSwitch_Setup_v%VERSION%.exe"
if exist "%OUTPUT_EXE%" del /F /Q "%OUTPUT_EXE%" >nul 2>nul

%ISCC% /DMyAppVersion=%VERSION% LGA_FolderSwitch_installer.iss
if %ERRORLEVEL% neq 0 (
    echo Aviso: primer intento de compilacion fallo. Reintentando en 5 segundos...
    ping 127.0.0.1 -n 6 >nul
    %ISCC% /DMyAppVersion=%VERSION% LGA_FolderSwitch_installer.iss
)
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo la compilacion del instalador con Inno Setup.
    exit /b 1
)

echo.
echo Instalador creado: %OUTPUT_EXE%

REM ---------------------------------------------------------------- instalar local
REM Primero se ofrece instalar / revelar el .exe; recien despues commit + GitHub.
REM El instalador corre en primer plano para que las preguntas de release aparezcan
REM recien cuando el wizard cierra.
echo.
choice /C YN /M "Desea ejecutar el instalador ahora mismo (instalar local)?"
if !errorlevel! EQU 1 (
    echo Ejecutando el instalador...
    "%OUTPUT_EXE%"
) else (
    echo Instalador no ejecutado.
)

choice /C YN /M "Desea revelar el instalador en Windows Explorer?"
if !errorlevel! EQU 1 (
    explorer /select,"%OUTPUT_EXE%"
) else (
    echo No se abrio Windows Explorer.
)

if /i "!GITHUB_READY!" NEQ "true" goto :END

echo.
echo ============================================================
echo  Commit y release opcional
echo ============================================================
echo.

set "RELEASE_ALLOWED=true"
set "COMMIT_CREATED=false"
set "HAS_INSTALLER_CHANGES=false"
for /f "tokens=*" %%S in ('git status --porcelain 2^>nul') do (
    set "HAS_INSTALLER_CHANGES=true"
)

if "!HAS_INSTALLER_CHANGES!"=="true" (
    echo Cambios detectados despues de crear el instalador:
    git status --short
    echo.
    choice /C YN /M "Desea commitear estos cambios como installer_v%VERSION%?"
    if !errorlevel! EQU 1 (
        echo Haciendo commit de cambios...
        git add -A
        if !errorlevel! NEQ 0 (
            echo ERROR: git add fallo.
            set "RELEASE_ALLOWED=false"
        ) else (
            git commit -m "installer_v%VERSION%"
            if !errorlevel! NEQ 0 (
                echo AVISO: git commit retorno codigo !errorlevel!.
                echo Puede que no haya cambios nuevos o que haya ocurrido un error.
                echo.
                choice /C YN /M "Desea continuar con la release sin un commit nuevo?"
                if !errorlevel! NEQ 1 (
                    set "RELEASE_ALLOWED=false"
                )
            ) else (
                set "COMMIT_CREATED=true"
            )
        )
    ) else (
        echo Commit omitido por el usuario.
        echo No se ofrecera publicar release para evitar taggear un estado no commiteado.
        set "RELEASE_ALLOWED=false"
    )
) else (
    echo No hay cambios nuevos para commitear.
    echo.
    choice /C YN /M "Desea continuar y ofrecer la publicacion de release igualmente?"
    if !errorlevel! NEQ 1 (
        set "RELEASE_ALLOWED=false"
    )
)

if /i "!COMMIT_CREATED!"=="true" (
    echo Haciendo push a origin/!CURRENT_BRANCH!...
    git push origin "!CURRENT_BRANCH!"
    if !errorlevel! NEQ 0 (
        echo ERROR: git push fallo.
        echo Verificar conexion a internet y permisos del repositorio.
        set "RELEASE_ALLOWED=false"
    )
)

if /i "!RELEASE_ALLOWED!" NEQ "true" goto :END

echo.
choice /C YN /M "Desea subir el instalador como release v%VERSION% a GitHub?"
if !errorlevel! NEQ 1 goto :END

if not exist "%OUTPUT_EXE%" (
    echo ERROR: No se encontro el instalador: %OUTPUT_EXE%
    goto :END
)

echo.
echo Creando tag v%VERSION%...
git tag -a "v%VERSION%" -m "Release v%VERSION%"
if !errorlevel! NEQ 0 (
    echo ERROR: No se pudo crear el tag v%VERSION%.
    echo Es posible que el tag ya exista.
    goto :END
)

echo Haciendo push del tag v%VERSION%...
git push origin "v%VERSION%"
if !errorlevel! NEQ 0 (
    echo ERROR: No se pudo hacer push del tag v%VERSION%.
    echo El tag local fue creado, pero no se publico en origin.
    goto :END
)

echo.
echo Creando release en GitHub...
"!GH_CMD!" release create "v%VERSION%" "%OUTPUT_EXE%" --repo "%PUBLIC_RELEASE_REPO%" --target "main" --title "v%VERSION%" --notes "Release v%VERSION%"
if !errorlevel! NEQ 0 (
    echo ERROR: No se pudo crear la release en GitHub.
    echo.
    echo El commit y el push del branch ya fueron hechos si correspondia.
    echo El tag v%VERSION% ya fue creado y subido a origin.
    echo.
    echo Recomendado: borrar el tag local/remoto para dejar GitHub limpio
    echo y volver a intentar la release despues.
    echo.
    choice /C YN /M "Desea borrar el tag v%VERSION% local/remoto ahora?"
    if !errorlevel! EQU 1 (
        "!GH_CMD!" release delete "v%VERSION%" --repo "%PUBLIC_RELEASE_REPO%" --yes >nul 2>nul
        git tag -d "v%VERSION%" >nul 2>nul
        git push origin ":refs/tags/v%VERSION%" >nul 2>nul
        echo Tag v%VERSION% borrado local/remoto.
    ) else (
        echo Se conserva el tag v%VERSION%.
        echo Puede crear la release manualmente desde:
        echo https://github.com/%PUBLIC_RELEASE_REPO%/releases
    )
    goto :END
)

echo.
echo ============================================================
echo  Release v%VERSION% publicada exitosamente en GitHub!
echo  https://github.com/%PUBLIC_RELEASE_REPO%/releases/tag/v%VERSION%
echo ============================================================

REM ---- Avisarle al manifiesto de versiones que hay algo nuevo ----
REM Sin esto hay que esperar al cron de legandrop/LGA_Updates, que corre cada 30 minutos:
REM hasta entonces el card de LGA Updates de PipeSync no ve la version recien publicada.
REM
REM Falla en SILENCIO a proposito. La release ya esta publicada y el cron la va a levantar
REM igual, asi que no tiene sentido ensuciar el final de una publicacion exitosa con un
REM error por algo que se arregla solo.
REM
REM El `<nul` cierra la entrada estandar. Hoy `refresh_versions.yml` no declara inputs en su
REM `workflow_dispatch`, pero si alguien le agrega uno, `gh` pasa a modo interactivo y pide
REM el valor por un prompt que aca esta redirigido a nul: el instalador quedaria colgado, en
REM silencio y justo despues de una publicacion exitosa.
"!GH_CMD!" workflow run refresh_versions.yml --repo legandrop/LGA_Updates >nul 2>nul <nul
if !errorlevel! EQU 0 (
    echo Manifiesto de versiones: refresco disparado.
) else (
    echo AVISO: no se pudo disparar el refresco del manifiesto. El cron lo levanta solo.
)

:END
endlocal
