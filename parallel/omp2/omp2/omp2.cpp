﻿#include <iostream>
using namespace std;
#include <omp.h>
void main()
{
    omp_set_num_threads(4);
#pragma omp parallel
    {
        printf("Hello World!\n");
    }
}