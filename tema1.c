#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

FILE* pFile; // declare them locally

pthread_barrier_t barrier;

// verifica sa nu se ia 0

typedef struct Package {
    long id;
    int M, R, P, fileCounter;
    int** v;
    int* length;
    FILE* qFile;
} Package;

typedef struct reducerPackage {
    long id;
    int M, R, P;
    // int* v;
    // int length;
    Package** package;
} reducerPackage;

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
    if (package->id < package->M) {
        char s[20]; // 20 hardcodat
        while (fscanf(pFile, "%s", s) != EOF)
        {
            printf("%s de catre thread-ul %ld\n", s, package->id);

            

            package->qFile = fopen(s, "r");
            int numberCount;
            fscanf(package->qFile, "%d", &numberCount);

            // printf("thread %ld: numberCount = %d\n", package->id, numberCount);

            

            for (int i = 0; i < numberCount; i++)
            {
                int nr;
                fscanf(package->qFile, "%d", &nr);
                // printf("citeste %d din thread-ul %ld, fisierul %s\n", package->nr, package->id, s);

                for (int j = 0; j < package->R; j++)
                {
                    if (nr > 0)
                        if (verifyNthPower(nr, j + 2))
                        {
                            // pthread_mutex_lock(&mutex);
                            
                            // printf("%d, putere perfecta de %d, thread %ld\n", package->nr, j + 2, package->id);
                            (package->length[j])++;
                            package->v[j] = realloc(package->v[j], package->length[j] * sizeof(int));
                            package->v[j][package->length[j] - 1] = nr;
                            
                            // pthread_mutex_unlock(&mutex);
                        }
                }
            }
            // printf("thread %ld\n", package->id);
            // for (int j = 0; j < package->R; j++) {
            //     printf("%d: ", j + 2);
            //     for (int k = 0; k < package->length[j]; k++) {
            //         printf("%d ", package->v[j][k]);
            //     }
            //     printf("\n");
            // }
            // printf("\n");
            
            
        }
    }

    pthread_barrier_wait(&barrier);

    reducerPackage* reddy = (reducerPackage*)arg;
    if (reddy->id >= reddy->M && reddy->id < (reddy->M + reddy->R)) {

        // printf("%ld inside fMapper\n", reddy->id - reddy->M);
        // printf("%d inside thread %ld\n", reddy->package[0]->v[reddy->id - reddy->M][0], reddy->id - reddy->M + 2);

        int v[214748]; // hardcodat // fa o functie de search care adauga elemente in vector
        for (int i = 0; i < 214748; i++)
            v[i] = 0;
        int numaraNumere = 0;
        for (int i = 0; i < reddy->M; i++)
        {
            for (int j = 0; j < reddy->package[i]->length[reddy->id - reddy->M]; j++)
            {
                int aux = reddy->package[i]->v[reddy->id - reddy->M][j];
                if (aux < 214748)
                {
                    if (v[aux] == 0)
                        numaraNumere++;
                    v[aux] = 1;

                }
                else
                    printf("vezi fii atent");
                // printf("%d inside thread %ld\n", reddy->package[i]->v[reddy->id - reddy->M][j], reddy->id - reddy->M);
            }
        }
        printf("%d from thread %ld\n", numaraNumere, reddy->id - reddy->M + 2);
    }

  	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int M, R, P, fileCounter;
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    P = M + R;

    pFile = fopen(argv[3], "r");
    fscanf(pFile, "%d", &fileCounter);
    // printf("%d %d %d\n", M, R, fileCounter);

    pthread_barrier_init(&barrier, NULL, P);

    Package** package = malloc(sizeof(Package*)); // M * sizeof(Package)
    for (int i = 0; i < M; i++) {
        package[i] = malloc(sizeof(Package));
        package[i]->M = M;
        package[i]->R = R;
        package[i]->P = P;
        package[i]->fileCounter = fileCounter;
        package[i]->qFile = malloc(sizeof(FILE*));
        package[i]->id = i;
        package[i]->v = malloc(1024 * R * sizeof(int)); // 1024 hardcodat
        for (int j = 0; j < R; j++)
        {
            package[i]->v[j] = malloc(sizeof(int));
        }
        package[i]->length = malloc(R * sizeof(int));
        for (int j = 0; j < R; j++)
            package[i]->length[j] = 0;
    }

    reducerPackage** reducerPackage = malloc(32 * sizeof(reducerPackage)); // fa reducer package sa mearga doar cu id
    for (int i = M; i < R + M; i++) {
        reducerPackage[i] = malloc(sizeof(reducerPackage));
        reducerPackage[i]->package = malloc(sizeof(Package*));
        for (int j = 0; j < M; j++)
        {
            reducerPackage[i]->package[j] = malloc(1024 * sizeof(Package));
            reducerPackage[i]->package[j] = package[j];
        }
        printf("%d\n", i);
        reducerPackage[i]->id = i;
        reducerPackage[i]->M = M;
        reducerPackage[i]->R = R;
        reducerPackage[i]->P = P;
        // reducerPackage[i]->v = malloc(R * sizeof(int));
        // for (int j = 0; j < R; j++)
        //     reducerPackage[i]->v[j] = 0;
        // reducerPackage[i]->length = 0;
        // printf("%ld\n", reducerPackage[i]->id);
    }
    // reducerPackage[0]->id = 0; // linia asta da seg fault
    // for (int i = 0; i < R; i++) {
        // reducerPackage[i]->package = package;
        // reducerPackage[i]->id = 0;
    // }

    // package[0]->v[2] = malloc(10 * sizeof(int));
    // package[0]->v[2][3] = 10;

	pthread_t threads[P];
  	int r;
  	long id;
  	void *status;

    for (id = 0; id < P; id++)
    {
        // printf("%ld inside main\n", package[id]->id);
        if (id < M)
            r = pthread_create(&threads[id], NULL, fMapper, package[id]);
        else
        {
            // printf("id = %ld\n", id);
            r = pthread_create(&threads[id], NULL, fMapper, reducerPackage[id]);
        }

		if (r) {
	  		printf("Eroare la crearea thread-ului %ld\n", id);
	  		exit(-1);
		}
    }

  	for (id = 0; id < P; id++) // id < P
    {
        r = pthread_join(threads[id], &status);

		if (r) {
	  		printf("Eroare la asteptarea thread-ului %ld\n", id);
	  		exit(-1);
		}
  	}

    pthread_barrier_destroy(&barrier);

    for (int i = 0; i < M; i++)
    {
        printf("Thread %d:\n", i);
        for (int j = 0; j < package[i]->R; j++) {
            printf("%d (length %d): ", j + 2, package[i]->length[j]);
            for (int k = 0; k < package[i]->length[j]; k++) {
                printf("%d ", package[i]->v[j][k]);
            }
            printf("\n");
            }
        printf("\n");
    }

  	pthread_exit(NULL);
}
