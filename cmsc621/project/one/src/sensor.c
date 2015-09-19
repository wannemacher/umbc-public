/*
 * Code for the sensor
 */
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stddef.h>
#include <time.h>

struct interval
{
    int startTime;
    int endTime;
    int value;
};

typedef struct interval Interval;

Interval *intArray;
pthread_mutex_t mutex;
pthread_cond_t notfull, notempty;
int value;
int maxIntervals = 10;

void *updateValue(void *numIntervals)
{
    int loop = 0;
    int numInts = *(int*)numIntervals;
    int currentInterval = 0;
    int modulus = intArray[numInts-1].endTime;
    
    //set the value for the first time here
    value = intArray[currentInterval].value;
    
    //printf("Num intervals is %d\n",numInts);
    //printf("Modulus is %d\n",modulus);

    //printf("Initial value is %d\n",value);
    //printf("About to start updating the value!\n");
    
    while(1)
    {
        long int curTime = clock()/CLOCKS_PER_SEC;
        printf("time is %ld\n",curTime);
        printf("value is %d\n",value);
        
        if ((curTime - (loop * modulus)) >= intArray[currentInterval].endTime)
        {
            currentInterval+=1;
            if(currentInterval > numInts-1)
            {
                //printf("I got here, but what happened next?\n");
                //test = 1;
                currentInterval = 0;
                loop+=1;
            }
            //test = 1;
            printf("switching to new value: %d\n",intArray[currentInterval].value);
            value = intArray[currentInterval].value;
        }
    }
}

int main(int argc, char *argv[])
{
    printf("Current time is: %ld\n",clock()/CLOCKS_PER_SEC);
    //check to make sure we have both input files
    if(argc != 3)
    {
        printf("Missing configuration or input file\n");
        exit(1);
    }
    
    //attempt to open and read the files
    FILE *config, *input;
    config = fopen(argv[1],"r");
    input = fopen(argv[2],"r");
    
    printf("Reading input files...");
    
    if (config == NULL)
    {
        printf("Cannot open %s\n",argv[1]);
        exit(1);
    }
    else if (input == NULL)
    {
        printf("Cannot open %s\n",argv[2]);
        exit(1);
    }
    else
    {
        printf("Success!\n");
    }
    
    //variables for reading from config file
    char gatewayLine[100];
    char itemLine[100];
    char gatewayIp[16];
    char intervalLine[100];
    char temp;
    int gatewayPort;
    int octet1,octet2,octet3,octet4;
    
    //grab the two lines from the file
    fgets(gatewayLine,sizeof(gatewayLine),config);
    fgets(itemLine,sizeof(itemLine),config);
    fclose(config);
    
    //read the first line of the file, parse the IP and port
    sscanf(gatewayLine,"%d%c%d%c%d%c%d%c%d", &octet1, &temp, &octet2, &temp, &octet3, &temp, &octet4, &temp, &gatewayPort);
    
    //printf("IP is %d\n",octet1);
    //printf("IP is %d\n",octet2);
    //printf("IP is %d\n",octet3);
    //printf("IP is %d\n",octet4);
    //printf("Port is %d\n",gatewayPort);
    
    //int pos, pos2, pos3, pos4;
    //pos = snprintf(gatewayIp,sizeof(gatewayIp),"%d.",octet1);
    //pos2 = snprintf(gatewayIp+pos,sizeof(gatewayIp)-pos,"%d.",octet2);
    //pos3 = snprintf(gatewayIp+pos+pos2,sizeof(gatewayIp)-pos-pos2,"%d.",octet3);
    //pos4 = snprintf(gatewayIp+pos+pos2+pos3,sizeof(gatewayIp)-pos-pos2-pos3,"%d",octet4);

    
    //printf("IP: %s\n",gatewayIp);
    
    const char delimeters[] = ":";
    char *gateIp, *gatePort;
    gateIp = strtok(gatewayLine,delimeters);
    gatePort = strtok(NULL,delimeters);
    
    printf("ip is %s\n",gateIp);
    printf("port is %s\n",gatePort);
    
    char *copy, *type, *ip, *port, *areaId;
    //copy = strdupa(itemLine);
    type = strtok(itemLine,delimeters);
    ip = strtok(NULL,delimeters);
    port = strtok(NULL,delimeters);
    areaId = strtok(NULL,delimeters); 
    
    printf("type is %s\n",type);
    printf("ip is %s\n",ip);
    printf("port is %s\n",port);
    printf("areaId is %s\n",areaId);
    
    intArray = malloc(maxIntervals*sizeof(Interval));
    
    int count = 0;
    while(fgets(intervalLine,sizeof(intervalLine),input) != NULL)
    {
        int start, stop, value;
        sscanf(intervalLine,"%d;%d;%d", &start, &stop, &value);
        Interval newInterval;
        newInterval.startTime = start;
        newInterval.endTime = stop;
        newInterval.value = value;
        intArray[count] = newInterval;
        printf("Have this interval: %d, %d, %d\n",intArray[count].startTime,intArray[count].endTime,intArray[count].value);
        count+=1;        
        if(count >= maxIntervals)
        {         
            void *temp = realloc(intArray,maxIntervals*2*sizeof(Interval));

            if(!temp)
            {
                printf("Bad things happened!\n");
                exit(1);
            }
            else
            {
                intArray = temp;
            }
            maxIntervals = maxIntervals*2;
        }
    }
    
    //need to create a socket
    
    pthread_t valueUpdater, notifier;
    pthread_create(&valueUpdater, NULL, updateValue, (void *)&count);
    
    pthread_join(valueUpdater, NULL);
    
    //pthread_create(&notifier, NULL, notifyGateway, NULL);
    
    //need to create a thread that updates the values
    //need to create a thread that sends message to the gateway   
}
