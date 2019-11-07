/*
* FILE: vptree_pthreads.c
* THMMY, 7th semester, Parallel and Distributed Systems: 1st assignment
* Parallel implementation of vantage point tree
* Authors:
*   Moustaklis Apostolos, 9127, amoustakl@auth.gr
*   Portokalidis Stavros, 9334, stavport@auth.gr
* Compile command with :
*   make vptree_pthreads
* Run command example:
*   ./vptree_pthreads
* It will create the tree given N points with D dimensions
* return a vptree struct and check it's validity
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/time.h>
#include "vptree.h"

#define MAX_THREADS 2
#define NOTHREADS 4


//Function declarations
double qselect(double *v, int len, int k);
double  distanceCalculation(double * X, double * Y, int n, int d);
void * distanceCalculationPar(void *data);
void printFam(double *X,int *idx , double * Xinner ,int  numberOfInner ,double * Xouter ,
              int  numberOfOuter ,double * distance ,int  n ,int  d ,int idxVp , double median);

//Struct to pass arguments in the pthread function
typedef struct param {
  int * counter; //local node counter to associate max threads per level
  double * data; // the data of the points
  int * idx; // indexes of the points in the set
  int n; // N number of points
  int d; // D number of dimensions
  double *distances; //the distances from the vantage point
} param;

//Counter for the nodes that were made
int nodesMade = 0;

//Mutex and attribute
pthread_mutex_t mux;
pthread_mutex_t mux1;
pthread_attr_t attr;

//Active threads for node construction
volatile int activeThreads = 0;

//Recursive build function
void * recBuild(void * arg) {
  param * x = (param *) arg;
  vptree *p = (vptree *)malloc(sizeof(vptree));

    if (x->n == 1){
      p->vp = x->data;
      p->idxVp=x->idx[0];
      p->md=0;
      p->inner=NULL;
      p->outer=NULL;
      return (void *)p;
    }
    if(x->n == 0)
      return NULL;
//Local counter for active threads per node
//Helps the parallel distance calculation
  int localCounter = -1;
  x->counter = &localCounter;
  x->distances = (double * )calloc(x->n - 1, sizeof(double));


  pthread_t th1,th2; //threads for the recursion
  pthread_t thr[NOTHREADS]; //threads for the distance calculation
  void * r1, * r2;
  param inner , outer ;
  int parallel = 0;

  int numberOfOuter = 0; //Number of points in the outer set
  int numberOfInner = 0; //NUmber of points in the inner set
  double median; // The median of the vantage point
  double * Xinner = NULL; //Inner subtree
  double * Xouter = NULL; //Outer subtree
  int * idxInner= NULL; //Inner indexes
  int * idxOuter = NULL; //Outer indexes


  p->vp = (double * ) malloc(x->d * sizeof(double));
  for (int j = 0; j < x->d; j++) {
    p->vp[j] = * (x->data+ (x->n-1) * x->d + j);
  }
  p->idxVp = x->idx[x->n-1];

pthread_mutex_lock(&mux1);
nodesMade++;
pthread_mutex_unlock(&mux1);

//THRESHOLD TO GO SERIAL
//Use parallel calculation for  n > (number of points / 4)
//Or after 6 nodes were made
   if(x->n<250000){
     for(int i = 0; i < x->n-1; i++){
       x->distances[i]=distanceCalculation((x->data + i * x->d),p->vp,x->n,x->d);
    }
  }
    else{
      for(int i=0; i<NOTHREADS; i++){
        pthread_create(&thr[i], &attr, distanceCalculationPar, (void *)x);
      }
      for(int i=0; i<NOTHREADS; i++){
        pthread_join(thr[i],NULL);
      }
    }

  median = qselect(x->distances, x->n - 1, (int)((x->n - 2) / 2));
  p->md = median;

  numberOfOuter = (int)((x->n - 1) / 2);
  numberOfInner = x->n - 1 - numberOfOuter;

//Allocating memory for the subtrees
  if (numberOfInner != 0) {
    Xinner = (double * ) malloc(numberOfInner * x->d * sizeof(double));
    idxInner = (int * )malloc(numberOfInner*sizeof(int));
  }
  if (numberOfOuter != 0) {
    Xouter = (double * ) malloc(numberOfOuter * x->d * sizeof(double));
    idxOuter = (int * )malloc(numberOfOuter*sizeof(int));
  }

  int inCounter = 0; //number of Inner points//
  int outCounter = 0; //number of Outer points//
  for (int i = 0; i < x->n - 1; i++) {
    if (x->distances[i] <= p->md) {
      for (int j = 0; j < x->d; j++) {
        *(Xinner + inCounter * x->d + j) = * (x->data + i * x->d + j);
      }
      idxInner[inCounter] = x->idx[i];
      inCounter++;
    } else {
      for (int j = 0; j < x->d; j++) {
        *(Xouter + outCounter * x->d + j) = * (x->data + i * x->d + j);
      }
      idxOuter[outCounter]= x->idx[i];
      outCounter++;
    }
  }

  // pthread_mutex_lock(&mux);
 //printFam(x->data, x->idx ,  Xinner , numberOfInner , Xouter , numberOfOuter ,  x->distances , x->n , x->d ,p->idxVp , median );


  if (activeThreads < MAX_THREADS) {

    pthread_mutex_lock (&mux);
    activeThreads += 2;
    pthread_mutex_unlock (&mux);
    inner.data = Xinner;
    inner.idx = idxInner;
    inner.n = numberOfInner;
    inner.d = x->d;
    pthread_create( &th1, &attr, recBuild, (void*)(&inner));
    outer.data = Xouter;
    outer.idx = idxOuter;
    outer.n = numberOfOuter;
    outer.d = x->d;
    pthread_create( &th2, &attr, recBuild, (void *)(&outer));
    parallel = 1;
  }

  if (parallel) {
    pthread_join(th1,&r1);
    p->inner = (vptree *) r1;
    pthread_join(th2,&r2);
    p->outer = (vptree *) r2;
    pthread_mutex_lock (&mux);
    activeThreads -= 2;
    pthread_mutex_unlock (&mux);
  }
  else {
      free(x->data);
      free(x->idx);
      free(x->distances);
      inner.data = Xinner;
      inner.n = numberOfInner;
      inner.d = x->d;
      inner.idx = idxInner;
      p->inner = (vptree *)recBuild((void *) &inner);

      outer.data = Xouter;
      outer.n = numberOfOuter;
      outer.d = x->d;
      outer.idx = idxOuter;
      p->outer = (vptree *)recBuild((void *) &outer);

  }
  return (void *)p;
}
// ======= LIST OF ACCESSORS ======= //
vptree * buildvp(double *X, int n, int d){
  int * idx = (int *) malloc(n * sizeof(int));
   for(int i=0; i<n; i++){
     idx[i] = i;
   }
   param parametr ;
   parametr.data = X;
   parametr.n = n;
   parametr.d = d;
   parametr.idx = idx;
   parametr.distances=NULL;
   parametr.counter =NULL;

   return (vptree *)recBuild(&parametr);
}

vptree * getInner(vptree * T) {
  return T->inner;
}

vptree * getOuter(vptree * T) {
  return T->outer;
}

double * getVP(vptree * T) {
  return T->vp;
}

double getMD(vptree * T) {
  return T->md;
}

int getIDX(vptree * T) {
  return T->idxVp;
}
// ======= HELPER FUNCTIONS ======= //
double qselect(double *v, int len, int k)
{
	#	define SWAP(a, b) { tmp = tArray[a]; tArray[a] = tArray[b]; tArray[b] = tmp; }
	int i, st;
	double tmp;
	double * tArray = (double * ) malloc(len * sizeof(double));
	for(int i=0; i<len; i++){
		tArray[i] = v[i];
	}
	for (st = i = 0; i < len - 1; i++) {
		if (tArray[i] > tArray[len-1]) continue;
		SWAP(i, st);
		st++;
	}
	SWAP(len-1, st);
	return k == st	? tArray[st]
			:st > k	? qselect(tArray, st, k)
				: qselect(tArray + st, len - st, k - st);
}


double  distanceCalculation(double * X, double * Y, int n, int d) {
    double dist2 = 0;
    for (int i = 0; i < d; i++){
      dist2 += (X[i] - Y[i])*(X[i] - Y[i]);
    }
    return sqrt(dist2);
}

void * distanceCalculationPar(void *data) {
  param *localDist= (param *) data;
  int i,j,start,end,iterations=0 , lastIt;

  double sumDist=0;
  int localThreadCounter = 0 ;

  pthread_mutex_lock(&mux);
  if (*(localDist->counter)==-1 || *(localDist->counter)==NOTHREADS-1){
    *(localDist->counter)=0;
    localThreadCounter = *(localDist->counter);
  }
  else if(*(localDist->counter)<NOTHREADS-1){
    (*(localDist->counter))++;
    localThreadCounter = *(localDist->counter);
  }
  pthread_mutex_unlock(&mux);


  if((localDist->n-1)%NOTHREADS ==0){
    iterations = ((localDist->n)/NOTHREADS);
    start = localThreadCounter *iterations;
    end = start + iterations;
  //  printf("Thread %d is doing iterations %d to %d \n",localThreadCounter,start,end-1);
  }
  else {
    iterations= round((localDist->n-1)/NOTHREADS);
    lastIt = (localDist->n-1) - (NOTHREADS-1)*iterations ;

    if(localThreadCounter == NOTHREADS-1){
      start = (localThreadCounter)*iterations;
      end = start+lastIt;
    //  printf("Thread %d is doing iterations %d to %d \n",localThreadCounter,start,end-1);
    }
    else{
      start = (localThreadCounter)*iterations;
      end = start + iterations;
  //   printf("Thread %d is doing iterations %d to %d \n",localThreadCounter,start,end-1);
    }
  }

  for (i=start; i<end; i++){
    for(j=0; j<localDist->d; j++){
      sumDist= sumDist + (*(localDist->data + (localDist->n-1)*localDist->d + j) - *(localDist->data + i*localDist->d + j)) * (*(localDist->data + (localDist->n-1)*localDist->d + j) - *(localDist->data + i*localDist->d + j));
    }
    localDist->distances[i]=sqrt(sumDist);
  //  printf("THE DISTANCES IS :  %lf \n" , localDist->distances[i]);
    sumDist =0;
  }
return NULL;
}

void printSubTree(double *XSubTree ,int Counter , int d){
  if (Counter == 0) {
    printf("  NULL\n");
  }
  else {
    for (int i = 0; i < Counter; i++) {
      for (int j = 0; j < d; j++) {
        printf("  %8.6lf ", *(XSubTree + i * d + j));
      }
      printf("\n");
    }
  }
}

void printFam(double *X,int *idx , double * Xinner ,int  numberOfInner ,double * Xouter ,int  numberOfOuter ,double * distance ,int  n ,int  d ,int idxVp , double median){
  printf("NEW vptree NODE\n----------------\n\n");
  for (int i = 0; i < n; i++) {
    printf("POINT NO.%d: (", idx[i]);
    for (int j = 0; j < d; j++) {
      printf("%lf, ", *(X + i * d + j));
    }
    if (i < n - 1) {
      printf("), Distance from Vantage Point: %lf \n", distance[i]);
    } else {
      printf("), VANTAGE POINT\n");
    }
  }
  printf("MEDIAN : %lf \n\n", median);
  printf("THE VANTAGE POINT INDEX IS: %d\n",idxVp);

  printf("Xinner ---->  /n");
  printSubTree(Xinner , numberOfInner, d);
  printf("Xouter --->  /n");
  printSubTree(Xouter , numberOfOuter , d);

}
