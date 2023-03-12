/* Generate test case program
 * Author: Panithi Suwanno
 * Date: 12 Feb 2023
 * To run: ./a.out [size of wanted array]
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int n=atoi(argv[1]);
	FILE *fp2;
	fp2=fopen("tc.txt", "w");
	fprintf(fp2, "%d\n", n);
	for(int i=0; i<n; ++i) {
		fprintf(fp2, "%d ", (rand()%1000)+1);
	}
	fprintf(fp2, "\n");
	fclose(fp2);
	printf("finish writing %d array into tc.txt!\n", n);
	return 0;
}
