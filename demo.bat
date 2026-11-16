@echo off
REM Compila e roda a demonstração no terminal (estruturas, buscas e um turno).
REM Usa o g++ que vem junto com o instalador da raylib, em C:\raylib.

set PATH=C:\raylib\w64devkit\bin;%PATH%
REM deixa o terminal mostrar os acentos certinho
chcp 65001 >nul

g++ -std=c++17 -Wall -Wextra -O2 demo.cpp ^
    jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp expedicao.cpp ^
    estruturas.cpp algoritmos.cpp aleatorio.cpp ^
    -o demo.exe

if errorlevel 1 (
    echo.
    echo Deu erro na compilacao, olha as mensagens acima.
    pause
    exit /b 1
)

demo.exe
pause
