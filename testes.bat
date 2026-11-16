@echo off
REM Compila e roda os testes da lógica (não precisa da raylib, só do compilador).
REM Usa o g++ que vem junto com o instalador da raylib, em C:\raylib.

set PATH=C:\raylib\w64devkit\bin;%PATH%
REM deixa o terminal mostrar os acentos certinho
chcp 65001 >nul

g++ -std=c++17 -Wall -Wextra testes.cpp ^
    jogo.cpp reino.cpp cartas.cpp herois.cpp inimigos.cpp combate.cpp expedicao.cpp ^
    estruturas.cpp algoritmos.cpp aleatorio.cpp ^
    -o testes.exe

if errorlevel 1 (
    echo.
    echo Deu erro na compilacao, olha as mensagens acima.
    pause
    exit /b 1
)

testes.exe
pause
