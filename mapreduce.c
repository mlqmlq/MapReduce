#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "mapreduce.h"

struct pairs {
	char *key;
	char *value;
};

struct files {
	char *name;
};

struct pairs **partitions;
struct files *fileNames;
int* pairCountInPartition;
int* pairAllocatedInPartition;
int* numberOfAccessInPartition;
pthread_mutex_t lock, fileLock;
Partitioner p;
Reducer r;
Mapper m;
int numberPartitions;
int filesProcessed;
int totalFiles;

void* mapperHelper(void *arg) {
	while(filesProcessed < totalFiles) {
		pthread_mutex_lock(&fileLock);
		char *filename = NULL;
		if(filesProcessed < totalFiles) {
			filename = fileNames[filesProcessed].name;
			filesProcessed++;
		}
		pthread_mutex_unlock(&fileLock);
		if(filename != NULL)
			m(filename);
	}
	return arg;
}

// Sort files by increasing size
int compareFiles(const void* p1, const void* p2) {
	struct files *f1 = (struct files*) p1;
	struct files *f2 = (struct files*) p2;
	struct stat st1, st2;
	stat(f1->name, &st1);
	stat(f2->name, &st2);
	long int size1 = st1.st_size;
	long int size2 = st2.st_size;
	return (size1 - size2);
}

// Sort the buckets by key and then by value in ascending order
int compare(const void* p1, const void* p2) {
	struct pairs *pair1 = (struct pairs*) p1;
	struct pairs *pair2 = (struct pairs*) p2;
	if(strcmp(pair1->key, pair2->key) == 0) {
		return strcmp(pair1->value, pair2->value);
	}
	return strcmp(pair1->key, pair2->key);
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers, Reducer reduce, int num_reducers, Combiner combine, Partitioner partition) {
    totalFiles = argc - 1;
    if(totalFiles < num_mappers) {
		num_mappers = totalFiles;
	}
    pthread_t mapperThreads[num_mappers];
	pthread_t reducerThreads[num_reducers];
	pthread_mutex_init(&lock, NULL);
	pthread_mutex_init(&fileLock, NULL);
	p = partition;
	m = map;
	r = reduce;
    numberPartitions = num_reducers;
    partitions = malloc(num_reducers * sizeof(struct pairs*));
    fileNames = malloc(totalFiles * sizeof(struct files));
    pairCountInPartition = malloc(num_reducers * sizeof(int));
	pairAllocatedInPartition = malloc(num_reducers * sizeof(int));
	numberOfAccessInPartition = malloc(num_reducers * sizeof(int));
    filesProcessed = 0;
    int arrayPosition[num_reducers];
    // Initialising the arrays needed to store the key value pairs in the partitions
	for(int i = 0; i < num_reducers; i++) {
		partitions[i] = malloc(1024 * sizeof(struct pairs));
		pairCountInPartition[i] = 0;
		pairAllocatedInPartition[i] = 1024;
		arrayPosition[i] = i;
		numberOfAccessInPartition[i] = 0;
	}
    // Copying files for sorting in struct
	for(int i = 0; i <argc-1; i++) {
		fileNames[i].name = malloc((strlen(argv[i+1])+1) * sizeof(char));
		strcpy(fileNames[i].name, argv[i+1]);
	}
    // Sorting files as Shortest File first
	qsort(&fileNames[0], argc-1, sizeof(struct files), compareFiles);
    // Creating the threads for the number of mappers
	for (int i = 0; i < num_mappers; i++) {
		pthread_create(&mapperThreads[i], NULL, mapperHelper, NULL);
	}
    // Waiting for threads to finish
	for(int i = 0; i < num_mappers; i++) {
		pthread_join(mapperThreads[i], NULL); 
	}

}