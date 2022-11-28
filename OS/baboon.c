/*            INTRO
-Cassandra Ganska, Mike Lopez III, Kaily Luna
11/22/22
Operating Systems

Baboon Problem: Synchronization
- 2 baboons swinging opposite direction using rope ; rope holds only 3 ; if they meet in the middle they fight and drop to their deaths
- once crossing must be guaranteed to arrive on the other side
- order must be preserved (FIFO)
- no starvation
- no deadlock

input: input file - input file name and time (seconds)
output: print on terminal screen
*/


//header
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// semaphore creation
    // prevents baboons from both sides getting on the rope
sem_t rope; 
    //ensures queue stays in order while crossing on the side of (r -> l)
sem_t r_l_mutex; 
    //ensures queue stays in order while crossing on the side of (l -> r)    
sem_t l_r_mutex; 
    //prevents one side from holding it forever    
sem_t mutex; 
     //prevents more than 3 -> capacity of problem
sem_t capacity; 
    //check if rope is free to tell the semaphore rope to post or wait
int left = 0; 
int right = 0; 
    //baboon waiting time to cross
int arrival_time; 

//function for left to right side travelling of baboon
void *left_to_right(void *traveling_baboon ) {
        // wait for both to be available
    sem_wait( &mutex );
    sem_wait( &l_r_mutex );
        // increment left value
    left++;
    // if left == 1 -> no babbons are attempting to travel
    // checks if rope is being used by other side
    if ( left == 1 ) {
        sem_wait( &rope );
    }

    // var holds babbon number & how many are on rope
    int *num = (int*) traveling_baboon;
    int on_rope;  

    /*
    mutex semaphores allowed to start another thread
    other side may not start until rope semaphore is posted : must wait for this side to finish
    */  
    sem_post( &l_r_mutex );
    sem_post( &mutex );

    // verifies capacity is not empty and that it's not greater than 3 at once
    sem_wait( &capacity );
    sem_getvalue( &capacity, &on_rope );

    // beginning of simulation      
    printf( "Baboon #%d beginning to cross, there are %d baboons on the rope going {L->R}\n\n", *num, 3 - on_rope );

     // arrival_time provided by user   
    sleep( arrival_time );
    sem_getvalue( &capacity, &on_rope );

    // simulate when baboon finishes crossing  
    printf( "Baboon #%d finished crossing from {L->R}, there are %d baboons on the rope\n\n",*num, 2 - on_rope);

    // repost to capacity when baboon finishes (- position on rope)
    sem_post( &capacity );

    // decrement value in left 
    sem_wait( &l_r_mutex );
    left--;
    
    /*
    if left == 0 -> no baboons are crossing
    rope incremented and other side allowed to start crossing
    */    
    if ( left == 0 )
        sem_post( &rope );

    sem_post( &l_r_mutex );
    pthread_exit( NULL );
}// end of left_right_rope

//function for right left to side travelling of baboon
void *right_to_left( void *traveling_baboon ) {
    sem_wait( &mutex );
    sem_wait( &r_l_mutex );
    right++;
    if ( right == 1 ) {
        sem_wait( &rope );
    }
    int *num = (int*) traveling_baboon;
    int on_rope;
    sem_post( &r_l_mutex );
    sem_post( &mutex );
    sem_wait( &capacity );
    sem_getvalue( &capacity, &on_rope );
    printf( "Baboon #%d beginning to cross, there are %d baboons on the rope going {R->L}\n\n", *num, 3 - on_rope );
    
    sleep( arrival_time );
    sem_getvalue( &capacity, &on_rope );
    
    printf( "Baboon #%d finished crossing from {R->L}, there are %d baboons on the rope\n\n", *num, 2 - on_rope);
    
    sem_post( &capacity );
    sem_wait( &r_l_mutex );
    right--;
    if ( right == 0 )
        sem_post( &rope );

    sem_post( &r_l_mutex );
    pthread_exit( NULL );
}// end of right_left_rope

// main
int main( int argc, char **argv ) {
    // to read file
    FILE *file; 

    // takes time for baboons to cross
    int *stopwatch = ( int * )malloc(sizeof( int ) );

    // hold queue of baboons 
    char baboons[100]; 
    int baboon_counter = 0;

    // counter for how many on rope in each direction
    int left_counter = 0;    
    int right_counter = 0;

    // holds read char from file
    char direction;

        //checks to see if it contains the right amount of parameters    
    if ( argc < 3 ) {
        printf( "Enter a file name followed by an integer between 1-10 that represents the time taken to cross the rope\n" );
        free( stopwatch );
        return -1;
    }
    file = fopen( argv[1], "r" ); 
    int grab_time = atoi( argv[2] );
    arrival_time = grab_time;

     // fill queue based on order from file    
    while ( ( fscanf( file, "%c", &direction ) != EOF ) ) {
        if ( direction == ',' )
            continue;
        if ( direction == 'L' ) {
            left_counter = left_counter + 1;
            baboons[baboon_counter] = 'L';
            baboon_counter = baboon_counter + 1;
        }
        if ( direction == 'R' ) {
            right_counter = right_counter + 1;
            baboons[baboon_counter] = 'R';
            baboon_counter = baboon_counter + 1;
        }
    }// end of while

    //handle printing of baboons crossinig    
    printf("\nThere are %d baboons waiting to cross from {L->R}. There are %d baboons waiting to cross from {R->L}\n\n",left_counter, right_counter);
    printf("BABOON ORDER: ");
   
    for(int i = 0; i < baboon_counter; i++){
        printf("%c ", baboons[i]);
    }
    printf("\n\nEach baboon will take %d seconds to cross.\n\n", arrival_time);
    
    //initializing threads for each side   
    pthread_t l_to_r_thread[left_counter];
    int lThread_counter = 0;
    pthread_t r_to_l_thread[right_counter];
    int rThread_counter = 0;

    //semaphore initialization   
    sem_init( &rope, 0, 1 );
    sem_init( &r_l_mutex, 0, 1 );
    sem_init( &l_r_mutex, 0, 1 );
    sem_init( &mutex, 0, 1 );
    sem_init( &capacity, 0, 3 );

    // threads created on order of queue   
    for ( int i = 0; i < baboon_counter; i++ ) {
        int *tharg = (int*)malloc(sizeof(*tharg));
            *tharg = i;
        sleep(1);
        if ( baboons[i] == 'L' ) {
                       
        printf( "Baboon #%d is waiting to cross from {L->R}\n\n", i );
        pthread_create( &l_to_r_thread[lThread_counter], NULL, left_to_right, tharg);
        lThread_counter++;
        } else if ( baboons[i] == 'R' ) {
        printf( "Baboon #%d is waiting to cross from {R->L}\n\n", i );
        pthread_create( &r_to_l_thread[rThread_counter], NULL, right_to_left, tharg);
        rThread_counter++;
        }
    }// end of for

     // waiting for threads to finish    
    for ( int j = 0; j < lThread_counter; j++ ) {
        pthread_join( l_to_r_thread[j], NULL );
    }
    for ( int j = 0; j < rThread_counter; j++ ) {
        pthread_join( r_to_l_thread[j], NULL );
    }
    free( stopwatch );
    
    //semaphores destroyed
    sem_destroy( &rope );
    sem_destroy( &r_l_mutex );
    sem_destroy( &l_r_mutex );
    sem_destroy( &mutex );
    sem_destroy( &capacity );
    return 0;
}// end of main
