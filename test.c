#include <stdio.h>
#include <assert.h> 
#include <pthread.h>

long thread_mapping[2];
void* mythread(void * arg) {
    printf("%s\n", (char * ) arg); 
    printf("%ld\n", (long) pthread_self());
    return NULL;
}

int main(int argc, char * argv[]) {

    pthread_t p1, p2;
    int rc; 
    printf("main: begin\n"); 
    pthread_create(&p1, NULL, mythread, "A"); 
    pthread_create(&p2, NULL, mythread, "B"); 
    pthread_join(p1, NULL); 
    pthread_join(p2, NULL); 
    printf("main: end\n"); 
    return 0;

}