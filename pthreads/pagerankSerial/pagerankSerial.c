#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>
#include <string.h>

struct timeval tBegin, tFinish;
typedef struct
{
  double pOld, pNew, e;
  int *neighbourID, totalOutgoing;
}Node;

double broadcastScore = 0;  //a node with no outgoing edges will give it's score equally to others
int totalNodes; // input from the user for total node
int i,j,iterations=0; //to count iterations
// threshold for converging and dampening factor
double d;
double thold;

Node *Nodes;    // Array pointer for vertices data

//Init random probabilities for all the nodes - here we do 1/totalNodes
void initRandomProb(){
   	int i;
       
    // Arrays initialization
    for (i = 0; i < totalNodes; i++){
        Nodes[i].pNew = 1;
        Nodes[i].pOld = 0;
        Nodes[i].pNew = (double) Nodes[i].pNew / totalNodes;
        
		Nodes[i].e = 1;
        Nodes[i].e = (double) Nodes[i].e / totalNodes;
    }
}

//Reading the graph input file
void readInputFile(char* fName) {
    FILE *fid;
    char buffer[1000];
    int start_node, tsize, end_node;;

    fid = fopen(fName, "r");
   	if (fid==NULL)
        printf("Unable to open file\n");

	while (!feof(fid)){
        fgets(buffer, sizeof(buffer), fid);
        if (strncmp(buffer, "#", 1) == 0)  //# are comments/metadata
            continue;
        if (sscanf(buffer,"%d\t%d\n", &start_node,&end_node)){
            Nodes[start_node].totalOutgoing++;
            tsize = Nodes[start_node].totalOutgoing;
            Nodes[start_node].neighbourID =(int*) realloc(Nodes[start_node].neighbourID, tsize * sizeof(int));
            Nodes[start_node].neighbourID[tsize - 1] = end_node;
        }
	}
    //done reading file
	fclose(fid);

    // init equal prob
    initRandomProb();
}

void serialPageRank(){
    double max_error = 1;   // it is the any allowable value > thold
    int i, j, index;
    // Continue if we don't have convergence yet
    while (thold < max_error){
        broadcastScore = 0;
        
        // Initialize P(t) and P(t + 1) values
        i=0;
        while(i<totalNodes){
            Nodes[i].pOld = Nodes[i].pNew;  //copy new values in old, so new can be generated  
            Nodes[i].pNew = 0;
            i++;
        }

        // Find P for each webpage
        for (i = 0; i < totalNodes; i++){
            if (Nodes[i].totalOutgoing != 0){
                j=0;
                while(j < Nodes[i].totalOutgoing){
                    index = Nodes[i].neighbourID[j];    
                    Nodes[index].pNew += (double) Nodes[i].pOld / Nodes[i].totalOutgoing;  //basic PR formula
                    j++;
                }
            }
            else
                broadcastScore = broadcastScore + (double)Nodes[i].pOld / totalNodes;  //'sink' gives value to all         
        }
        
        max_error = -1;        
        // Compute the new probabilities and find maximum error
        i=0;
        while(i<totalNodes){
            Nodes[i].pNew = (1 - d) * Nodes[i].e + d * (Nodes[i].pNew + broadcastScore);
            max_error = (max_error < fabs(Nodes[i].pNew - Nodes[i].pOld) ) ? fabs(Nodes[i].pNew - Nodes[i].pOld) : max_error;
            i++;
        }
        
        printf("Max Error round %d = %f\n", iterations+1, max_error);
        iterations++;
    }   
}

void initAndRunPR(){
    gettimeofday(&tBegin, NULL);
    serialPageRank();
    gettimeofday(&tFinish, NULL);
}

int main(int argc, char** argv){
    char fName[256];
	// if commandline args are correct
	if (argc < 3){
        printf("\nPlease enter correct number(4) of arguments\n");
		printf(" 1) Input file Graph\n 2) Total Vertices\n 3) threshold\n");
		return 0;
	}

	// read arguments 
	strcpy(fName, argv[1]);
	totalNodes = atoi(argv[2]);
	d = 0.3;
    // thold = atof(argv[3]);
    thold = 0.0000001;
	
    
	double duration;
    printf("\npThreads Pagerank Serial\n");
    
	// malloc for totalNodes
    Nodes = (Node*) malloc(totalNodes * sizeof(Node));
    
    for (i = 0; i < totalNodes; i++) {
		Nodes[i].totalOutgoing = 0;
        Nodes[i].neighbourID = (int*) malloc(sizeof(int));
    }
    readInputFile(fName);
    
    initAndRunPR();
	
    printf("\nNumber of iterations spent: %d\n", iterations); // Print no of iterations
    duration = (((tFinish.tv_usec - tBegin.tv_usec) / 1000000 + tFinish.tv_sec - tBegin.tv_sec) * 1000) / 1000;
	printf("Total seconds spent = %f \n", duration);
    
    return (EXIT_SUCCESS);
}