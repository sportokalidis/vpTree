/*
* FILE: vptree_sequential.c
* THMMY, 7th semester, Parallel and Distributed Systems: 1st assignment
* Sequential implementation of vantage point tree
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
#include "vptree.h"

//Function declarations
double qselect(double *v, int len, int k);
double  distanceCalculation(double * X, double * Y, int n, int d);
void setTree(vptree *tree , double * X , int *idx ,  int n , int d );
void printFam(double *X,int *idx , double * Xinner ,int  numberOfInner ,double * Xouter ,
              int  numberOfOuter ,double * distance ,int  n ,int  d ,int idxVp , double median);
void createNewX(double * Xinner , double * Xouter , double * X ,int *idx ,
                int *innerIdx , int *outerIdx , int n , int d , double * distance , double median);


//Recursive build function
vptree * recBuild(double * X, int * idx, int n, int d) {

  vptree *p = (vptree * ) malloc(sizeof(vptree));
  int numberOfOuter = 0; //Number of points in the outer set
  int numberOfInner = 0; //NUmber of points in the inner set
  double median; // The median of the vantage point
  double * Xinner = NULL; //Inner subtree
  double * Xouter = NULL; //Outer subtree
  int * innerIdx = NULL; //Inner indexes
  int * outerIdx = NULL; //Outer indexes

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

  double  *distance = (double * ) calloc(n - 1, sizeof(double));
  setTree(p,X,idx,n,d);

  //Calculating the distance from the vantage point
  for(int i =0; i < n; i++){
    distance[i]=distanceCalculation((X + i * d),p->vp,n,d);
  }

  median = qselect(distance, n - 1, (int)((n - 2) / 2));

  p->md = median;

  numberOfOuter = (int)((n - 1) / 2);
  numberOfInner = n - 1 - numberOfOuter;

//Allocating memory for the subtrees
  if (numberOfInner != 0) {
    Xinner = (double * ) malloc(numberOfInner * d * sizeof(double));
    innerIdx = (int *) malloc(numberOfInner * sizeof(int));
  }
  if (numberOfOuter != 0) {
    Xouter = (double * ) malloc(numberOfOuter * d * sizeof(double));
    outerIdx = (int *) malloc(numberOfOuter * sizeof(int));
  }

  createNewX( Xinner , Xouter , X , idx ,  innerIdx , outerIdx ,n , d , distance , median);

//Helper function to print everything
  //printFam(X, idx ,  Xinner , numberOfInner , Xouter , numberOfOuter ,  distance , n , d ,p->idxVp , median );


  if (Xinner != NULL) {
    p->inner = recBuild(Xinner, innerIdx, numberOfInner, d);
  }
  if (Xouter != NULL) {
    p->outer = recBuild(Xouter, outerIdx, numberOfOuter, d);
  }

  return p;
}

// ======= LIST OF ACCESSORS ======= //
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
