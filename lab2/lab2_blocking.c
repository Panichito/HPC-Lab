/* 63070503426_lab2_blocking.c
 * Author: Panithi Suwanno 63070503426
 * Date: 06 Feb 2023 - 10 Feb 2023 */
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	int nprocs, myrank;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	printf("#number of process = %d\n", nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

	if(myrank==0) {
			int row1, col1, row2, col2;  /* number of row and column for matrix 1 and 2 (must be equal) */
			double **arr1, **arr2, **answer_blocking;  /* 2D array for matrix 1, 2 and result */

			/* start readding file here (only rank 0) */
			FILE* fp1;  /* first file pointer */
			fp1=fopen("matAsmall.txt", "r");
			//fp1=fopen("matAlarge.txt", "r");
			if(NULL==fp1) {
					printf("can't open\n");
			}
			printf("opening file...\n");
			fscanf(fp1, "%d %d", &row1, &col1);

			/* dynamic array allocation */
			arr1=(double **)calloc(row1, sizeof(double *));
			arr2=(double **)calloc(row1, sizeof(double *));
			answer_blocking=(double **)calloc(row1, sizeof(double *));
			for(int i=0; i<row1; ++i) {
					arr1[i]=(double *)calloc(col1, sizeof(double));
					arr2[i]=(double *)calloc(col1, sizeof(double));
					answer_blocking[i]=(double *)calloc(col1, sizeof(double));
			}

			if(arr1==NULL) {
					printf("Leaked\n");
					exit(1);
			}
			else {
					for(int i=0; i<row1; ++i) {
							for(int j=0; j<col1; ++j) {
									fscanf(fp1, "%lf", &arr1[i][j]);
							}
					}
					printf("Reading file 1 success!\n");
			}
			fclose(fp1);

			FILE* fp2;  /* second file pointer */
			fp2=fopen("matBsmall.txt", "r");
			//fp2=fopen("matBlarge.txt", "r");
			fscanf(fp2, "%d %d", &row2, &col2);
			if(arr2==NULL) {
					printf("Leaked\n");
					exit(1);
			}
			else {
					for(int i=0; i<row2; ++i) {
							for(int j=0; j<col2; ++j) {
									fscanf(fp2, "%lf", &arr2[i][j]);
							}
					}
					printf("Reading file 2 success!\n");
			}
			fclose(fp2);
			/* finished readding file */

			double startTime=MPI_Wtime();
			int sep_row=row1/nprocs;  /* separate row for each process */
			int rest_row=row1%nprocs;  /* for indivisible number */
			printf("sep = %d, rest = %d\n", sep_row, rest_row);

			/* compute matrix for rank 0 */
			for(int i=0; i<sep_row; ++i) {
					for(int j=0; j<col1; ++j) {
							answer_blocking[i][j]=arr1[i][j]+arr2[i][j];
					}
			}

			/* send to other ranks */
			if(nprocs>1) {
					printf("\tWelcome to Rank 0\n");
					/* MPI_Send
						void *data, int count, MPI_Datatype datatype, int destination, int tag, MPI_Comm communicator */
					for(int r=1; r<nprocs; ++r) {
							MPI_Send(&sep_row, 1, MPI_INT, r, 1, MPI_COMM_WORLD);
							MPI_Send(&col1, 1, MPI_INT, r, 2, MPI_COMM_WORLD);
					}

					for(int r=1; r<nprocs; ++r) {
							for(int i=sep_row*r; i<sep_row*(r+1); ++i) {
									for(int j=0; j<col1; ++j) {
											MPI_Send(&arr1[i][j], 1, MPI_DOUBLE, r, 3, MPI_COMM_WORLD);
											MPI_Send(&arr2[i][j], 1, MPI_DOUBLE, r, 4, MPI_COMM_WORLD);
									}
							}
					}

					/* have fraction because matrix_size%n_process !=0  */
					if(rest_row) {
							/* compute last sep row at rank 0 (here) */
							for(int i=sep_row*nprocs; i<(sep_row*nprocs)+rest_row; ++i) {
									for(int j=0; j<col1; ++j) {
											answer_blocking[i][j]=arr1[i][j]+arr2[i][j];
									}
							}
					}

					/* receive the rest from rank 1->n (rank 0 is computed above) */
					for(int r=1; r<nprocs; ++r) {
							for(int i=sep_row*r; i<sep_row*(r+1); ++i) {
									for(int j=0; j<col1; ++j) {
											MPI_Recv(&answer_blocking[i][j], 1, MPI_DOUBLE, r, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
									}
							}
					}
			}
			double endTime=MPI_Wtime();

			/* write output file */
			FILE* fp3;
			fp3=fopen("blocksmallresult_3426.txt", "w");
			//fp3=fopen("blocklargeresult_3426.txt", "w");
			if(NULL==fp3) {
					printf("can't open file for writing\n");
			}
			fprintf(fp3, "%d %d\n", row1, col1);
			for(int i=0; i<row1; ++i) {
					for(int j=0; j<col1; ++j) {
							fprintf(fp3, "%lf ", answer_blocking[i][j]);
					}
					fprintf(fp3, "\n");
			}
			fclose(fp3);
			printf("Time = %lf sec\n", endTime-startTime);
	}
	else {
			int recv_row, recv_col;
			/* MPI_Recv
				void *data, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm communicator, MPI_Status *status */
			MPI_Recv(&recv_row, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(&recv_col, 1, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("recv row=%d col=%d (rank=%d)\n", recv_row, recv_col, myrank);

			/* start allocate matrix from received size */
			double **recv_arr1, **recv_arr2, **result;
			recv_arr1=(double **)calloc(recv_row, sizeof(double *));
			for(int i=0; i<recv_row; ++i) {
					recv_arr1[i]=(double *)calloc(recv_col, sizeof(double));
			}
			recv_arr2=(double **)calloc(recv_row, sizeof(double *));
			for(int i=0; i<recv_row; ++i) {
					recv_arr2[i]=(double *)calloc(recv_col, sizeof(double));
			}
			result=(double **)calloc(recv_row, sizeof(double *));
			for(int i=0; i<recv_row; ++i) {
					result[i]=(double *)calloc(recv_col, sizeof(double));
			}
			/* end of matrix allocation (for rank1->n) */

			/* receive matrix data */
			for(int i=0; i<recv_row; ++i) {
					for(int j=0; j<recv_col; ++j) {
							MPI_Recv(&recv_arr1[i][j], 1, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
							MPI_Recv(&recv_arr2[i][j], 1, MPI_DOUBLE, 0, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
							result[i][j]=recv_arr1[i][j]+recv_arr2[i][j];  /* compute here */
					}
			}
			printf("Finished receive matrix.\n");

			/* send computed matrix */
			for(int i=0; i<recv_row; ++i) {
					for(int j=0; j<recv_col; ++j) {
							MPI_Send(&result[i][j], 1, MPI_DOUBLE, 0, 5, MPI_COMM_WORLD);
					}
			}
			printf("Send matrix back!\n");
	}

	MPI_Finalize();
	return 0;
}
