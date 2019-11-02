#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>


#include <omp.h>

// #define N 20
// #define D 2


typedef struct vptree {
  double * vp; //vantage
  double md; //median distance
  int idxVp; //the index of the vantage point in the original set
  struct vptree * inner;
  struct vptree * outer;
} vptree;



double * generate_points(int n, int d) {

  srand(time(NULL));


  double * points = (double * ) malloc(n * d * sizeof(double));

  for (int i = 0; i < n*d; i++) {
      points[i] = (double) rand() / RAND_MAX;
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





















double  distanceCalculation(double * X, double * vp, int n, int d) {
    double temp = 0;
    for (int i = 0; i < d; i++){
      temp += (X[i] - vp[i])*(X[i] - vp[i]);
    }
    return sqrt(temp);
}

void setTree(vptree *tree , double * X , int *idx ,  int n , int d ){
  tree->vp = (double * ) malloc(d * sizeof(double));
  for (int j = 0; j < d; j++) {
    tree->vp[j] = * (X + (n-1) * d + j);
  }
  tree->idxVp = idx[n-1];
}


void printSubTree(double *XSubTree ,int Counter , int d){
  if (Counter == 0) {
    printf("  NULL\n");
  } else
  for (int i = 0; i < Counter; i++) {
    for (int j = 0; j < d; j++) {
      printf("  %8.6lf ", *(XSubTree + i * d + j));
    }
    printf("\n");
  }
}

void createNewX(double * Xinner , double * Xouter , double * X ,int *idx ,  int *innerIdx , int *outerIdx , int n , int d , double * distance , double median){

  int inCounter = 0; //number of Inner points//
  int outCounter = 0; //number of Outer points//
  for (int i = 0; i < n - 1; i++) {
    if (distance[i] <= median) {
      for (int j = 0; j < d; j++) {
        *(Xinner + inCounter * d + j) = * (X + i * d + j);
      }
      innerIdx[inCounter]=idx[i];
      inCounter++;
    } else {
      for (int j = 0; j < d; j++) {
        *(Xouter + outCounter * d + j) = * (X + i * d + j);
      }
      outerIdx[outCounter]=idx[i];
      outCounter++;
    }
  }
}


vptree * recBuild(double * X, int * idx, int n, int d) {

  vptree *p = (vptree * ) malloc(sizeof(vptree));
  int numberOfOuter = 0;
  int numberOfInner = 0;
  double  *distance = (double * ) calloc(n - 1, sizeof(double));

  double median;
  double * Xinner = NULL;
  double * Xouter = NULL;
  int * innerIdx = NULL;
  int * outerIdx = NULL;

  if (n == 1){
    p->vp=X;
    p->idxVp=idx[0];
    p->md=0;
    p->inner=NULL;
    p->outer=NULL;
    return p;
  }
  if(n == 0)
    return NULL;

  setTree(p,X,idx,n,d);

#pragma omp parallel for  schedule(static,4) num_threads (2)//schedule(static,250000) num_threads(8)
  for (int i = 0; i < n-1; i++)
  {
    distance[i]=distanceCalculation((X + i * d),p->vp,n,d);
  }
  // distance = distanceCalculation(X, p->vp, n, d);
  median = qselect(distance, n - 1, (int)((n - 2) / 2));

  p->md = median;


  numberOfOuter = (int)((n - 1) / 2);
  numberOfInner = n - 1 - numberOfOuter;

  if (numberOfInner != 0) {
    Xinner = (double * ) malloc(numberOfInner * d * sizeof(double));
    innerIdx = (int *) malloc(numberOfInner * sizeof(int));
  }
  if (numberOfOuter != 0) {
    Xouter = (double * ) malloc(numberOfOuter * d * sizeof(double));
    outerIdx = (int *) malloc(numberOfOuter * sizeof(int));
  }

  createNewX( Xinner , Xouter , X , idx ,  innerIdx , outerIdx ,n , d , distance , median);
//
//   printf("NEW vptree NODE\n----------------\n\n");
//   for (int i = 0; i < n; i++) {
//     printf("POINT NO.%d: (", idx[i]);
//     for (int j = 0; j < d; j++) {
//       printf("%lf, ", *(X + i * d + j));
//     }
//     if (i < n - 1) {
//       printf("), Distance from Vantage Point: %lf \n", distance[i]);
//     } else {
//       printf("), VANTAGE POINT\n");
//     }
//   }
//   printf("MEDIAN : %lf \n\n", median);
//   printf("THE VANTAGE POINT INDEX IS: %d\n",p->idxVp);
//
// printf("->XINNER  :\n");
// printSubTree(Xinner , numberOfInner, d);
// printf("->XOUTER  :\n");
// printSubTree(Xouter , numberOfOuter , d);


//TO BE PARALLEL
// #pragma omp task
// p->inner =  recBuild(Xinner, innerIdx, numberOfInner, d);
// p->outer =  recBuild(Xouter, outerIdx, numberOfOuter, d);
// #pragma omp taskwait


  #pragma omp task shared(p)
    p->inner =  recBuild(Xinner, innerIdx, numberOfInner, d);

  #pragma omp task shared(p)
    p->outer =  recBuild(Xouter, outerIdx, numberOfOuter, d);

  return p;
}


vptree * buildvp(double * X, int n, int d) {

  int * idx = (int *) malloc(n * sizeof(int));
   for(int i=0; i<n; i++){
     idx[i] = i;
   }
   return recBuild(X, idx, n, d);
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
