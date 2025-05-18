#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NX 100   
#define NT 200   
#define A 1.0   
#define X_MAX 2.0 
#define T_MAX 1.0 

double phi(double x) {
    return exp(-x * x);  
}

double psi(double t) {
    return 1.0 / (1.0 + t * t);  
}

double f(double t, double x) {
    return 0.0;  
}

int main() {
    double h = X_MAX / (NX - 1); 
    double tau = T_MAX / (NT - 1); 
    double u[NT][NX];      


    for (int m = 0; m < NX; m++) {
        double x = m * h;
        u[0][m] = phi(x); 
    }
    for (int k = 0; k < NT; k++) {
        double t = k * tau;
        u[k][0] = psi(t); 
    }


    for (int k = 0; k < NT - 1; k++) {
        for (int m = 1; m < NX; m++) {
            u[k+1][m] = u[k][m] - A * (tau / h) * (u[k][m] - u[k][m-1]) + tau * f(k*tau, m*h);
        }
    }

    FILE *fp = fopen("result.txt", "w");
    for (int k = 0; k < NT; k++) {
        for (int m = 0; m < NX; m++) {
            fprintf(fp, "%.6f ", u[k][m]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

    printf("Result saved in result.txt\n");
    return 0;
}