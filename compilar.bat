@echo off
REM Compila o Kingdoms no Windows.
REM Usa o instalador oficial da raylib, que fica em C:\raylib
REM (já vem com o compilador w64devkit junto).

set RAYLIB=C:\raylib\raylib\src
set PATH=C:\raylib\w64devkit\bin;%PATH%

g++ -std=c++17 -Wall -Wextra ^
    main.cpp jogo.cpp interface.cpp ui.cpp tela_batalha.cpp tela_herois.cpp figuras.cpp ^
    pincel.cpp natureza.cpp visao3d.cpp fonte.cpp sprites.cpp ^
    reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp expedicao.cpp ^
    estruturas.cpp algoritmos.cpp aleatorio.cpp ^
    -o kingdoms.exe -I%RAYLIB% -L%RAYLIB% ^
    -lraylib -lopengl32 -lgdi32 -lwinmm

if errorlevel 1 (
    echo.
    echo Deu erro na compilacao, olha as mensagens acima.
    pause
    exit /b 1
)

echo Compilou! Abrindo o jogo...
start kingdoms.exe
