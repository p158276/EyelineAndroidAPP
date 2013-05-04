#include "eyelinecv.hpp"

int Round(float dIn)
{
	return (dIn > 0) ? (int)(dIn + 0.5) : (int)(dIn - 0.5);
}

int bwLabel(vector<vector<int> > &picture,int m,int n,int connType,vector<OBJECT> & component){
	OBJECT BB;
	component.clear();
	int count=0;
	vector<vector<bool> > visit(m, vector<bool>(n));
/*	bool **visit;
	visit = new bool*[m];
	for(int i=0;i<m;i++)
		visit[i] = new bool[n];*/
	for (int i=0; i<m; i++)
		for (int j=0; j<n; j++)
		{
			visit[i][j] = false;
		}
	for (int y=0; y<n; y++)
	{
		for (int x=0; x<m; x++)
			if (picture[x][y] == 1 && !visit[x][y]){
				count++;
				BB.numberOfElement=0; // element initalize
				BB.placeX.clear(); //clean the temp for another object use
				BB.placeY.clear(); //clean the temp for another object use
				if(connType==4)
					flood4(x, y, 1,count,m,n,picture,visit,BB);
				else if (connType==8)
					flood8(x, y, 1,count,m,n,picture,visit,BB);
				component.push_back(BB);//add an component
			}
	}

	return count;
}

void Perimeter(vector<vector<int> > &picture,int m,int n,vector<OBJECT> & component)
{
	offsets move[8];
	move[0].vert = -1; move[0].horiz = 0; //N
	move[1].vert = -1; move[1].horiz = 1; //NE
	move[2].vert = 0; move[2].horiz = 1; //E
	move[3].vert = 1; move[3].horiz = 1; //SE	
	move[4].vert = 1; move[4].horiz = 0; //S
	move[5].vert = 1; move[5].horiz = -1; //SW
	move[6].vert = 0; move[6].horiz = -1; //W
	move[7].vert = -1; move[7].horiz = -1; //NW	

	//compute the boundary of x
	for(int i=0;i<component.size();i++)
	{
		for(int j=0;j<component[i].placeX.size();j++)
		{
			//for accessing array ,we need to change x to y.
			int indexY = component[i].placeX[j];
			int indexX = component[i].placeY[j];
			if(indexX-1<0)
			{
				component[i].boundX.push_back(component[i].placeX[j]);
				component[i].boundY.push_back(component[i].placeY[j]);
				continue;
			}
			if(indexY-1<0)
			{
				component[i].boundX.push_back(component[i].placeX[j]);
				component[i].boundY.push_back(component[i].placeY[j]);
				continue;
			}
			if(indexX+1>m-1)
			{
				component[i].boundX.push_back(component[i].placeX[j]);
				component[i].boundY.push_back(component[i].placeY[j]);
				continue;
			}
			if(indexY+1>n-1)
			{
				component[i].boundX.push_back(component[i].placeX[j]);
				component[i].boundY.push_back(component[i].placeY[j]);
				continue;
			}
			if(picture[indexX-1][indexY] ==0  || picture[indexX+1][indexY] ==0 || picture[indexX][indexY+1] ==0 || picture[indexX][indexY-1] ==0)
			{
				component[i].boundX.push_back(component[i].placeX[j]);
				component[i].boundY.push_back(component[i].placeY[j]);
			}
		}
		//calcute the Perimeter
		int dir = 0;
		bool EXIT = false;
		component[i].PerRecordX.push_back(component[i].boundX[0]);
		component[i].PerRecordY.push_back(component[i].boundY[0]);
		int row = component[i].boundX[0];
		int col = component[i].boundY[0];
		int nextrow = row + move[dir].horiz;
		int nextcol = col + move[dir].vert;	
		int visittimes = 0;
		int count =0;
		if(component[i].boundX.size() != 1)
		{
			while(!EXIT)
			{
				if (nextrow == component[i].boundX[0] && nextcol == component[i].boundY[0])
				{
					visittimes ++;
					switch(dir)
					{
						case 0:
						case 1:
							dir = 6;
							break;
						case 2:
						case 3:
							dir = 0;
							break;
						case 4:
						case 5:
							dir = 2;
							break;
						case 6:
						case 7: 
							dir = 4;
							break;
						default:
							printf("failed");
					}
					row=nextrow;
					col=nextcol;					
					component[i].PerRecordX.push_back(nextrow);
					component[i].PerRecordY.push_back(nextcol);
					if((visittimes >=2) || ((visittimes>=1) && dir ==0))
						EXIT = true;
					break;
				}
				if(pointConfirm(nextrow,nextcol,component[i]))
				{
					component[i].PerRecordX.push_back(nextrow);
					component[i].PerRecordY.push_back(nextcol);
					switch(dir)
					{
						case 0:
						case 1:
							dir = 6;
							break;
						case 2:
						case 3:
							dir = 0;
							break;
						case 4:
						case 5:
							dir = 2;
							break;
						case 6:
						case 7: 
							dir = 4;
							break;
						default:
							printf("failed");
					}
					row=nextrow;
					col=nextcol;
				}else
				{
					if(dir<7)
						dir++;
					else
						dir=0;
				}
				nextrow = row + move[dir].horiz;
				nextcol = col + move[dir].vert;
			}
		}else
		{
			component[i].PerRecordX.push_back(component[i].boundX[0]);
			component[i].PerRecordY.push_back(component[i].boundY[0]);
		}
		//calculate diff()
		float total =0;
		for (int j=0;j<component[i].PerRecordX.size()-1;j++)
		{
			float diffresult= sqrt(pow(float(component[i].PerRecordX[j+1]-component[i].PerRecordX[j]),2)+pow(float(component[i].PerRecordY[j+1]-component[i].PerRecordY[j]),2));
			//component[i].diff2pow.push_back(diffresult);
			total += diffresult;
		}
		component[i].Perimeter = total;
	}
}
void boundingbox(vector<OBJECT> &component)
{
	for(int i=0;i<component.size();i++)
	{
		int maxX=component[i].boundX[0];
		int maxY=component[i].boundY[0];
		int minX=component[i].boundX[0];
		int minY=component[i].boundY[0];;
		for(int j=1;j<component[i].boundX.size();j++)
		{
			maxX=MAX(maxX,component[i].boundX[j]);
			maxY=MAX(maxY,component[i].boundY[j]);
			minX=MIN(minX,component[i].boundX[j]);
			minY=MIN(minY,component[i].boundY[j]);
		}
		component[i].BoundingBox[0]=minX-0.5;
		component[i].BoundingBox[1]=minY-0.5;
		component[i].BoundingBox[2]=maxX-minX+1;
		component[i].BoundingBox[3]=maxY-minY+1;
	}
}
bool pointConfirm(int& row,int& col,OBJECT &component)
{
	for(int j=0;j<component.boundX.size();j++)
	{
		if(row == component.boundX[j] && col == component.boundY[j])
		{
			row = component.boundX[j];
			col = component.boundY[j];
			return true;
		}
	}
	return false;
}
void centroid(vector<OBJECT> & component)
{
	for(int i=0;i<component.size();i++)
	{
		float totalX=0;
		float totalY=0;
		for(int j=0;j<component[i].placeX.size();j++)//placeX is the way to present in a[x][y],so it need to transfer in y
		{
			totalX+=component[i].placeX[j];
			totalY+=component[i].placeY[j];
		}
		component[i].Centroid[0] = totalX/component[i].placeX.size(); 
		component[i].Centroid[1] = totalY/component[i].placeY.size();
	}
}
void orientation(vector<OBJECT> & component)
{
	for(int i=0;i<component.size();i++)
	{
		float xbar = component[i].Centroid[0];
		float ybar = component[i].Centroid[1];

	//In matlab
	//   x = list(:,1) - xbar;
	//   y = -(list(:,2) - ybar); % This is negative for the
	//   % orientation calculation (measured in the
	//   % counter-clockwise direction).
		float uxx=0;
		float uyy=0;
		float uxy=0;
		float sumx=0;
		float sumy=0;
		float sumxy=0;
		float x,y;
		float num=0,den=0;
		for (int j=0;j<component[i].placeX.size();j++)
		{
			x = component[i].placeX[j]-xbar;
			y = -(component[i].placeY[j]-ybar);
			sumx += pow(x,2);
			sumy += pow(y,2);
			sumxy += x*y;
		}
		uxx = sumx/component[i].placeX.size() + 1/12;
		uyy = sumy/component[i].placeX.size() + 1/12;
		uxy = sumxy/component[i].placeX.size();
		// Calculate orientation.
		if (uyy > uxx)
		{
			num = uyy - uxx + sqrt(pow((uyy - uxx),2) + 4*pow(uxy,2));
			den = 2*uxy;
		}else
		{
			num = 2*uxy;
			den = uxx - uyy + sqrt(pow((uxx - uyy),2) + 4*pow(uxy,2));
		}
		if ((num == 0) && (den == 0))
			component[i].Orientation = 0;
		else
			component[i].Orientation = (180/PI) * atan(num/den);
	}
}

void flood4(int x, int y, int original_color,int fill_color,int m,int n,vector<vector<int> > &picture,vector<vector<bool> > &visit,OBJECT &BB){
	if (x>=0 && x<m && y>=0 && y<n)
		if (picture[x][y] == original_color && !visit[x][y]){
			BB.numberOfElement++;
			BB.placeX.push_back(y);
			BB.placeY.push_back(x);			
			visit[x][y] = true;
			picture[x][y] = fill_color;
			flood4(x+1, y, original_color,fill_color,m,n,picture,visit,BB);
			flood4(x-1, y, original_color,fill_color,m,n,picture,visit,BB);
			flood4(x, y+1, original_color,fill_color,m,n,picture,visit,BB);
			flood4(x, y-1, original_color,fill_color,m,n,picture,visit,BB);
		}
}

void flood8(int x, int y, int original_color,int fill_color,int m,int n,vector<vector<int> > &picture,vector<vector<bool> > &visit,OBJECT &BB){
	if (x>=0 && x<m && y>=0 && y<n)
		if (picture[x][y] == original_color && !visit[x][y]){
			BB.numberOfElement++;
			BB.placeX.push_back(y);
			BB.placeY.push_back(x);	
			visit[x][y] = true;
			picture[x][y] = fill_color;	
			flood8(x+1, y, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x-1, y, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x, y+1, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x, y-1, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x+1, y+1, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x-1, y+1, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x-1, y-1, original_color,fill_color,m,n,picture,visit,BB);
			flood8(x+1, y-1, original_color,fill_color,m,n,picture,visit,BB);
		}
}

/* display error message and exit */
void fisError(char *msg){
#ifdef MATLAB_MEX_FILE
	mexErrMsgTxt(msg);
#else
	printf(msg);
	printf("\n");
	exit(1);
#endif
}


char **fisCreateMatrix(int row_n, int col_n, int element_size){
	char **matrix;
	int i;

	if (row_n == 0 && col_n == 0)
		return(NULL);

	matrix = (char **)calloc(row_n, sizeof(char *));
	if (matrix == NULL)
		fisError((char *)"Error in fisCreateMatrix!");
	for (i = 0; i < row_n; i++) { 
		matrix[i] = (char *)calloc(col_n, element_size);
		if (matrix[i] == NULL)
			fisError((char *)"Error in fisCreateMatrix!");
	}
	return(matrix);
}

void polyval(double *coef,double *value,vector<double> &polyvalue,int order,int coefsize)
{
		for(int j=0;j<=order;j++)
		{
			polyvalue[j]=0;
 			for(int i=coefsize-1;i>=0;i--)
			{
				polyvalue[j]+=(coef[coefsize-i-1]*pow(value[j],i));
			}
		}
}

bool convolve2D(float* in, float* out, int dataSizeX, int dataSizeY, 
                float* kernel, int kernelSizeX, int kernelSizeY)
{
    int i, j, m, n;
    float *inPtr, *inPtr2, *outPtr, *kPtr;
    int kCenterX, kCenterY;
    int rowMin, rowMax;                             // to check boundary of input array
    int colMin, colMax;                             //

    // check validity of params
    if(!in || !out || !kernel) return false;
    if(dataSizeX <= 0 || kernelSizeX <= 0) return false;

    // find center position of kernel (half of kernel size)
    kCenterX = kernelSizeX >> 1;
    kCenterY = kernelSizeY >> 1;

    // init working  pointers
    inPtr = inPtr2 = &in[dataSizeX * kCenterY + kCenterX];  // note that  it is shifted (kCenterX, kCenterY),
    outPtr = out;
    kPtr = kernel;

    // start convolution
    for(i= 0; i < dataSizeY; ++i)                   // number of rows
    {
        // compute the range of convolution, the current row of kernel should be between these
        rowMax = i + kCenterY;
        rowMin = i - dataSizeY + kCenterY;

        for(j = 0; j < dataSizeX; ++j)              // number of columns
        {
            // compute the range of convolution, the current column of kernel should be between these
            colMax = j + kCenterX;
            colMin = j - dataSizeX + kCenterX;

            *outPtr = 0;                            // set to 0 before accumulate

            // flip the kernel and traverse all the kernel values
            // multiply each kernel value with underlying input data
            for(m = 0; m < kernelSizeY; ++m)        // kernel rows
            {
                // check if the index is out of bound of input array
                if(m <= rowMax && m > rowMin)
                {
                    for(n = 0; n < kernelSizeX; ++n)
                    {
                        // check the boundary of array
                        if(n <= colMax && n > colMin)
                            *outPtr += *(inPtr - n) * *kPtr;
                        ++kPtr;                     // next kernel
                    }
                }
                else
                    kPtr += kernelSizeX;            // out of bound, move to next row of kernel

                inPtr -= dataSizeX;                 // move input data 1 raw up
            }

            kPtr = kernel;                          // reset kernel to (0,0)
            inPtr = ++inPtr2;                       // next input
            ++outPtr;                               // next output
        }
    }

    return true;
}
