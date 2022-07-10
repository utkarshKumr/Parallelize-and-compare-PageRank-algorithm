#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

#define maxThreads 64

struct timeval tBegin, tFinish;
typedef struct{
	int threadID;
	int start, end;
} Thread; 
typedef struct{
	double pOld;
	double pNew;
	double e;
	int *From_id;
	int totalOutgoing;
	int edges;
}Node;

int totalNodes, totalThds, iterations = 0;	// Number of nodes and threads and init iterations of PR
double permissibleDiff = 1, broadcastScore = 0;

double thold;
double d;	// threshold for converging and dampening factor

pthread_t *thdArray;  //array for threads

Thread *thdStats;	// array for thread's id, begin and end indices

Node *Nodes;	// Table of node's data
pthread_mutex_t lockBdcastScore = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockError = PTHREAD_MUTEX_INITIALIZER;

//Init thdArray storing for storing each Thread's stats
void thdNodeArrayInit(){
	thdArray = (pthread_t *)malloc(totalThds * sizeof(pthread_t));
	thdStats = (Thread*)malloc(totalThds * sizeof(Thread));	
	
	double nodesPerThread =  (double) totalNodes / totalThds;
	//for giving each thread its share of nodes and storing stats
	thdStats[0].threadID = 0;
	thdStats[0].start = 0;
	thdStats[0].end = floor(nodesPerThread);

	int i;
	for (i = 1; i < totalThds; i++){
		thdStats[i].threadID = i;
		thdStats[i].start = thdStats[i - 1].end;
		thdStats[i].end = i < (totalThds - 1) ? thdStats[i].start + floor(nodesPerThread) : totalNodes;
	}

	// for (i = 0; i < totalThds; i++)
	// 	printf("\nThread %d, start = %d, end = %d", thdStats[i].threadID, thdStats[i].start, thdStats[i].end);
	// printf("\n");

	Nodes = (Node*)malloc(totalNodes*sizeof(Node));
    for (i = 0; i < totalNodes; i++){
		Nodes[i].totalOutgoing = 0;
		Nodes[i].edges = 0;
        Nodes[i].From_id = (int*) malloc(sizeof(int));
    }	
}

//Init random probabilities for all the nodes - here we do 1/totalNodes
void initRandomProb(){
   	int i;
    for (i = 0; i < totalNodes; i++){
        Nodes[i].pOld = 0;
        Nodes[i].pNew = 1;	//init initial prob to 1
        Nodes[i].pNew = (double) Nodes[i].pNew / totalNodes;	//then assign equal prob to all
        
		Nodes[i].e = 1;
        Nodes[i].e = (double) Nodes[i].e / totalNodes;
    }
}

//Reading the graph input file
void readInputFile(char* fName){
    FILE *fid;

    int fromIndex, toIndex;
	int currSize;
	char buffer[1000];

    fid = fopen(fName, "r");
   	if (fid==NULL)
        printf("Unable to open file\n");

	while (!feof(fid)){
		fgets(buffer, sizeof(buffer), fid);
		if (strncmp(buffer, "#", 1) == 0)  //# are comments/metadata
            continue;
		if (sscanf(buffer,"%d\t%d\n", &fromIndex, &toIndex)){
			Nodes[fromIndex].totalOutgoing++;
			Nodes[toIndex].edges++;
			currSize = Nodes[toIndex].edges;
			Nodes[toIndex].From_id = (int*) realloc(Nodes[toIndex].From_id, currSize * sizeof(int));
			Nodes[toIndex].From_id[currSize - 1] = fromIndex; 
		}
	}
	//done reading file, close it
	fclose(fid);

	// init equal prob
    initRandomProb();
}

void* reinitRandomP(void* arg){
	Thread *tidStats = (Thread *)arg;
	int i;
	for (i = tidStats->start; i < tidStats->end; i++){
		Nodes[i].pOld = Nodes[i].pNew;	
		Nodes[i].pNew = 0;
	}
	return 0;
}

//pR algorithm run by each thread on subsets assigned
void* prParallel(void* arg){
	Thread *tidStats = (Thread *) arg;
	int i, j, index;

	// Every thread will compute a local broadcastScore and add it to the global one
	double temp_sum = 0;
	for (i = tidStats->start; i < tidStats->end; i++){
		if (Nodes[i].edges != 0){
            // Compute the total probability, contributed by node's neighbors
            for (j = 0; j < Nodes[i].edges; j++){
				index = Nodes[i].From_id[j];	
				Nodes[i].pNew += (double) Nodes[index].pOld / Nodes[index].totalOutgoing;
			}
        }
        if (Nodes[i].totalOutgoing == 0)
			temp_sum = temp_sum + (double) Nodes[i].pOld / totalNodes;
	}
	
	pthread_mutex_lock(&lockBdcastScore);
	broadcastScore += temp_sum;		//critical section
	pthread_mutex_unlock(&lockBdcastScore);
	return 0;
}

void* perThreadError(void* arg){
	Thread *tidStats = (Thread *) arg;
	int i;

	// Every thread will find a local max and then check if this is a global one
	double temp_max = -1;

	for (i = tidStats->start; i < tidStats->end; i++){
		Nodes[i].pNew = d * (Nodes[i].pNew + broadcastScore) + (1 - d) * Nodes[i].e;
        if (fabs(Nodes[i].pNew - Nodes[i].pOld) > temp_max)
            temp_max  = fabs(Nodes[i].pNew - Nodes[i].pOld);
	}

	// Check if we have a new global max
	pthread_mutex_lock(&lockError);
	permissibleDiff = permissibleDiff  < temp_max ? temp_max : permissibleDiff;
	pthread_mutex_unlock(&lockError);	
	return 0;
}

void parallelPagerank(){
	//until the current error is within the permissible range keep going
    while (thold < permissibleDiff){
    	permissibleDiff = -1; broadcastScore = 0;
		int i;
        for (i = 0; i < totalThds; i++)	// P array re-Initialization
			pthread_create(&thdArray[i], NULL, &reinitRandomP,(void*) &thdStats[i]);

		for (i = 0; i < totalThds; i++)	//explicit wait for all threads
			pthread_join(thdArray[i], NULL);

        for (i = 0; i < totalThds; i++)	//basic PR formula
            pthread_create(&thdArray[i], NULL, &prParallel, (void*) &thdStats[i]);   

		for (i = 0; i < totalThds; i++)
			pthread_join(thdArray[i], NULL);

		for (i = 0; i < totalThds; i++)	// Find local and global max
            pthread_create(&thdArray[i], NULL, &perThreadError, (void*) &thdStats[i]);   

		for (i = 0; i < totalThds; i++)
			pthread_join(thdArray[i], NULL);
        
        printf("Max Error round %d = %f\n", iterations+1, permissibleDiff);
        iterations++;
    }
}

void initBegin(){
	gettimeofday(&tBegin, NULL);
}

void initEnd(){
	gettimeofday(&tFinish, NULL);
}

int main(int argc, char** argv){
	double duration;
	char fName[256];

	// if commandline args are correct
	if (argc < 5){
        printf("\nPlease enter correct number(4) of arguments\n");
		printf(" 1) Input file Graph\n 2) Total Vertices\n 3) threshold\n 4) total Threads");
		exit(1);
	}
	
	totalThds = atoi(argv[4]);
	// Check input arguments
	if ((totalThds < 1) || (totalThds > maxThreads)){
		printf("Threads number must be >= 1 and  <= %d!\n", maxThreads);
		exit(1);
	}
	
	printf("pThreads Pagerank Parallel\n");
	strcpy(fName, argv[1]);
	totalNodes = atoi(argv[2]);
	
	// thold = atof(argv[3]);
	thold = 0.0000001;
	// d = atof(argv[4]);
	d = 0.3;

	//initialize the arrays to be used in algo
	thdNodeArrayInit();
	
	//getting input from the graph files
    readInputFile(fName);    

    //call to parallel pagerank init
    initBegin();
    parallelPagerank();
	initEnd();
    
	// total iterations that passed during the algo run
    printf("\nNumber of iterations spent: %d\n", iterations);

    //total time spent doing the pThread parallel pagerank
    duration = (((tFinish.tv_usec - tBegin.tv_usec) / 1.0e6+ tFinish.tv_sec - tBegin.tv_sec) * 1000) / 1000;
	printf("Total seconds spent = %f \n", duration);
    return (EXIT_SUCCESS);
}