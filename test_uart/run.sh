#Script para limpiar, recompilar, y ejecutar openocd y putty, y cerrar después.

#!/bin/bash

make clean-bsp
make clean
make run &
make term

read yn

killall xterm
killall putty
