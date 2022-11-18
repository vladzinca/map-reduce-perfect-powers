#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

FILE* pFile; // declare them locally

pthread_mutex_t mutex;

typedef struct Package {
    int M, R, P, fileCounter;
    int** v;
    int* length;
    int totalLength;
    // int numberCount;
    // int nr;
    long id;
    FILE* qFile;
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

    // printf("%ld inside fMapper\n", package->id);

    char s[20];
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


    pthread_mutex_init(&mutex, NULL);

    Package** package = malloc(M * sizeof(Package));
    for (int i = 0; i < M; i++) {
        package[i] = malloc(sizeof(Package));
        package[i]->M = M;
        package[i]->R = R;
        package[i]->P = P;
        package[i]->fileCounter = fileCounter;
        package[i]->qFile = malloc(sizeof(FILE*));
        package[i]->id = i;
        package[i]->totalLength = R;
        package[i]->v = malloc(1024 * R * sizeof(int)); // 1024 hardcodat
        for (int j = 0; j < R; j++)
        {
            package[i]->v[j] = malloc(sizeof(int));
        }
        package[i]->length = malloc(R * sizeof(int));
        for (int j = 0; j < R; j++)
            package[i]->length[j] = 0;
    }

    // package[0]->v[2] = malloc(10 * sizeof(int));
    // package[0]->v[2][3] = 10;

	pthread_t threads[P];
  	int r;
  	long id;
  	void *status;

    for (id = 0; id < M; id++)
    {
        // printf("%ld inside main\n", package[id]->id);
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

    pthread_mutex_destroy(&mutex);

    for (int i = 0; i < M; i++)
    {
        printf("Thread %d:\n", i);
        for (int j = 0; j < package[i]->R; j++) {
            printf("%d: ", j + 2);
            for (int k = 0; k < package[i]->length[j]; k++) {
                printf("%d ", package[i]->v[j][k]);
            }
            printf("\n");
            }
        printf("\n");
    }

  	pthread_exit(NULL);
}
