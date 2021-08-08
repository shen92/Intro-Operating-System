#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "mapreduce.h"

typedef struct value {
    char *value;
    struct value *next;
} v;

typedef struct node {
    char *key;
    v *head;
    struct node *next;
    v *currValue;
    pthread_mutex_t nodeLock;
} n;

typedef struct bucket {
    n **head;
    struct bucket *next;
    int num_keys;
    pthread_mutex_t partitionLock;
} b;

int NUM_KEY = 2048;
b **table = NULL;
int NUM_FILES;
char **files;
Mapper map_function;
Reducer reduce_function;
int NUM_PARTITIONS;
pthread_mutex_t filelock = PTHREAD_MUTEX_INITIALIZER; // used when assign files to mappers
pthread_mutex_t partitionlock = PTHREAD_MUTEX_INITIALIZER; // used when reduce each partition
Partitioner partition_function; //partition function
int cur_file;
int cur_partition;

void freeTable() {
    pthread_mutex_destroy(&filelock);
    b *bptr;
    int i = 0;
    while (i < NUM_PARTITIONS) {
        bptr = table[i];
        pthread_mutex_destroy(&(bptr->partitionLock));
        int j = 0;
        while (j < NUM_KEY) {
            n *nptr1 = bptr->head[j];
            n *nptr2;
            while (nptr1 != NULL) {
                pthread_mutex_destroy(&(nptr1->nodeLock));
                v *vptr1;
                v *vptr2;
                vptr1 = nptr1->head;
                while (vptr1 != NULL) {
                    free(vptr1->value);
                    vptr2 = vptr1->next;
                    free(vptr1);
                    vptr1 = vptr2;
                }

                free(nptr1->key);
                nptr2 = nptr1->next;
                free(nptr1);
                nptr1 = nptr2;
            }
            j++;
        }
        free(bptr);
        i++;
    }
    free(table);
}

void init_hash_table() {

    table = malloc(NUM_PARTITIONS * sizeof(b *));
    int i;
    for (i = 0; i < NUM_PARTITIONS; i++) {
        table[i] = malloc(sizeof(b));
        if (table[i] == NULL) {
            printf("Unable to allocate memory.\n");
            exit(1);
        }
        pthread_mutex_init(&(table[i] -> partitionLock), NULL);

        table[i]->head = malloc(sizeof(n*) * NUM_KEY);
        if(table[i]->head == NULL) {
            printf("Unable to allocate memory.\n");
            exit(1);
        }
        table[i]->num_keys = 0;
    }
}


void MR_Emit(char *key, char *value) {
    unsigned long index = partition_function(key, NUM_PARTITIONS);
    unsigned long keyIndex = MR_DefaultHashPartition(key, NUM_KEY);

    b *ptr = table[index];
    pthread_mutex_lock(&(ptr->partitionLock));

    n *theNode = NULL;

    if (ptr->head[keyIndex] == NULL) {
        n *newNode = malloc(sizeof(n));
        if (newNode == NULL) {
            printf("Unable to allocate memory.\n");
            exit(1);
        }
        newNode->key = strdup(key);
        ptr->num_keys++;
        pthread_mutex_init(&(newNode->nodeLock), NULL);
        ptr->head[keyIndex] = newNode;
        theNode = newNode;
    } else {
        n *nptr = ptr->head[keyIndex];
        while (nptr != NULL) {
            if (strcmp(nptr->key, key) == 0) {
                theNode = nptr;
                break;
            }
            nptr = nptr->next;
        }
        if (theNode == NULL) {

            n *newNode = malloc(sizeof(n));
            if (newNode == NULL) {
                printf("Unable to allocate memory.\n");
                exit(1);
            }
            newNode->key = strdup(key);
            ptr->num_keys++;
            pthread_mutex_init(&(newNode->nodeLock), NULL);
            newNode->next = ptr->head[keyIndex];
            ptr->head[keyIndex] = newNode;

            theNode = newNode;
        }

    }
    pthread_mutex_unlock(&(ptr->partitionLock));

    v *newValue = malloc(sizeof(v));
    if (newValue == NULL) {
        printf("Unable to allocate memory.\n");
        exit(1);
    }
    newValue->value = strdup(value);

    pthread_mutex_lock(&(theNode->nodeLock));

    if (theNode->head == NULL) {
        theNode->head = newValue;
        theNode->currValue = newValue;
    }
    else {
        v *vptr = theNode->head;
        while (vptr->next != NULL)
            vptr = vptr->next;
        vptr->next = newValue;
    }
    pthread_mutex_unlock(&(theNode->nodeLock));
}


unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}


unsigned long MR_SortedPartition(char *key, int num_partitions) {
    char *ptr;
    unsigned long num;
    num = strtoul(key, &ptr, 10) & 0x0FFFFFFFF;
    unsigned int pos = 0 ;
    while( num_partitions>>=1 ) pos++;
    return num >> (32 - pos);
}


char *get_next(char *key, int partition_number) {
    b *bptr = table[partition_number];
    n *nptr = bptr->head[MR_DefaultHashPartition(key, NUM_KEY)];
    if (nptr == NULL)
        return NULL;
    while (strcmp(nptr->key, key) != 0) {
        nptr = nptr->next;
    }
    v *curVal = nptr->currValue;
    if(curVal == NULL) return NULL;
    nptr->currValue = curVal->next;
    return curVal->value;

}

void *Mapper_thread(void *arg) {
    for (;;) {
        pthread_mutex_lock(&filelock);
        int index;
        if ((cur_file + 1) > NUM_FILES) {
            pthread_mutex_unlock(&filelock);
            return NULL;
        } else {
            index = cur_file;
            cur_file++;
        }
        pthread_mutex_unlock(&filelock);
        map_function(files[index]);
    }
}

int compare_key(const void *a, const void *b){
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
}

void *Reducer_thread(void *arg) {
    for (;;) {
        pthread_mutex_lock(&partitionlock);
        int index;
        if (cur_partition >= NUM_PARTITIONS) {
            pthread_mutex_unlock(&partitionlock);
            return NULL;
        } else {
            index = cur_partition;
            cur_partition++;
        }
        pthread_mutex_unlock(&partitionlock);

        b *bptr = table[index];
        if(table[index] == NULL) return NULL;

        char **sorted = malloc(sizeof(char*) * bptr->num_keys);

        int i;
        int j = 0;
        for (i = 0; i < NUM_KEY; i++) {
            n *nptr = bptr->head[i];
            while (nptr != NULL) {
                sorted[j] = strdup(nptr->key);
                nptr = nptr->next;
                j++;
            }
        }

        qsort(sorted, bptr->num_keys, sizeof(char*), compare_key);

        for(i = 0; i < bptr->num_keys; i++)
            reduce_function(sorted[i], get_next, index);

        for(i = 0; i < bptr->num_keys; i++)
            free(sorted[i]);

        free(sorted);
    }
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition, int num_partitions) {

    NUM_FILES = argc - 1;
    files = (char **) (argv + 1);
    map_function = map;
    reduce_function = reduce;
    cur_file = 0;
    cur_partition = 0;
    if (num_partitions == 0)
        return;
    else
        NUM_PARTITIONS = num_partitions;

    if (partition != MR_SortedPartition)
        partition_function = MR_DefaultHashPartition;
    else
        partition_function = MR_SortedPartition;

    init_hash_table();

    pthread_t mapper_threads[num_mappers];

    int i;
    for (i = 0; i < num_mappers; i++) {
        pthread_create(&mapper_threads[i], NULL, Mapper_thread, NULL);
    }

    for (i = 0; i < num_mappers; i++) {
        pthread_join(mapper_threads[i], NULL);
    }

    pthread_t reducer_threads[num_reducers];
    for (i = 0; i < num_reducers; i++) {

        pthread_create(&reducer_threads[i], NULL, Reducer_thread, NULL);
    }

    for (i = 0; i < num_reducers; i++) {
        pthread_join(reducer_threads[i], NULL);
    }

    freeTable();
}