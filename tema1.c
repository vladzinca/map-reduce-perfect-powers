#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>

#define NUM_THREADS 32
#define FILE_NAME_LENGTH 32
#define ARRAY_ELEMENTS_NUMBER 1024

typedef struct Package
{
    pthread_barrier_t* barrier;
    long id;
    int M, R, P, fileCounter;
    int **v;
    int *length;
    int** powerMatrix;
    int* lungimiVec;
    FILE* pFile;
    FILE *qFile;
} Package;

typedef struct ReducerPackage
{
    pthread_barrier_t* barrier;
    long id;
    int M, R, P;
    int lungime;
    int *sol;
    Package **package;
    FILE *rFile;
} ReducerPackage;

int calculeazaFaraPowSubunitar(int i)
{
    int a = 1;
    while (pow(a, i) < INT_MAX)
        a++;
    return a - 1;
}

int verifyNthPowerRecursively(int arr[], int l, int r, int x)
{
    if (r >= l) {
        int mid = l + (r - l) / 2;

        if (arr[mid] == x)
            return mid;

        if (arr[mid] > x)
            return verifyNthPowerRecursively(arr, l, mid - 1, x);

        return verifyNthPowerRecursively(arr, mid + 1, r, x);
    }

    return -1;
}

int verifyNthPower(int a, int n, int* lungimiVec, int** powerMatrix) // verifica ca exista x astfel incat x ^ n = a
{
    int left = 0;
    int right = lungimiVec[n - 2] - 1;
    if (verifyNthPowerRecursively(powerMatrix[n - 2], left, right, a) == -1)
        return 0;
    else
        return 1;
}

void *f(void *arg)
{
    Package *package = (Package *)arg;
    if (package->id < package->M)
    {
        char s[FILE_NAME_LENGTH];

        // lock
        while (fscanf(package->pFile, "%s", s) != EOF)
        {
            //unlock

            package->qFile = fopen(s, "r");
            int numberCount;
            fscanf(package->qFile, "%d", &numberCount);

            for (int i = 0; i < numberCount; i++)
            {
                int nr;
                fscanf(package->qFile, "%d", &nr);

                for (int j = 0; j < package->R; j++)
                {
                    if (nr > 0)
                        if (verifyNthPower(nr, j + 2, package->lungimiVec, package->powerMatrix))
                        {
                            (package->length[j])++;
                            package->v[j][package->length[j] - 1] = nr;
                        }
                }
            }
            //lock
        }
        //unlock
    }

    pthread_barrier_wait(package->barrier);

    ReducerPackage *reddy = (ReducerPackage *)arg;

    if (reddy->id >= reddy->M && reddy->id < (reddy->M + reddy->R))
    {
        for (int i = 0; i < reddy->M; i++)
        {
            for (int j = 0; j < reddy->package[i]->length[reddy->id - reddy->M]; j++)
            {
                int aux = reddy->package[i]->v[reddy->id - reddy->M][j];
                int existaDeja = 0;

                if (reddy->lungime != 0) {
                    for (int k = 0; k < reddy->lungime; k++)
                        if (reddy->sol[k] == aux)
                            existaDeja = 1;
                }
                if (existaDeja == 0)
                {
                    reddy->lungime++;
                    reddy->sol[reddy->lungime - 1] = aux;
                }
            }
        }
        char fisier[FILE_NAME_LENGTH];
        sprintf(fisier, "out%ld.txt", reddy->id - reddy->M + 2);
        reddy->rFile = fopen(fisier, "w");
        fprintf(reddy->rFile, "%d", reddy->lungime);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    FILE *pFile;
    int M, R, P, fileCounter;
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    P = M + R;

    int** powerMatrix;
    int* lungimiVec;

    lungimiVec = malloc(NUM_THREADS * sizeof(int));
    for (int i = 0; i < NUM_THREADS; i++)
        lungimiVec[i] = calculeazaFaraPowSubunitar(i + 2);

    powerMatrix = malloc(NUM_THREADS * sizeof(int*));
    for (int i = 0; i < NUM_THREADS; i++) // vectori pentru puteri perfecte de 2, 3, ..., 33 (0 - 31)
        powerMatrix[i] = malloc(lungimiVec[i] * sizeof(int));

    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < lungimiVec[i]; j++) {
            powerMatrix[i][j] = pow(j + 1, i + 2);
        }
    }

    pFile = fopen(argv[3], "r");
    fscanf(pFile, "%d", &fileCounter);

    pthread_barrier_t barrier;

    pthread_barrier_init(&barrier, NULL, P);

    Package **package = malloc(NUM_THREADS * sizeof(Package));
    for (int i = 0; i < M; i++)
    {
        package[i] = malloc(sizeof(Package));
        package[i]->M = M;
        package[i]->R = R;
        package[i]->P = P;
        package[i]->fileCounter = fileCounter;
        package[i]->qFile = malloc(sizeof(FILE *));
        package[i]->id = i;
        package[i]->v = malloc(NUM_THREADS * sizeof(int*));
        for (int j = 0; j < R; j++)
        {
            package[i]->v[j] = malloc(ARRAY_ELEMENTS_NUMBER * sizeof(int));
        }
        package[i]->length = malloc(NUM_THREADS * sizeof(int));
        for (int j = 0; j < R; j++)
            package[i]->length[j] = 0;
        package[i]->pFile = pFile;
        package[i]->barrier = &barrier;
        package[i]->powerMatrix = powerMatrix;
        package[i]->lungimiVec = lungimiVec;

    }

    ReducerPackage **reducerPackage = malloc(2 * NUM_THREADS * sizeof(ReducerPackage));
    for (int i = M; i < R + M; i++)
    {
        reducerPackage[i] = malloc(sizeof(ReducerPackage));
        reducerPackage[i]->package = malloc(NUM_THREADS * sizeof(Package));
        for (int j = 0; j < M; j++)
        {
            reducerPackage[i]->package[j] = malloc(sizeof(Package));
            reducerPackage[i]->package[j] = package[j];
        }
        reducerPackage[i]->id = i;
        reducerPackage[i]->M = M;
        reducerPackage[i]->R = R;
        reducerPackage[i]->P = P;
        reducerPackage[i]->sol = malloc(ARRAY_ELEMENTS_NUMBER * sizeof(int));
        reducerPackage[i]->lungime = 0;
        reducerPackage[i]->rFile = malloc(sizeof(FILE *));
        reducerPackage[i]->barrier = &barrier;
    }

    pthread_t threads[P];
    int r;
    long id;
    void *status;

    for (id = 0; id < P; id++)
    {
        if (id < M)
            r = pthread_create(&threads[id], NULL, f, package[id]);
        else
            r = pthread_create(&threads[id], NULL, f, reducerPackage[id]);

        if (r)
        {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
    }

    for (id = 0; id < P; id++)
    {
        r = pthread_join(threads[id], &status);

        if (r)
        {
            printf("Eroare la asteptarea thread-ului %ld\n", id);
            exit(-1);
        }
    }

    pthread_barrier_destroy(&barrier);

    // for (int i = 0; i < M; i++)
    // {
    //     printf("Thread %d:\n", i);
    //     for (int j = 0; j < package[i]->R; j++)
    //     {
    //         printf("%d (length %d): ", j + 2, package[i]->length[j]);
    //         for (int k = 0; k < package[i]->length[j]; k++)
    //         {
    //             printf("%d ", package[i]->v[j][k]);
    //         }
    //         printf("\n");
    //     }
    //     printf("\n");
    // }
    // for (int i = M; i < P; i++)
    // {
    //     printf("Thread %d (puteri de %d):\n", i, i - M + 2);
    //     printf("(lungime %d): ", reducerPackage[i]->lungime);
    //     for (int j = 0; j < reducerPackage[i]->lungime; j++) {
    //         printf("%d ", reducerPackage[i]->sol[j]);
    //     }
    //     printf("\n\n");
    // }

    pthread_exit(NULL);

    for (int i = 0; i < NUM_THREADS; i++)
        free(powerMatrix[i]);
    free(powerMatrix);
    free(lungimiVec);
    for (int i = M; i < R + M; i++) {
        for (int j = 0; j < M; j++)
            free(reducerPackage[i]->package[j]);
        free(reducerPackage[i]->package);
        free(reducerPackage[i]->sol);
        free(reducerPackage[i]->rFile);
        free(reducerPackage[i]);
    }
    free(reducerPackage);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < R; j++)
            free(package[i]->v[j]);
        free(package[i]->v);
        free(package[i]->qFile);
        free(package[i]->length);
        free(package[i]);
    }
    free(package);

    return 0;
}
