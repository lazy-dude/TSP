#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define X_MAX 800.0
#define Y_MAX 600.0
#define CITY_NUM 200

int main(void)
{
    FILE * coor;
    char file_name[60]={'\0'};
    sprintf(file_name, "tsp_coord_float_%d.text", CITY_NUM);
    coor=fopen(file_name,"w");
    
    srand(time(NULL));
    float xrand,yrand;
 
    for (int n=0; n < CITY_NUM; n++) {
        xrand = (rand()/(double)RAND_MAX)* X_MAX;
        yrand = (rand()/(double)RAND_MAX)* Y_MAX;
        
        xrand += 50.0; // centering
        yrand += 50.0;
        
        fprintf(coor, "%.2lf,%.2lf\n",xrand,yrand);
    }
    
    fclose(coor);
    return 0;
}
