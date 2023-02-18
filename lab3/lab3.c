/* 63070503426_lab3.c
 * Author: Panithi Suwanno
 * Date: 16 Feb 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
	int size, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	printf("#number of process = %d (rank %d)\n", size, rank);

	int row1, col1, row2, col2;  /* number of row and column for matrix 1 and 2 */
	int **matrixA, **matrixB, *answer;  /* 2D array for matrix 1, 2 and result */

	FILE *fp1, *fp2;  /* file pointer */

	/* Read matrix data only in rank 0 so as not to waste run time. */
	if(rank==0) {
		/* start reading file here */
		//fp1=fopen("matAsmall.txt", "r");
		//fp2=fopen("matBsmall.txt", "r");
		fp2=fopen("matBlarge.txt", "r");
		fp1=fopen("matAlarge.txt", "r");
		if(fp1==NULL) {
			printf("cannot open file 1\n");
			exit(0);
		}
		if(fp2==NULL) {
			printf("cannot open file 2\n");
			exit(1);
		}
		fscanf(fp1, "%d %d", &row1, &col1);
		fscanf(fp2, "%d %d", &row2, &col2);
		if(col1!=row2) {
			printf("The size of the matrix cannot be multiplied");
			exit(2);
		}

		/* allocating the matrix space */
		matrixA=(int **)calloc(row1, sizeof(int *));  /* dynamic memory allocation */
		for(int i=0; i<row1; ++i) 
			matrixA[i]=(int *)calloc(col1, sizeof(int));
		matrixB=(int **)calloc(row2, sizeof(int *));  /* dynamic memory allocation */
		for(int i=0; i<row2; ++i) 
			matrixB[i]=(int *)calloc(col2, sizeof(int));
		/* allocating answer matrix */
		answer=(int *)calloc(row1*col2, sizeof(int));  /* dynamic memory allocation */
		if(answer==NULL) {
			printf("Answer Matrix memory leaked\n");
			exit(5);
		}

		printf("reading file 1...\n");
		if(matrixA==NULL) {
			printf("Matrix A memory leaked\n");
			exit(3);
		}
		else {
			for(int i=0; i<row1; ++i)
				for(int j=0; j<col1; ++j)
					fscanf(fp1, "%d", &matrixA[i][j]);
			printf("Sucessfully read file 1!\n");
		}

		printf("reading file 2...\n");
		if(matrixB==NULL) {
			printf("Matrix B memory leaked\n");
			exit(4);
		}
		else {
			for(int i=0; i<row2; ++i)
				for(int j=0; j<col2; ++j)
					fscanf(fp2, "%d", &matrixB[i][j]);
			printf("Sucessfully read file 2!\n");
		}
		fclose(fp1);
		fclose(fp2);
		/* finished reading file */
	}
	
	/* start broadcasting */
	MPI_Bcast(&row1, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&col1, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&row2, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&col2, 1, MPI_INT, 0, MPI_COMM_WORLD);
	if(rank!=0) {
		//printf("%d %d %d %d\n", row1, col1, row2, col2);
		matrixA=(int **)calloc(row1, sizeof(int *));  /* dynamic memory allocation for other ranks*/
		for(int i=0; i<row1; ++i) 
			matrixA[i]=(int *)calloc(col1, sizeof(int));
		matrixB=(int **)calloc(row2, sizeof(int *));  /* dynamic memory allocation for other ranks*/
		for(int i=0; i<row2; ++i) 
			matrixB[i]=(int *)calloc(col2, sizeof(int));
	}
	for(int i=0; i<row1; ++i)
		MPI_Bcast(matrixA[i], col1, MPI_INT, 0, MPI_COMM_WORLD);
	for(int i=0; i<row2; ++i)
		MPI_Bcast(matrixB[i], col2, MPI_INT, 0, MPI_COMM_WORLD);
	printf("Rank %d received broadcast!\n", rank);

	if(rank==0) {
		double startTime=MPI_Wtime(), endTime;
		int sep_row=row1/size;  /* separate row for each process */
		int rest_row=row1%size;  /* for indivisible number */
		printf("sep = %d, rest = %d\n", sep_row, rest_row);

		int sum;
		/* compute matrix for rank 0 */
		for(int i=0; i<sep_row; ++i) {
			for(int j=0; j<col2; ++j) {
				sum=0;
				for(int k=0; k<row2; ++k)
					sum+=matrixA[i][k]*matrixB[k][j];
				answer[(i*col2)+j]=sum;
			}
		}

		/* send to other ranks */
		printf("\tWelcome rank 0\n");

		/* have fraction because matrix_size%n_process !=0  */
		if(rest_row) {
			for(int i=sep_row*size; i<(sep_row*size)+rest_row; ++i) {
				for(int j=0; j<col2; ++j) {
					sum=0;
					for(int k=0; k<row2; ++k)
						sum+=matrixA[i][k]*matrixB[k][j];
					answer[(i*col2)+j]=sum;
				}
			}
		}

		/* receive the rest from rank 1->n (rank 0 is computed above) */
		for(int r=1; r<size; ++r) {
			MPI_Recv(&answer[r*col2*sep_row], sep_row*col2, MPI_INT, r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		endTime=MPI_Wtime();

		/* write output file */
		FILE* fp3;
		//fp3=fopen("small_3426.txt", "w");
		fp3=fopen("large_3426.txt", "w");
		if(NULL==fp3) {
			printf("can't open file for writing\n");
			exit(6);
		}
		fprintf(fp3, "%d %d\n", row1, col2);
		for(int i=0; i<row1; ++i) {
			for(int j=0; j<col2; ++j)
				fprintf(fp3, "%d ", answer[(i*col2)+j]);
			fprintf(fp3, "\n");
		}
		fclose(fp3);
		printf("Time = %lf sec\n", endTime-startTime);
	}
	else {
		int sep_row=row1/size;
		printf("sep_row=%d col=%d (rank=%d)\n", sep_row, col2, rank);

		/* start allocate matrix from calculated size */
		int *result=(int *)calloc(sep_row*col2, sizeof(int));

		int sum;
		printf("Start multiplication in rank %d\n", rank);
		/* perform calculations based on the given rows. */
		int cnt=0;
		for(int i=sep_row*rank; i<sep_row*(rank+1); ++i) {
			for(int j=0; j<col2; ++j) {
				sum=0;
				for(int k=0; k<row2; ++k)
					sum+=matrixA[i][k]*matrixB[k][j];
				result[((i-sep_row*rank)*col2)+j]=sum;
				++cnt;
			}
		}
		//printf("%d %d %d\n", cnt, sep_row*row2, sep_row*col2);
		printf("Complete multiplication in rank %d\n", rank);
		/* send result back to rank 0 */
		MPI_Send(&result[0], sep_row*col2, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
