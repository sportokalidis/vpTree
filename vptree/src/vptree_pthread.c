#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
//#include "quickselect.h"
//subnodes
#define MAX_THREADS 6
//distance calculation
#define NOTHREADS 4

// #define N 1000000
// #define D 2

typedef struct vptree {
  double *vp; //vantage
  double md; //median distance
  int idxVp; //the index of the vantage point in the original set
  struct vptree * inner;
  struct vptree * outer;
} vptree;

typedef struct param {
  double * data;
  int * idx;
  int n;
  int d;
  double *distances;
} param;



pthread_mutex_t mux;
pthread_attr_t attr;
volatile int activeThreads = 0;



volatile int threadCounter = -1;
pthread_mutex_t mux;
pthread_mutex_t mux1;
pthread_attr_t attr;

double * generate_points(int n, int d) {

  srand(time(NULL));

  double * points = (double * ) malloc(n * d * sizeof(double));

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < d; j++) {
      *(points + i * d + j) = (double) rand() / RAND_MAX;
    }
  }
  return points;
}








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
  if (threadCounter==-1 || threadCounter==NOTHREADS-1){
    threadCounter=0;
    localThreadCounter = threadCounter;
  }
  else if(threadCounter<NOTHREADS-1){
    threadCounter++;
    localThreadCounter = threadCounter;
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
      sumDist= sumDist + (*(localDist->data + (localDist->n)*localDist->d + j) - *(localDist->data + i*localDist->d + j)) * (*(localDist->data + (localDist->n)*localDist->d + j) - *(localDist->data + i*localDist->d + j));
    }
    localDist->distances[i]=sqrt(sumDist);
  //  printf("THE DISTANCES IS :  %lf \n" , localDist->distances[i]);
    sumDist =0;
  }

}

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

  x->distances = (double * )calloc(x->n - 1, sizeof(double));


  pthread_t th1,th2;
  pthread_t thr[NOTHREADS];
  //pthread_t thr[NOTHREADS];
  void * r1, * r2;
  param a, b;
  //a = (param *) malloc(sizeof(param));
  //b = (param *) malloc(sizeof(param));
  int parallel = 0;


  int numberOfOuter = 0;
  int numberOfInner = 0;
  double median;
  double * Xinner = NULL;
  double * Xouter = NULL;
  int * idxInner = NULL;
  int * idxOuter = NULL;


  p->vp = (double * ) malloc(x->d * sizeof(double));
  for (int j = 0; j < x->d; j++) {
    p->vp[j] = * (x->data+ (x->n-1) * x->d + j);
  }
  p->idxVp = x->idx[x->n-1];



//THRESHOLD TO GO SERIAL
   if(x->n<300000){
     for(int i = 0; i < x->n-1; i++){
       x->distances[i]=distanceCalculation((x->data + i * x->d),p->vp,x->n,x->d);
    }
  }
    else{
      //PTHREADS
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
  // printf("NEW vptree NODE\n----------------\n\n");
  // for (int i = 0; i < n; i++) {
  //   printf("POINT NO.%d: (", idx[i]);
  //   for (int j = 0; j < d; j++) {
  //     printf("%lf, ", *(X + i * d + j));
  //   }
  //   if (i < n - 1) {
  //     printf("), Distance from Vantage Point: %lf\n", x->distances[i]);
  //   } else {
  //     printf("), VANTAGE POINT\n");
  //   }
  // }
  // printf("MEDIAN : %lf \n\n", median);
  // printf("->XINNER  :\n");
  // for (int i = 0; i < inCounter; i++) {
  //   for (int j = 0; j < d; j++) {
  //     printf("  %8.6lf ", *(Xinner + i * d + j));
  //   }
  // printf("\n");
  // }
  // //printf("\n\n--------------\nTHREADS :: %d\n--------------\n\n",activeThreads);
  // printf("->XOUTER  :\n");
  // if (outCounter == 0) {
  //   printf("  NULL\n");
  // } else
  // for (int i = 0; i < outCounter; i++) {
  //   for (int j = 0; j < d; j++) {
  //     printf("  %8.6lf ", *(Xouter + i * d + j));
  //   }
  //   printf("\n");
  // }
  // printf("\n");
  // pthread_mutex_unlock(&mux);

  if (activeThreads < MAX_THREADS) {
    pthread_mutex_lock (&mux);
    activeThreads += 2;
    pthread_mutex_unlock (&mux);
    a.data = Xinner;
    a.idx = idxInner;
    a.n = numberOfInner;
    a.d = x->d;
    pthread_create( &th1, &attr, recBuild, (void*)(&a));
    b.data = Xouter;
    b.idx = idxOuter;
    b.n = numberOfOuter;
    b.d = x->d;
    pthread_create( &th2, &attr, recBuild, (void *)(&b));
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
    if (Xinner != NULL) {
      a.data = Xinner;
      a.n = numberOfInner;
      a.d = x->d;
      a.idx = idxInner;
      p->inner = (vptree *)recBuild((void *) &a);
    }
    if (Xouter != NULL) {
      b.data = Xouter;
      b.n = numberOfOuter;
      b.d = x->d;
      b.idx = idxOuter;
      p->outer = (vptree *)recBuild((void *) &b);
    }
  }

  // free(a);
  // free(b);
  // free(idxInner);
  // free(idxOuter);
  // free(Xinner);
  // free(Xouter);
  return (void *)p;
}


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
