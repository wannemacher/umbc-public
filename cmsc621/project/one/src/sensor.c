/*
 * Code for the sensor
 */
//#define _GNU_SOURCE
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
int updateInterval = 5;
char state[4] = "ON";

void *updateValue(void *numIntervals)
{
    int test = 0;
    int loop = 0;
    int numInts = *(int*)numIntervals;
    int currentInterval = 0;
    int modulus = intArray[numInts-1].endTime;
    printf("Starting to update values...\n");
    
    //printf("Num intervals is %d\n",numInts);
    //printf("Modulus is %d\n",modulus);

    //printf("Initial value is %d\n",value);
    //printf("About to start updating the value!\n");
    
    while(1)
    {
        long int curTime = clock()/CLOCKS_PER_SEC;
        
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

//send the gateay currValue updates based on the specified interval
void *notifyGateway(void *openSocket)
{
    int socketDesc = *(int*)openSocket;
    char updateMessage[1000];
    long int sentTime = clock()/CLOCKS_PER_SEC;
    long int curTime;
    
    //printf("Starting to notify gateway...\n");
    
    //send initial value
    sprintf(updateMessage,"Type:currValue;Action:%d",value);
    //printf("My socket is %d\n",socketDesc);
    if( send(socketDesc, updateMessage, sizeof(updateMessage), 0) < 0)
    {
        perror("Initial notification send failed. Error.\n");
    }
    
    //continually send messages based upon the specified interval
    while(1)
    {
        curTime = clock()/CLOCKS_PER_SEC;
        if(curTime - sentTime >= updateInterval)
        {
            if(strcmp(state,"ON") == 0)
            {
                sprintf(updateMessage,"Type:currValue;Action:%d",value);
                if( send(socketDesc, updateMessage, sizeof(updateMessage), 0) < 0)
                {
                    perror("Routine notification send failed. Error.\n");
                }
                sentTime = curTime;
            }
        }
    }
}

void *receiveGatewayMessages(void *openSocket)
{
    int socketDesc = *(int*)openSocket;
    char receivedMessage[2000];
    const char delimeters[] = ":;";
    char* nextState;
    //printf("Starting receiving thread\n");
    
    //fancy select stuff we don't need
    //fd_set mySocks;
    //struct timeval myTimeVal;
    //myTimeVal.tv_sec = 0;
    //myTimeVal.tv_usec = 0;
    
    while(1)
    {
        if( recv(socketDesc, receivedMessage, 2000, 0) < 0)
        {
            printf("No data from server :(\n");
        }
        else
        {
            if(strncasecmp(receivedMessage,"Type:Switch;Action:",19) == 0)
            {
                printf("Got message to switch state of sensor...\n");
                strtok(receivedMessage,delimeters);
                strtok(NULL,delimeters);
                strtok(NULL,delimeters);
                nextState = strtok(NULL,delimeters);
                if(strncasecmp(nextState,"ON",2) == 0)
                {
                    printf("Sensor is now ON\n");
                    strcpy(state,"ON");
                }
                else if(strncasecmp(nextState,"OFF",3) == 0)
                {
                    printf("Sensor is now OFF\n");
                    strcpy(state,"OFF");
                }
                else
                {
                    printf("Received invalid state request from server...\n");
                }
                
                //TO-DO: send a currState message to server (potentially need to be careful here)?
                //print-out doesn't say that we need to?
            }
            else if(strncasecmp(receivedMessage,"Type:setInterval;Action:",24) == 0)
            {
            }
            else
            {
                printf("Got unknown message type from server...\n");
            }
        }
        //fanciness with select() that might not be worthwhile
        /*FD_ZERO(&mySocks);
        FD_SET(socketDesc, &mySocks);
        
        if( select(1, &mySocks, NULL, NULL, &myTimeVal) == 0)
        {
            printf("Timed out waiting for server message...\n");
        }
        else
        {
            if(FD_ISSET(socketDesc, &mySocks))
            {
            
            }
            else
            {
                printf("Got a message from an invalid socket?...\n");
            }
        }*/
    }    
}

//reads sensor config files, sets up socket, spawns threads for communication
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
    
    //printf("Reading input files...");
    
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
    
    //printf("ip is %s\n",gateIp);
    //printf("port is %s\n",gatePort);
    
    char *copy, *type, *ip, *port, *areaId;
    //copy = strdupa(itemLine);
    type = strtok(itemLine,delimeters);
    ip = strtok(NULL,delimeters);
    port = strtok(NULL,delimeters);
    areaId = strtok(NULL,delimeters); 
    
    //printf("type is %s\n",type);
    //printf("ip is %s\n",ip);
    //printf("port is %s\n",port);
    //printf("areaId is %s\n",areaId);
    
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
        //printf("Have this interval: %d, %d, %d\n",intArray[count].startTime,intArray[count].endTime,intArray[count].value);
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
    fclose(input);
    
    //set initial value for sensor here before threads start!
    value = intArray[0].value;
    
    pthread_t valueUpdater, notifier, receiver;
    pthread_create(&valueUpdater, NULL, updateValue, (void *)&count);

    int sock;
    struct sockaddr_in server;
    char message[1000], server_reply[2000];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        printf("Could not create socket\n");
    }
    //printf("Socket created!\n");
    
    server.sin_addr.s_addr = inet_addr(gateIp);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(gatePort));
    
    //TO-DO - Client side binding!
    
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Connection failed. Error.\n");
        return 1;
    }
    //printf("Socket connected!\n");
    
    //compose initial registration message
    char registerMessage[1000];
    int len = sprintf(registerMessage,"Type:register;Action:%s-%s-%s-%s",type,ip,port,areaId);
    
    //send sensor registration message
    if( send(sock, registerMessage, sizeof(registerMessage), 0) < 0)
    {
        printf("Sending registration failed. Error.\n");
    }
    
    //printf("My initial socket is %d\n",sock);
    
    //TO-DO - send currState message?
    //Talk to Randy to see if gateway should just assume that the state is true at first
    
    //start threads for sending/receiving from gateway
    pthread_create(&notifier, NULL, notifyGateway, (void *)&sock);
    //pthread_create(&receiver, NULL, receiveGatewayMessages, NULL);        
    
    pthread_join(valueUpdater, NULL);
    pthread_join(notifier, NULL);
    //pthread_join(receiver, NULL);
    
    close(sock);
}
