/* 63070503426_pj1.c
 * Author: Panithi Suwanno
 * Date: 11 Feb 2023
 * To run: ./a.out [number of thread] [filename] or ./a.out | 3 arguments or 1 argument only (set default)
 * Ex.: ./pj1 8 testcase.txt or ./pj1
 */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int *arr;

void swap(int* a, int* b) { int tmp=*a; *a=*b; *b=tmp; }

int partition(int arr[], int low, int high) {
    int pivot=arr[high], smaller=low-1; // Index of smaller element and indicates
    for (int i=low; i<high; ++i) {
        if (arr[i]<pivot) {  // If current element is smaller than the pivot
            ++smaller;  // increment index of smaller element
            swap(&arr[smaller], &arr[i]);
        }
    }
    swap(&arr[smaller+1], &arr[high]);
    return smaller+1;
}
  
void quick_sort(int arr[], int low, int high) {
    if (low<high) {
        int ind=partition(arr, low, high);
		#pragma omp task firstprivate (arr, low, high, ind) // each threads has its own copy
		{
        	quick_sort(arr, low, ind-1);
        	quick_sort(arr, ind+1, high);
		}
    }
}

int main(int argc, char **argv) {
	printf("Initialize array!\n");
	/* default initializing */
	arr=(int *)malloc(20 * sizeof(int));
	int n=sizeof(arr)/sizeof(arr[0]);  /* default array size */
	for(int i=0; i<n; ++i) {
		arr[i]=rand()%100;
		//printf("%d ", arr[i]);
	}
	//printf("\n");

	FILE *fp1, *fp2;  /* file pointer */
	if(argc>2) {  /* user input filename */
		fp1=fopen(argv[2], "r");
		fscanf(fp1, "%d", &n);
		arr=(int *)malloc(n * sizeof(int));
		for(int i=0; i<n; ++i) {
			fscanf(fp1, "%d", &arr[i]);
		}
		fclose(fp1);
	}

	int nthread=1; /* set number of thread to 1 by default */
	if(argc>1) {
		nthread=atoi(argv[1]);
		printf("set nthread=%d\n", nthread);
	}
	omp_set_num_threads(nthread);  /* Specify a number of threads needed in the computation */
	double start_time=omp_get_wtime();
	#pragma omp parallel
	{
		int rank=omp_get_thread_num(); /* MPI_Comm_rank */
		int size=omp_get_num_threads();  /* MPI_Comm_size */
		//printf("#number of process %d (rank %d)\n", size, rank);
		#pragma omp single nowait
		quick_sort(arr, 0, n-1);
	}

	printf("Time = %lf secs\n", omp_get_wtime()-start_time);

	fp2=fopen("output_quicksort.txt", "w");
	fprintf(fp2, "%d\n", n);
	printf("Sorted array (size %d)\n", n);
	for(int i=0; i<n; ++i) {
		//printf("%d ", arr[i]);
		fprintf(fp2, "%d ", arr[i]);
	}
	//printf("\n");
	fprintf(fp2, "\n");
	fclose(fp2);
	return 0;
}
