#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <queue>
#include <iostream>
#include <chrono>
#include <vector>
#include <fstream>
#include <string>
#include <semaphore.h>

sem_t semEmpty;
sem_t semFull;

volatile int running_threads = 0;
using std::cout;
using std::endl;
using std::vector;
using std::ofstream;
pthread_mutex_t mutex;
int timer=0;
//define a constant value for number of producers and consumers, can change.
#define NUM_PRODUCERS 1000
#define NUM_CONSUMERS 1
#define N 3
//size of the buffer to which traffic signals write data and consumers read from it.
const long SZ=12*NUM_PRODUCERS;
int l=0;
long _index=0;

/*
    Data structure to simulate a traffic signal data, which contains:
    Timestamp when the reading is taken,
    Traffic Light ID of the signal, and
    number of cars on the signal
*/
class TrafficData{
    public:
        long timestamp;
        long traffic_light_id;
        int number_of_cars; 
        TrafficData(long time,long id, int cars){
            timestamp=time;
            traffic_light_id=id;
            number_of_cars=cars;
        }
};

//buffer: an array of size SZ, with initial values being -1,-1,-1 for all the attributes.
TrafficData *data[SZ]={new TrafficData(-1,-1,-1)};

//pthread_cond_t: A condition variable must always be associated with a mutex, to avoid the race condition where a thread prepares to wait on a condition variable and another thread signals the condition just before the first thread actually waits on it.
pthread_cond_t canProduce;
pthread_cond_t canConsume;
//using the lock for blocking.
pthread_mutex_t lock;
//timestamp, id, and other variables to simulate the data.
long timestamp=0;
long traffic_light_id=0;
//file to write the data in
ofstream Data("TrafficData.txt");
ofstream Result("TopN.txt");

void *produce(void *args)
{
    //Simulation data is produced for an entire day, a record of traffic signal after each 5 mins.
    //thus we stop once the required number of readings are done.
    while(timestamp<=1440)
    {
        //blocking using the lock 
        pthread_mutex_lock(&lock);
        while(_index==SZ)
        {
            pthread_cond_wait(&canProduce,&lock);
        }
        //write the data produced in the file.
        //increase the index value of the buffer.
        if(timestamp<=1440)
        {
        //traffic data produced by the signal and stored in the buffer.
            for(int i=0;i<NUM_PRODUCERS;i++)
            {
                //generate the random number of cars.
                int car_numbers=rand()%100000;
                //add the data to the buffer.
                data[_index]=new TrafficData(timestamp,traffic_light_id,car_numbers);
                //write data to the file.
                Data<<data[_index]->timestamp<<","<<data[_index]->traffic_light_id<<","<<data[_index]->number_of_cars<<endl;
                //increment id number and index number.
                traffic_light_id++;
                _index++;
            }
        }
        //increase timestamp by 5 minutes.
        timestamp+=5;
        //broadcast to the consumers of that the data has been produced.
        pthread_cond_broadcast(&canConsume);
        //unblocking by releasing the lock. 
        pthread_mutex_unlock(&lock);
    }
}
//define the function: function to sort out the values based on number of cars.
bool comparePtrToNode(TrafficData* a, TrafficData* b) { return (a->number_of_cars > b->number_of_cars); }


void *consume(void *args)
{
    //list to store the data for an hour.
    vector<TrafficData*> listOfData;
    while(true)
    {
        //blocking using the lock. 
        pthread_mutex_lock(&lock);
        //if the index is 0, no data yet produced and wait for consumption, else data produced
        while(_index==0)
        {
            pthread_cond_wait(&canConsume,&lock);
        }
        //cout<<_index<<endl;
        for(int i=0;i<_index;i+=1)
        {
            //push data to new list
            listOfData.push_back(data[i]);
        }
        _index=0;
        //sign the producer threads to produce the data.
        pthread_cond_broadcast(&canProduce);
        //data has been consumed, release the lock.
        pthread_mutex_unlock(&lock);
        //the data of an hour has arrived.
        if(listOfData.size()==(NUM_PRODUCERS*12))
        {
            cout<<listOfData.size()<<endl;
            //sort the list based on number of cars.
            std::sort(listOfData.begin(), listOfData.end(), comparePtrToNode);
            //lock to manage the timer variable
            pthread_mutex_lock(&mutex);
            cout<<timer/60<<endl;
            if(timer/60!=23)
                cout<<"Time: "<<timer/60<<":00"<<" -- "<<timer/60+1<<":00"<<endl;
            else
                cout<<"Time: "<<timer/60<<":00"<<" -- "<<"00"<<":00"<<endl;
            //lock to manage the timer variable
            pthread_mutex_unlock(&mutex);
            //top 3 congested traffic signals.
            for(int begin=0;begin<N;begin++)
            {
                cout<<"Timestamp:  "<<listOfData[begin]->timestamp<<endl;
                cout<<"Number of Cars: "<<listOfData[begin]->number_of_cars<<endl;
                cout<<"Traffic Signal Id: "<<listOfData[begin]->traffic_light_id<<endl;
                Result<<"Timestamp:  "<<listOfData[begin]->timestamp<<endl;
                Result<<"Number of Cars: "<<listOfData[begin]->number_of_cars<<endl;
                Result<<"Traffic Signal Id: "<<listOfData[begin]->traffic_light_id<<endl;
                cout<<endl;

            }
            cout<<endl;
            //move to next hour.
            timer+=60;
            listOfData.clear();
            //cout<<j<<" is the timestamp"<<endl;
        }
    }
}

int main()
{
    srand(time(NULL));
    Data<<"Timestamp"<<","<<"Traffic Signal ID"<<","<<"Number of Cars"<<endl;
    //initialise the mutexes and wait conditions.
    pthread_cond_init(&canProduce,NULL);
    pthread_cond_init(&canConsume,NULL);
    pthread_mutex_init(&lock,NULL);
    pthread_mutex_init(&mutex,NULL);

    //threads array.
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    for(int i=0;i<NUM_PRODUCERS;i++)
    {
        pthread_create(&producers[i],NULL,produce,NULL);
    }
    for(int i=0;i<NUM_CONSUMERS;i++)
    {
        pthread_create(&consumers[i],NULL,consume,NULL);
    }
    for(int i=0;i<NUM_CONSUMERS;i++)
    {
        pthread_join(consumers[i],NULL);
    }
    for(int i=0;i<NUM_PRODUCERS;i++)
    {
        pthread_join(producers[i],NULL);
    }
    pthread_mutex_destroy(&lock);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&canConsume);
    pthread_cond_destroy(&canProduce);

    return 0;

}