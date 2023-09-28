# Parallel implemetation of vpTree

## Description

This project aims to implement and parallelize the construction of a vantage-point tree, a data structure used for k-nearest neighbor (kNN) search, in the C programming language. 

<br>

The project is divided into several tasks:

<br>


#### **1. Implement Vantage-Point Tree in C**

Sequential implementation of vpTree in C.

<br>

#### **2. Parallelization with Pthreads:** 


In this step, the implementation  is parallelized using Pthreads (POSIX Threads). There are two primary areas for parallelization: <br>

- Computing distances between points in parallel.<br>
- Computing the inner and outer sets in parallel.

<br>

#### **3. Thresholding Parallel Calls:**

The code was modified to switch to sequential execution for small workloads and restrict the maximum number of live threads to optimize performance. 

<br>


#### **4. Reimplementation with OpenMP and Cilk**

In the final step, the project explores high-level parallelism expressions using Cilk and OpenMP.
