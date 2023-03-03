#!/bin/bash

clear

# -Werror is because of only warnings do execute tsp.out
clang -DEXAMPLE_50 -g -Wall -Wextra -pedantic -Werror -std=c11 tsp.c -lSDL2_gfx -lSDL2 -lSDL2_ttf -lm -o tsp.out

if [ $? -eq 0 ]; then ./tsp.out ; fi
# ./tsp.out

# clang-tidy tsp.c
