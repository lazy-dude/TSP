clear

# -Werror is because of only warnings do execute tsp.out
gcc -g -Wall -Wextra -pedantic -Werror -std=c11 tsp.c `simple2d --libs` -lm -o tsp.out

if [ $? -eq 0 ]; then ./tsp.out ; fi
# ./tsp.out
