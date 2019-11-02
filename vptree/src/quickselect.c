#include "quickselect.h"

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
