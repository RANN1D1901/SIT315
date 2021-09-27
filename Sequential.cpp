#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <queue>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

volatile int running_threads = 0;
using std::cout;
using std::endl;
using std::vector;
using std::ofstream;
using std::stringstream;
std::string line;
pthread_mutex_t mutex;
std::ifstream inFile;
int timer=0;

#define NUM_PRODUCERS 10
#define NUM_CONSUMERS 2

const long SZ=288*NUM_PRODUCERS;
int l=0;
long _index=0;


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


TrafficData *data[SZ]={new TrafficData(-1,-1,-1)};

pthread_cond_t canProduce;
pthread_cond_t canConsume;

pthread_mutex_t lock;
long timestamp=0;
long traffic_light_id=0;
int j=0;
int count=0;

void produce()
{
    inFile.open("TrafficDataSequential.txt");
    if(!inFile)
    {
        cout<<"Not file open"<<endl;
    }
    int lines=0;
    while (std::getline(inFile, line)) // Read a line at a time.  // This also checks the state of script after the read.
    {
        //cout<<line<<endl;// Do something with line
        stringstream ss(line);
        long time, id;
        int cars;
        int _count=0;
        if(lines!=0)
        {
            while (ss.good()) {
                std::string substr;
                getline(ss, substr, ',');
                if(_count==0)
                {
                    time=std::stol(substr);
                }
                else if(_count==1)
                {

                    id=std::stol(substr);
                }
                else{

                    cars=stoi(substr);
                }
                _count++;
            }
            data[_index++]=new TrafficData(time,id,cars);
        }
        lines++;
    }
    _index-=1;
    cout<<_index<<endl;
}
//define the function: to compare the number of cars at a given time.
bool comparePtrToNode(TrafficData* a, TrafficData* b) { return (a->number_of_cars > b->number_of_cars); }


void consume()
{
    vector<TrafficData*> listOfData;
    //int N=100;
    for(int i=0;i<_index;i+=1)
    {
        // cout<<data[i]->timestamp<<" is the timestamp"<<endl;
        // cout<<data[i]->number_of_cars<<" is the car count"<<endl;
        // cout<<data[i]->traffic_light_id<<" is the traffic signal id number"<<endl;
        listOfData.push_back(data[i]);
        if(listOfData.size()==12*NUM_PRODUCERS)
        {

            //cout<<j<<" is the timestamp"<<endl;
            //for(int begin=0;begin<listOfData.end();begin++)

            std::sort(listOfData.begin(), listOfData.end(), comparePtrToNode);

            if(timer/60!=23)
                cout<<"Time: "<<timer/60<<":00"<<" -- "<<timer/60+1<<":00"<<endl;
            else
                cout<<"Time: "<<timer/60<<":00"<<" -- "<<"00"<<":00"<<endl;

            for(int begin=0;begin<3;begin++)
            {
                cout<<"Number of Cars: "<<listOfData[begin]->number_of_cars<<endl;
                cout<<"Traffic Signal Id: "<<listOfData[begin]->traffic_light_id<<endl;
            }

            cout<<endl;
            timer+=60;


            listOfData.clear();

            j=0;

            //cout<<j<<" is the timestamp"<<endl;
        }
 
    }
}
int main()
{
    srand(time(NULL));
    produce();
    consume();
    cout<<_index<<endl;
    return 0;

}