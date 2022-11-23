#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <limits.h>
#include <pthread.h>

#define NUM_THREADS 32
#define FILE_NAME_LENGTH 32
#define ARRAY_ELEMENTS_NUMBER 1024

/* argumentul functiilor mapper */
typedef struct MapperPackage
{
    pthread_barrier_t *barrier;
    long id;
    int M, R, P, fileCounter;
    int **v;
    int *length;
    int **powerMatrix;
    int *powerMatrixLengths;
    FILE *pFile;
    FILE *qFile;
} MapperPackage;

/* argumentul functiile reducer */
typedef struct ReducerPackage
{
    pthread_barrier_t *barrier;
    long id;
    int M, R, P;
    int lungime;
    int *sol;
    MapperPackage **mapperPackage;
    FILE *rFile;
} ReducerPackage;

/* incepe de la 1 si ridica la puterea i pana cand atinge INT_MAX */
int computePowerMatrixLimit(int i)
{
    int a = 1;
    while (pow(a, i) < INT_MAX)
        a++;
    return a - 1;
}

/* face binary search pe vectorul de puteri perfecte pentru a verifica daca x
este putere perfecta */
int verifyNthPowerRecursively(int v[], int st, int dr, int x)
{
    if (dr >= st)
    {
        int mid = st + (dr - st) / 2;

        if (v[mid] == x)
            return mid;

        if (v[mid] > x)
            return verifyNthPowerRecursively(v, st, mid - 1, x);

        return verifyNthPowerRecursively(v, mid + 1, dr, x);
    }

    return -1;
}

/* verifica ca a este n-putere perfecta apeland verifyNthPowerRecursively */
int verifyNthPower(int a, int n, int *powerMatrixLengths, int **powerMatrix)
{
    int st = 0;
    int dr = powerMatrixLengths[n - 2] - 1;
    if (verifyNthPowerRecursively(powerMatrix[n - 2], st, dr, a) == -1)
        return 0;
    else
        return 1;
}

/* functia mapper/reducer */
void *f(void *arg)
{
    MapperPackage *mapperPackage = (MapperPackage *)arg;
    /* daca thread-ul este mapper. Altfel, se duce la bariera si asteapta
    executarea tuturor thread-urilor mapper */
    if (mapperPackage->id < mapperPackage->M)
    {
        char s[FILE_NAME_LENGTH];

        /* citeste urmatorul fisier care e liber */
        while (fscanf(mapperPackage->pFile, "%s", s) != EOF)
        {
            mapperPackage->qFile = fopen(s, "r");
            int numberCount;
            /* citeste numarul de numere din fisier */
            fscanf(mapperPackage->qFile, "%d", &numberCount);

            for (int i = 0; i < numberCount; i++)
            {
                int nr;
                /* citeste un numar din fisier */
                fscanf(mapperPackage->qFile, "%d", &nr);

                /* pentru toate j-urile, verifica ca nr sa fie
                (j+2)-putere perfecta*/
                for (int j = 0; j < mapperPackage->R; j++)
                {
                    if (nr > 0)
                        if (verifyNthPower(nr, j + 2, mapperPackage->powerMatrixLengths, mapperPackage->powerMatrix))
                        {
                            /* daca este putere perfecta, il pune in vector */
                            (mapperPackage->length[j])++;
                            mapperPackage->v[j][mapperPackage->length[j] - 1] = nr;
                        }
                }
            }

            fclose(mapperPackage->qFile);
        }
    }

    /* dupa ce termina toate thread-urile mapper executia, thread-urile
    reducer le vor astepta aici */
    pthread_barrier_wait(mapperPackage->barrier);

    ReducerPackage *reducerPackage = (ReducerPackage *)arg;

    /* toate thread-urile trec de bariera. Daca thread-ul este mapper, nu
    intra in acest if si se duce la pthread_exit */
    if (reducerPackage->id >= reducerPackage->M && reducerPackage->id < (reducerPackage->M + reducerPackage->R))
    {
        /* ia rezultatele de la fiecare thread M */
        for (int i = 0; i < reducerPackage->M; i++)
        {
            /* ia rezultatele pentru numerele putere perfecta pe care le
            calculeaza el */
            for (int j = 0; j < reducerPackage->mapperPackage[i]->length[reducerPackage->id - reducerPackage->M]; j++)
            {
                int aux = reducerPackage->mapperPackage[i]->v[reducerPackage->id - reducerPackage->M][j];
                int existaDeja = 0;

                /* verifica daca numarul verificat e duplicat */
                if (reducerPackage->lungime != 0)
                {
                    for (int k = 0; k < reducerPackage->lungime; k++)
                        if (reducerPackage->sol[k] == aux)
                            existaDeja = 1;
                }
                /* daca nu e, il adauga in vectorul solutie */
                if (existaDeja == 0)
                {
                    reducerPackage->lungime++;
                    reducerPackage->sol[reducerPackage->lungime - 1] = aux;
                }
            }
        }
        /* la final scrie in fisierul corect lungimea vectorului solutie */
        char fisier[FILE_NAME_LENGTH];
        sprintf(fisier, "out%ld.txt", reducerPackage->id - reducerPackage->M + 2);
        reducerPackage->rFile = fopen(fisier, "w");
        fprintf(reducerPackage->rFile, "%d", reducerPackage->lungime);

        fclose(reducerPackage->rFile);
    }

    /* thread-urile mapper trec peste if-ul de mai sus si ajung aici.
    Thread-urile reducer ajung aici dupa scrierea in fisier a rezultatului
    corect */
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    /* pointer-ul cu care se citesc fisiere */
    FILE *pFile;
    /* numarul de thread-uri mapper, reducer si numarul de fisiere de citit */
    int M, R, P, fileCounter;
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    P = M + R;

    /* calculeaza o matrice cu toate puterile perfecte < INT_MAX de la 2 la 33.
    Ma folosesc de faptul ca pot fi maxim 32 de thread-uri */
    int **powerMatrix;
    int *powerMatrixLengths;

    powerMatrixLengths = malloc(NUM_THREADS * sizeof(int));
    for (int i = 0; i < NUM_THREADS; i++)
        powerMatrixLengths[i] = computePowerMatrixLimit(i + 2);

    powerMatrix = malloc(NUM_THREADS * sizeof(int *));
    for (int i = 0; i < NUM_THREADS; i++)
        /* vectorii puteri perfecte de 2, 3, ..., 33 (i de la 0 - 31) */
        powerMatrix[i] = malloc(powerMatrixLengths[i] * sizeof(int));

    for (int i = 0; i < NUM_THREADS; i++)
    {
        for (int j = 0; j < powerMatrixLengths[i]; j++)
        {
            powerMatrix[i][j] = pow(j + 1, i + 2);
        }
    }

    /* deschide fisierul initial si citeste cate fisiere are de citit */
    pFile = fopen(argv[3], "r");
    fscanf(pFile, "%d", &fileCounter);

    pthread_barrier_t barrier;

    pthread_barrier_init(&barrier, NULL, P);

    /* initializeaza argumentele thread-urilor. Fiecare mapper primeste un
    mapperPackage, iar fiecare reducer primeste un reducerPackage. De mentionat
    ca fiecare reducer primeste in pachetul sau si toate mapperPackage-urile */
    MapperPackage **mapperPackage = malloc(NUM_THREADS * sizeof(MapperPackage));
    for (int i = 0; i < M; i++)
    {
        mapperPackage[i] = malloc(sizeof(MapperPackage));
        mapperPackage[i]->M = M;
        mapperPackage[i]->R = R;
        mapperPackage[i]->P = P;
        mapperPackage[i]->fileCounter = fileCounter;
        mapperPackage[i]->qFile = malloc(sizeof(FILE *));
        mapperPackage[i]->id = i;
        mapperPackage[i]->v = malloc(NUM_THREADS * sizeof(int *));
        for (int j = 0; j < R; j++)
        {
            mapperPackage[i]->v[j] = malloc(ARRAY_ELEMENTS_NUMBER * sizeof(int));
        }
        mapperPackage[i]->length = malloc(NUM_THREADS * sizeof(int));
        for (int j = 0; j < R; j++)
            mapperPackage[i]->length[j] = 0;
        mapperPackage[i]->pFile = pFile;
        mapperPackage[i]->barrier = &barrier;
        mapperPackage[i]->powerMatrix = powerMatrix;
        mapperPackage[i]->powerMatrixLengths = powerMatrixLengths;
    }

    ReducerPackage **reducerPackage = malloc(2 * NUM_THREADS * sizeof(ReducerPackage));
    for (int i = M; i < R + M; i++)
    {
        reducerPackage[i] = malloc(sizeof(ReducerPackage));
        reducerPackage[i]->mapperPackage = malloc(NUM_THREADS * sizeof(MapperPackage));
        for (int j = 0; j < M; j++)
        {
            reducerPackage[i]->mapperPackage[j] = malloc(sizeof(MapperPackage));
            reducerPackage[i]->mapperPackage[j] = mapperPackage[j];
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

    /* deschide thread-urile */
    for (id = 0; id < P; id++)
    {
        /* thread-urile mapper si reducer se deschid la acelasi moment de
        timp */
        if (id < M)
            r = pthread_create(&threads[id], NULL, f, mapperPackage[id]);
        else
            r = pthread_create(&threads[id], NULL, f, reducerPackage[id]);

        if (r)
        {
            printf("Eroare la crearea thread-ului %ld\n", id);
            exit(-1);
        }
    }

    /* dau join thread-urile */
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

    pthread_exit(NULL);

    /* se elibereaza memoria */
    fclose(pFile);

    for (int i = 0; i < NUM_THREADS; i++)
        free(powerMatrix[i]);
    free(powerMatrix);
    free(powerMatrixLengths);
    for (int i = M; i < R + M; i++)
    {
        for (int j = 0; j < M; j++)
            free(reducerPackage[i]->mapperPackage[j]);
        free(reducerPackage[i]->mapperPackage);
        free(reducerPackage[i]->sol);
        free(reducerPackage[i]->rFile);
        free(reducerPackage[i]);
    }
    free(reducerPackage);
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < R; j++)
            free(mapperPackage[i]->v[j]);
        free(mapperPackage[i]->v);
        free(mapperPackage[i]->qFile);
        free(mapperPackage[i]->length);
        free(mapperPackage[i]);
    }
    free(mapperPackage);

    return 0;
}
