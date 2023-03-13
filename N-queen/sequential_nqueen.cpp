#include <bits/stdc++.h>

using namespace std;

int n, ans;
bitset<30> rw, ld, rd;  // row, left&right digaonal
vector<string> grid;   // string vector for printing
					   //
bool first_found=false;

void printAns() {
	FILE *fp;
	fp=fopen("output_nqueen.txt", "w");
	for(int i=0; i<n; ++i) {
		cout << grid[i] << "\n";
		fprintf(fp, "%s\n", grid[i].c_str());
	}
	fclose(fp);
}

void solve_nqueen(int col) {
	if(col==n) {
		++ans;
		if(!first_found) {
			first_found=true;
			printAns();  // print 1st answer into output file
		}
		return;
	} 
	// abs(row-x) = abs(col-y) cannot be used anymore -> there are two possibilities abs fun: row-col = x-y and row+col = x+y
	for(int row = 0; row < n; ++row) {
		if(!rw[row] && !ld[row-col+n-1] && !rd[row+col]) {  // row-col can be negative (add offset of n-1)
			grid[row][col]='Q';  // place queen here
			rw[row] = ld[row-col+n-1] = rd[row+col] = true;  // indices for two diagonal axes
			solve_nqueen(col+1);
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
	solve_nqueen(0);  // start from coloumn 0
	cout << "Answer of " << n << "x" << n << " chessboard, no. of queen is " << ans << " way(s)\n";
	return 0;
}
