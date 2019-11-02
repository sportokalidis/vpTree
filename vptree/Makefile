CC=gcc
CFLAGS= -O3 -fopenmp -fcilkplus -Wall -std=c99 -pthread -lm
SDIR=./src
LDIR=./lib
IDIR=./inc
INCLUDES = inc/vptree.h inc/quickselect.h
LIBS= $(LDIR)/vptree_sequential.a $(LDIR)/vptree_pthread.a $(LDIR)/vptree_openmp.a $(LDIR)/vptree_cilk.a
EXE= $(SDIR)/vptree_sequential $(SDIR)/vptree_pthread $(SDIR)/vptree_openmp $(SDIR)/vptree_cilk
MAIN= main
LDFLAGS = -Iinc



all: $(LIBS) $(EXE)
lib: $(LIBS)



$(SDIR)/vptree_sequential.o: $(SDIR)/vptree_sequential.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

$(SDIR)/vptree_pthread.o: $(SDIR)/vptree_pthread.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

$(SDIR)/vptree_openmp.o: $(SDIR)/vptree_openmp.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

$(SDIR)/vptree_cilk.o: $(SDIR)/vptree_cilk.c
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)



$(LDIR)/vptree_sequential.a: $(SDIR)/vptree_sequential.o
	ar rcs $@ $< $(INCLUDES)

$(LDIR)/vptree_pthread.a: $(SDIR)/vptree_pthread.o
	ar rcs $@ $<  $(INCLUDES)

$(LDIR)/vptree_openmp.a: $(SDIR)/vptree_openmp.o
	ar rcs $@ $< $(INCLUDES)

$(LDIR)/vptree_cilk.a: $(SDIR)/vptree_cilk.o
	ar rcs $@ $< $(INCLUDES)



$(SDIR)/vptree_sequential: $(SDIR)/$(MAIN).c $(LDIR)/vptree_sequential.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(SDIR)/vptree_pthread: $(SDIR)/$(MAIN).c $(LDIR)/vptree_pthread.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^  $(LDFLAGS)

$(SDIR)/vptree_openmp: $(SDIR)/$(MAIN).c $(LDIR)/vptree_openmp.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

$(SDIR)/vptree_cilk: $(SDIR)/$(MAIN).c $(LDIR)/vptree_cilk.a
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^  $(LDFLAGS)



clean:
	$(RM) $(SDIR)/*.o *~ $(LDIR)/vptree_*.a $(EXE)
