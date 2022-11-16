#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

FILE* pFile, *qFile; // declare them locally

typedef struct Package {
    int M, R, P, fileCounter;
    int* length;
    long id;
} Package;

int verifyNthPower(int a, int n)
{
    int i = 1;
    while (a > pow(i, n))
        i++;
    if (a == pow(i, n))
        return 1;
    return 0;
}

void *fMapper(void *arg) {
  	Package* package = (Package*)arg;

    printf("%ld inside fMapper\n", package->id);

  	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int M, R, P, fileCounter;
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    P = M + R;

    pFile = fopen(argv[3], "r");
    fscanf(pFile, "%d", &fileCounter);
    printf("%d %d %d\n", M, R, fileCounter);

    Package** package = malloc(M * sizeof(Package));
    for (int i = 0; i < M; i++) {
        package[i] = malloc(sizeof(Package));
        package[i]->M = M;
        package[i]->R = R;
        package[i]->P = P;
        package[i]->fileCounter = fileCounter;
        package[i]->id = i;
        package[i]->length = malloc(R * sizeof(int));
        for (int j = 0; j < R; j++)
            package[i]->length[j] = 0;
    }

	pthread_t threads[P];
  	int r;
  	long id;
  	void *status;

    for (id = 0; id < M; id++)
    {
        printf("%ld inside main\n", package[id]->id);
        r = pthread_create(&threads[id], NULL, fMapper, package[id]);

		if (r) {
	  		printf("Eroare la crearea thread-ului %ld\n", id);
	  		exit(-1);
		}
    }

  	for (id = 0; id < M; id++)
    {
        r = pthread_join(threads[id], &status);

		if (r) {
	  		printf("Eroare la asteptarea thread-ului %ld\n", id);
	  		exit(-1);
		}
  	}

  	pthread_exit(NULL);
}
