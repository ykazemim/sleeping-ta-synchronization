#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define MAX_CHAIRS 5
#define NUM_STUDENTS 10

// Semaphore to indicate number of students waiting
sem_t students_sem;
// Semaphore for TA to indicate ready to help a student
sem_t ta_sem;
// Mutex for mutual exclusion on waiting chairs count
pthread_mutex_t mutex;

// Number of students currently waiting
int waiting_students = 0;

void *student(void *id)
{
    int student_id = *(int *)id;

    while (1)
    {
        // Student arrives randomly
        sleep(rand() % 5 + 1);

        pthread_mutex_lock(&mutex);

        if (waiting_students < MAX_CHAIRS)
        {
            waiting_students++;
            printf("Student %d sits down. Waiting students = %d\n", student_id, waiting_students);

            // Signal TA that a student is waiting
            sem_post(&students_sem);

            pthread_mutex_unlock(&mutex);

            // Wait for TA to be ready to help
            sem_wait(&ta_sem);

            // Getting help
            printf("Student %d is getting help from TA.\n", student_id);
            sleep(1); // Time taken by TA to help

            printf("Student %d leaves after getting help.\n", student_id);

            break; // Student done, exit thread
        }
        else
        {
            // No chairs available
            printf("Student %d found no empty chair and will try later.\n", student_id);
            pthread_mutex_unlock(&mutex);

            // Wait a while before trying again
            sleep(rand() % 5 + 1);
        }
    }

    pthread_exit(NULL);
}

void *ta(void *arg)
{
    while (1)
    {
        // Wait until a student arrives
        sem_wait(&students_sem);

        pthread_mutex_lock(&mutex);

        // One student taken from waiting queue
        waiting_students--;
        printf("TA is helping a student. Waiting students now = %d\n", waiting_students);

        pthread_mutex_unlock(&mutex);

        // Signal one student that TA is ready to help
        sem_post(&ta_sem);

        // Simulate TA helping student
        sleep(1);

        if (waiting_students == 0)
        {
            printf("TA goes to sleep as no students are waiting.\n");
        }
    }
}

int main()
{
    pthread_t ta_thread;
    pthread_t students[NUM_STUDENTS];
    int student_ids[NUM_STUDENTS];

    srand(time(NULL));

    // Initialize semaphores and mutex
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Create TA thread
    pthread_create(&ta_thread, NULL, ta, NULL);

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++)
    {
        student_ids[i] = i + 1;
        pthread_create(&students[i], NULL, student, &student_ids[i]);
    }

    // Wait for all student threads to finish
    for (int i = 0; i < NUM_STUDENTS; i++)
    {
        pthread_join(students[i], NULL);
    }

    // Cleanup (in practice, TA thread runs infinitely)
    pthread_cancel(ta_thread);
    pthread_join(ta_thread, NULL);

    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);
    pthread_mutex_destroy(&mutex);

    return 0;
}
