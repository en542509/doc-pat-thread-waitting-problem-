#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>


pthread_mutex_t mutex_lock;

//declare semaphores
//semaphore for when doctor waits for a patient to show up *sleeps* patient notifies of arrival
sem_t patients_sem;
//patient waits for docs help, and the doc notifies patient they are ready to help
sem_t doc_sem;
//max sleep time
#define MAX_SLEEP_TIME 3
//the max number of visits a patient can make
#define NUMOFVISITS 2
//available seats in waiting room
#define numAvailableSeats 4

volatile int seats[numAvailableSeats] = {0};
volatile int empty_chair;
volatile int next_patient;
//waitting patients 
int occupied=0;

//intislize
void initialize()
{
								sem_init(&doc_sem,1,0);
								sem_init(&patients_sem,1,0);
								pthread_mutex_init(&mutex_lock,NULL);
								empty_chair=0;
								next_patient=0;
}

void* doctor(void *arg)
{
								int help_time = 0;
								printf("\nDoctors thread started");

								while(1) {

																sem_wait(&patients_sem); //doctor waiting for patient
																help_time = rand() % MAX_SLEEP_TIME + 1;

																pthread_mutex_lock(&mutex_lock);
																printf("\n\tDoctor has %d patients waiting",occupied);

																printf("\n\tDoctor is helping patient %d from seat #%d",seats[next_patient],(next_patient+1));
																occupied--;
																next_patient = (next_patient + 1)%numAvailableSeats;

																pthread_mutex_unlock(&mutex_lock);
																sleep(help_time);
																printf("\n\tDoctor is available for help.");
																sem_post(&doc_sem); //doc avaiable

								}
								pthread_exit(NULL);
}

void* patient(void *thread_id)
{

								
								int id = *(int*)thread_id;
								int help_count=0;
								int coffee_time;

								printf("\nStarted patient thread %d",id);

								while(1)
								{

																coffee_time = rand() % MAX_SLEEP_TIME + 1;
																sleep(coffee_time);
																printf("\npatient %d is going to doc for help #%d .",id,(help_count+1));


																pthread_mutex_lock(&mutex_lock);
																//Check if all seats are occupied
																if(occupied==numAvailableSeats)
																{
																								printf("\n No seat available for the patient %d. Going for a cup of Joe.",id);
																								pthread_mutex_unlock(&mutex_lock);
																								continue;
																}
																else
																{
																								seats[empty_chair]=id;
																								//increase chairs occupied
																								occupied++;

																								printf("\nPatient %d is waiting for the doc in their seat #%d",id,(empty_chair+1));
																								empty_chair = (empty_chair + 1)%numAvailableSeats;
																								pthread_mutex_unlock(&mutex_lock);
																								//tell the doc a patient is waiting sem
																								sem_post(&patients_sem);

																								pthread_mutex_unlock(&mutex_lock);

																								sem_wait(&doc_sem); //Now wait for doctor to to help

																								help_count++;

																								if(help_count==NUMOFVISITS) //if already got the maximum number of helps come out
																																break;
																}
								}

								printf("\npatient %d finished and exiting the hospital!",id);
								pthread_exit(NULL);
}

int main(int argc, char **argv )
{

								int numberOfpatients;
								printf("Enter # of patients: ");
								scanf("%d",&numberOfpatients);



								pthread_t dc_thread, patient_thread[numberOfpatients];
								int i = 1;
								initialize();

								pthread_create(&dc_thread,NULL,doctor,NULL);

								for(i=1; i<=numberOfpatients; i++)
								{
																pthread_create(&patient_thread[i-1],NULL,patient, &i);

								}

								for(i=1; i<=numberOfpatients; i++)
								{
																pthread_join(patient_thread[i-1],NULL);
								}

								printf("\n All patients are healed. The doctor is going home...\n");
								pthread_cancel(dc_thread);
}
