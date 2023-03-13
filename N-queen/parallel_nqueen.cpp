#include <bits/stdc++.h>
#include <omp.h>

using namespace std;

int n, ans;
bitset<30> rw, ld, rd;  // row, left&right digaonal
vector<string> grid;   // string vector for printing
bool first_found=false;

double start_time;

void printAns() {
		FILE *fp;
		fp=fopen("output_nqueen.txt", "w");
		for(int i=0; i<n; ++i) {
				//cout << grid[i] << "\n";
				fprintf(fp, "%s\n", grid[i].c_str());
		}
		fclose(fp);
}

void solve_nqueen(int col) {
		int rank=omp_get_thread_num(); /* MPI_Comm_rank */
		int size=omp_get_num_threads();  /* MPI_Comm_size */
		printf("#number of process %d (rank %d)\n", size, rank);

		if(col==n) {
				++ans;
				if(!first_found) {
						first_found=true;
						printAns();  // print 1st answer into output file
				}
				return;
		} 
		// fixed row 0 with col in order to split working in parallel
		for(int row=0; row<n; ++row) {
				if(!rw[row] && !ld[row-col+n-1] && !rd[row+col]) {  // row-col can be negative (add offset of n-1)
						grid[row][col]='Q';  // place queen here
						rw[row] = ld[row-col+n-1] = rd[row+col] = true;  // indices for two diagonal axes
						#pragma omp parallel
						#pragma omp single nowait
						{
								#pragma omp task shared(ans)
								solve_nqueen(col+1);
						}
						grid[row][col]='.';  // unplace queen
						rw[row] = ld[row-col+n-1] = rd[row+col] = false;
				}
		}
}

int main() {
		cin >> n;
		for(int i=0; i<n; ++i) {
				string s;
				for(int j=0; j<n; ++j) s+=".";
				grid.push_back(s);
		}
		omp_set_num_threads(3);
		start_time=omp_get_wtime();

		solve_nqueen(0);  // start from coloumn 0

		printf("Time = %lf secs\n", omp_get_wtime()-start_time);
		cout << "Answer of " << n << "x" << n << " chessboard, no. of queen is " << ans << " way(s)\n";
		return 0;
}
