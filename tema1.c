#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

int count;
FILE* pFile;
pthread_mutex_t* mutex;

void *fMapper(void *arg) {
  	long id = *(long*)arg;

    char s[20];
    for (int i = 0; i < count; i++)
    {
        if (i == id)
        {
            pthread_mutex_lock(&mutex[id]);
            fscanf(pFile, "%s", s);
            printf("%s from thread %ld\n", s, id);
            pthread_mutex_unlock(&mutex[id]);
        }
    }

  	pthread_exit(NULL);
}

void *fReducer(void *arg) {
  	long id = *(long*)arg;

  	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int M, R, P;
    pFile = fopen(argv[3], "r");
    M = atoi(argv[1]);
    R = atoi(argv[2]);
    P = M + R;
    fscanf(pFile, "%d", &count);
    printf("%d %d %d\n", M, R, count);

	pthread_t threads[P];
  	int r;
  	long id;
  	void *status;
	long ids[P];

    mutex = malloc(count * sizeof(pthread_mutex_t));

    for (int i = 0; i < count; i++)
        pthread_mutex_init(&mutex[i], NULL);

    for (id = 0; id < P; id++)
    {
        ids[id] = id;
        if (id < M)
            r = pthread_create(&threads[id], NULL, fMapper, &ids[id]);
        else
            r = pthread_create(&threads[id], NULL, fReducer, &ids[id]);

		if (r) {
	  		printf("Eroare la crearea thread-ului %ld\n", id);
	  		exit(-1);
		}
    }

  	for (id = 0; id < P; id++) {
		r = pthread_join(threads[id], &status);

		if (r) {
	  		printf("Eroare la asteptarea thread-ului %ld\n", id);
	  		exit(-1);
		}
  	}

    for (int i = 0; i < count; i++)
        pthread_mutex_destroy(&mutex[i]);

  	pthread_exit(NULL);

    fclose(pFile);
    return 0;
}
