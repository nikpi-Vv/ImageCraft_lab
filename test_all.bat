@echo off
set EXE=image_craft.exe
set GEN=gen_bmps.exe

echo ==== Генерация тестовых BMP ====
if exist %GEN% (
    %GEN%
) else (
    echo WARNING: %GEN% not found, пропускаю генерацию.
)
echo.

echo ==== 2. Неверные имена фильтров и формат ====
%EXE% in.bmp out.bmp crop 10 10
%EXE% in.bmp out.bmp -unknown 1 2
%EXE% in.bmp out.bmp -gs qwe
echo.

echo ==== 3. Некорректные параметры фильтров ====
%EXE% in.bmp out.bmp -crop
%EXE% in.bmp out.bmp -crop 0 0
%EXE% in.bmp out.bmp -crop -10 100
%EXE% in.bmp out.bmp -edge -0.1
%EXE% in.bmp out.bmp -edge 1.5
%EXE% in.bmp out.bmp -edge abc
%EXE% in.bmp out.bmp -blur 0
%EXE% in.bmp out.bmp -blur -1
%EXE% in.bmp out.bmp -glass 0
%EXE% in.bmp out.bmp -glass -2
%EXE% in.bmp out.bmp -crystallize 0
%EXE% in.bmp out.bmp -crystallize -5
echo.

echo ==== 4. Огромные параметры (устойчивость) ====
%EXE% in.bmp out.bmp -crop 1000000000 1000000000
%EXE% in.bmp out.bmp -blur 10000
%EXE% in.bmp out.bmp -glass 1000
%EXE% in.bmp out.bmp -crystallize 1000000000
echo.

echo ==== 5. Странные цепочки фильтров ====
%EXE% in.bmp out.bmp -crop 1 1 -med 101
%EXE% in.bmp out.bmp -crop 2 2 -blur 1000
%EXE% in.bmp out.bmp -crop 1 1 -crop 0 0
echo.

echo ==== 6. Mosaic на разных размерах (без CIFAR) ====
%EXE% small32.bmp  mosaic32.bmp   -mosaic_cifar
%EXE% small64.bmp  mosaic64.bmp   -mosaic_cifar
%EXE% tiny.bmp     mosaictiny.bmp -mosaic_cifar
echo.

echo ==== 7. Плохие входные файлы (не BMP) ====
%EXE% notbmp.bmp out_notbmp.bmp -gs
echo.

echo ==== 8. Длинная цепочка фильтров ====
%EXE% in.bmp chain1.bmp ^
  -crop 2000 2000 -blur 5 -sharp -edge 0.2 -neg -gs -med 7 -crystallize 20 -glass 3
echo.

echo ==== 9. Повторные crop/med ====
%EXE% in.bmp chain2.bmp -crop 100 100 -crop 50 50 -crop 10 10 -crop 1 1
%EXE% in.bmp chain3.bmp -med 3 -med 5 -med 7 -med 9
echo.

echo ==== 10. Огромный window для median ====
%EXE% in.bmp med_big.bmp -med 999
echo.

echo ==== 11. Mosaic с цепочкой фильтров (если есть CIFAR) ====
if exist cifar-100-binary\train.bin (
    %EXE% small32.bmp  mosaic_chain32.bmp  -sharp -mosaic_cifar -gs
    %EXE% small64.bmp  mosaic_chain64.bmp  -crop 40 40 -mosaic_cifar -blur 2
) else (
    echo WARNING: cifar-100-binary/train.bin not found, пропускаю сложные mosaic-тесты.
)
echo.

echo ==== 12. Перепутанные input/output ====
%EXE% tiny.bmp out.bmp -gs
%EXE% out.bmp in.bmp -gs
echo.

echo ==== 13. Лишние аргументы после выходного файла ====
%EXE% tiny.bmp in.bmp foo bar baz
echo.

echo ==== 14. Очень узкие/маленькие картинки (если есть) ====
if exist wide.bmp   %EXE% wide.bmp   wide_blur.bmp    -blur 5
if exist tall.bmp   %EXE% tall.bmp   tall_med.bmp     -med 5
if exist oddpad.bmp %EXE% oddpad.bmp oddpad_sharp.bmp -sharp
echo.

echo ==== 15. Фильтры на tiny.bmp ====
%EXE% tiny.bmp tiny_edge.bmp  -edge 0.5
%EXE% tiny.bmp tiny_med.bmp   -med 9
%EXE% tiny.bmp tiny_blur.bmp  -blur 10
%EXE% tiny.bmp tiny_combo.bmp -sharp -med 9 -blur 10 -edge 0.3
echo.

echo ==== 16. Многократные realloc-сценарии ====
%EXE% in.bmp chain_heavy.bmp ^
  -crop 1024 1024 -crop 512 512 -crop 256 256 -crop 128 128 ^
  -crop 64 64 -crop 32 32 -crop 16 16 -crop 8 8 -crop 4 4 -crop 2 2 -crop 1 1 ^
  -med 11 -blur 5 -crystallize 4 -glass 2
echo.

echo ==== 17. Большое изображение по памяти (опционально) ====
if exist big.bmp (
    %EXE% big.bmp big_blur.bmp -blur 3
) else (
    echo WARNING: big.bmp not found, memory-тест пропущен.
)
echo.

echo ==== 18. Повреждённые заголовки BMP (опционально) ====
if exist corrupt_size.bmp (
    %EXE% corrupt_size.bmp out_corrupt_size.bmp -gs
)
if exist corrupt_data.bmp (
    %EXE% corrupt_data.bmp out_corrupt_data.bmp -gs
)
echo.

echo ==== Все тесты выполнены ====
pause
