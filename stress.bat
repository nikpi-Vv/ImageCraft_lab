@echo off
set EXE=image_craft.exe

echo ==== STRESS: 50 раз heavy-цепочка ====
for /L %%i in (1,1,50) do (
    echo Run %%i
    %EXE% in.bmp stress_%%i.bmp -crop 1024 1024 -med 9 -blur 5 -crystallize 8 -glass 3
)
pause
