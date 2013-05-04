#ifndef EYELINECV_HPP
#define EYELINECV_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <algorithm>
using namespace std;
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#define MIN(A,B) ((A) < (B) ? (A) : (B))
#define PI  3.1416
typedef struct
{
	float Orientation;
	//int PixelList;
	vector<int> placeX;
	vector<int> placeY;
	vector<int> boundX;
	vector<int> boundY;
	vector<int> PerRecordX;
	vector<int> PerRecordY;
	vector<float> diff2pow;
	int numberOfElement;
	float Perimeter;
	float Centroid[2];
	float BoundingBox[4];
	float hypo;//calculate the perimeter when meet the Hypotenuse plus 1
}OBJECT;
typedef struct
{
	short int vert;
	short int horiz;
} offsets;
int Round(float);
int bwLabel(vector<vector<int> > &,int,int,int,vector<OBJECT> &);
void flood4(int, int, int,int,int,int,vector<vector<int> > &,vector<vector<bool> > &,OBJECT &);
void flood8(int, int, int,int,int,int,vector<vector<int> > &,vector<vector<bool> > &,OBJECT &);
void Perimeter(vector<vector<int> > &,int,int,vector<OBJECT> & );
void boundingbox(vector<OBJECT> &);
bool pointConfirm(int &,int &,OBJECT &); //confirm the point
void centroid(vector<OBJECT> &);		 //calculate regionprops centroid
void orientation(vector<OBJECT> &);      //calculate regionprops orientation
void polyval(double*,double*,vector<double> &,int,int);
bool convolve2D(float*, float*, int, int, float*, int, int);
// polyfit c code
char **fisCreateMatrix(int row_n, int col_n, int element_size);



/* matrix addition */
template <class T>
void matAdd(T **m1, T **m2, int row, int col, T **out){
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++)
			out[i][j] = m1[i][j] + m2[i][j];
}

/* matrix subtraction */
template <class T>
void matSubtract(T **m1, T **m2, int row, int col, T **out){
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++)
			out[i][j] = m1[i][j] - m2[i][j];
}

/* matrix mutliplication */
template <class T>
void matMultiply(T **m1, T **m2, int row1, int col1, int col2, T **out){
	int i, j, k;
	for (i = 0; i < row1; i++)
		for (j = 0; j < col2; j++) {
			out[i][j] = 0;
			for (k = 0; k < col1; k++)
				out[i][j] += m1[i][k]* m2[k][j];
		}
}

/* scalar times matrix */
template <class T>
void matMultiplyScalar(T c, T **m, int row, int col, T **out){
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++)
			out[i][j] = c*m[i][j];
}

/* matrix transpose */
template <class T>
void matTranspose(T **m, int row, int col, T **m_t){
	int i, j;
	for (i = 0; i < row; i++)
		for (j = 0; j < col; j++)
			m_t[j][i] = m[i][j];
}

/* Recursive least-squares estimate (a single update) */
template <class T>
void rlseOneUpdate(T *observation, int inNum, T *targetOutput, int outNum, T lambda, T *extP ,T **S,T **P,int *flag){
	int i, j;
	T denom;
	/* Create necessary matrices */
	T **a = (T **)fisCreateMatrix(inNum, 1, sizeof(T));
	T **b = (T **)fisCreateMatrix(outNum, 1, sizeof(T));
	T **a_t = (T **)fisCreateMatrix(1, inNum, sizeof(T));
	T **b_t = (T **)fisCreateMatrix(1, outNum, sizeof(T));
	T **tmp1 = (T **)fisCreateMatrix(inNum, 1, sizeof(T));
	T **tmp2 = (T **)fisCreateMatrix(1, 1, sizeof(T));
	T **tmp3 = (T **)fisCreateMatrix(1, inNum, sizeof(T));
	T **tmp4 = (T **)fisCreateMatrix(inNum, inNum, sizeof(T));
	T **tmp5 = (T **)fisCreateMatrix(1, outNum, sizeof(T));
	T **tmp6 = (T **)fisCreateMatrix(inNum, outNum, sizeof(T));
	T **tmp7 = (T **)fisCreateMatrix(inNum, outNum, sizeof(T));
	//printf("flag=%d\n", flag);
	if (*flag==0){	// Initialization of S and P
		T alpha = 1e6;
		for (i = 0; i < inNum; i++)
			for (j = 0; j < outNum; j++)
				P[i][j] = 0;
		for (i = 0; i < inNum; i++)
			for (j = 0; j < inNum; j++)
				if (i == j)
					S[i][j] = alpha; 
				else
					S[i][j] = 0;
	}
	*flag=*flag+1;
//	printf("flag=%d\n", flag);
//	printf("inNum=%d, outNum=%d\n", inNum, outNum);
	
	/* dispatch input and output */
	for (i = 0; i < inNum; i++)
		a[i][0] = observation[i];
	for (i = 0; i < outNum; i++)
		b[i][0] = targetOutput[i];
	matTranspose(a, inNum, 1, a_t);
	matTranspose(b, outNum, 1, b_t);
//	printf("After dispatching input and output!\n");

	/* recursive formulas for S, covariance matrix */
	matMultiply(S, a, inNum, inNum, 1, tmp1);
	matMultiply(a_t, tmp1, 1, inNum, 1, tmp2);
	denom = lambda + tmp2[0][0];
	matMultiply(a_t, S, 1, inNum, inNum, tmp3);
	matMultiply(tmp1, tmp3, inNum, 1, inNum, tmp4);
	matMultiplyScalar(1/denom, tmp4, inNum, inNum, tmp4);
	matSubtract(S, tmp4, inNum, inNum, S);
	matMultiplyScalar(1/lambda, S, inNum, inNum, S);
//	printf("After recursive formula for S\n");

	/* recursive formulas for P, the estimated parameter matrix */
	matMultiply(a_t, P, 1, inNum, outNum, tmp5);
	matSubtract(b_t, tmp5, 1, outNum, tmp5);
	matMultiply(a, tmp5, inNum, 1, outNum, tmp6);
	matMultiply(S, tmp6, inNum, inNum, outNum, tmp7);
	matAdd(P, tmp7, inNum, outNum, P);
//	printf("After recursive formula for P\n");

	/* return the current computed output */
	matMultiply(a_t, P, 1, inNum, outNum, tmp5);
//	printf("Output = %f\n", tmp5[0][0]);

	/* Copy the param to extP */
	if (extP!=NULL)
		for (i=0; i<inNum; i++)
			for (j=0; j<outNum; j++)
				extP[j*inNum+i]=P[i][j];
	/* return the computed output */
	//return(tmp5[0][0]);
	for(int i=0;i<inNum;i++)
	{
		free(tmp1[i]);
		free(tmp4[i]);
		free(tmp6[i]);
		free(tmp7[i]);
		free(a[i]);
	}
	for(int i=0;i<outNum;i++)
	{
		free(b[i]);
	}
	free(tmp2[0]);
	free(tmp3[0]);
	free(tmp5[0]);
	free(a_t[0]);
	free(b_t[0]);
	free(tmp1);
	free(tmp2);
	free(tmp3);
	free(tmp4);
	free(tmp5);
	free(tmp6);
	free(tmp7);
	free(a);
	free(b);
	free(a_t);
	free(b_t);
}


/* Recursive least-squares estimate (batch mode) */
template <class T>
void rlseBatch(T *a, int dataNum, int inNum, T *b, int outNum, T lambda, T *x){
	T *observation =(T *)calloc( inNum, sizeof(T));
	T *targetOutput=(T *)calloc(outNum, sizeof(T));
	T **S = (T **)fisCreateMatrix(inNum, inNum, sizeof(T));
	T **P = (T **)fisCreateMatrix(inNum, outNum, sizeof(T));
	int flag=0;
	for (int i=0; i<dataNum; i++){
		for (int j=0; j<inNum; j++)	// Put each row of a into obesrvation
			observation[j]=a[j*dataNum+i];
		for (int j=0; j<outNum; j++)	// Pur each row of b into targetOutput
			targetOutput[j]=b[j*dataNum+i];
		rlseOneUpdate(observation, inNum, targetOutput, outNum, lambda, x, S, P,  &flag );
	}
	printf("%d\n",flag);
	free(observation);
	free(targetOutput);
	free(S);
	free(P);
}

/* Polynomial fit via RLSE */
template <class T>
void polyfit(T *x, int dataNum, T *y, int order, T *coef){
	T *a =(T *)calloc( dataNum*(order+1), sizeof(T));
	// To pupulate the matrix a
	for (int i=0; i<dataNum; i++)
		for (int j=0; j<order+1; j++)
			a[j*dataNum+i]=pow(x[i], order-j);
	rlseBatch(a, dataNum, order+1, y, 1, 1.0, coef);
	free(a);
}

#endif
