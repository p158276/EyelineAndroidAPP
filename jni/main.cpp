/* #include <cv.h>
#include <highgui.h> */
#include <time.h>
#include <jni.h>
#include<android/log.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include "eyelinecv.hpp"
#include "cliu_TutorialOnFaceDetect_TutorialOnFaceDetect.h"
using namespace std;
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav",__VA_ARGS__)
#define KERNELM	7
#define KERNELN	3
#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))
jintArray Java_cliu_TutorialOnFaceDetect_TutorialOnFaceDetect_sendToC
(JNIEnv* env, jobject obj,jint height,jint width,jintArray ba)
{
	jint* im;
	jint size;
	im =  env -> GetIntArrayElements(ba, NULL);
	if(im == NULL)
		return 0;
	size = env->GetArrayLength(ba);
//	vector<int> im3(height*width);
	int IMAGEM = height;
	int IMAGEN = width;
	LOGD("#############test log in size is %d\n",size);
	int count=0;
	/* initialization from eyeLinePrmSet */
	int threshold = 25;
	float angle = 40;
	float mask[KERNELM][KERNELN] = {{float(-1)/9,float(-1)/9,float(-1)/9},
									{float(-1)/9,float(-1)/9,float(-1)/9},
									{float(-1)/9,float(-1)/9,float(-1)/9},
									{float(1)/12,float(1)/12,float(1)/12},
									{float(1)/12,float(1)/12,float(1)/12},
									{float(1)/12,float(1)/12,float(1)/12},
									{float(1)/12,float(1)/12,float(1)/12}};
	int top = 5;
	int polyDegree = 3;
	int downTunningMax = 5;
	int paddingWidth = 7;
	/* do convolution */
	float *dataValue;
	float *maskValue;
	float *conv2Value;
	dataValue = new float[IMAGEM*IMAGEN];
	maskValue = new float[KERNELM*KERNELN];
	conv2Value = new float[IMAGEM*IMAGEN];
	vector< OBJECT > component(0);
	for (int i=0;i<size;i++)
	{
		dataValue[i] = float(int(im[i]));
	}
/*	for (int i=0;i<count;i++)
	{
		dataValue[i]=float(im);
		//printf("%d \n",im3[i]);
	}*/
	for (int i=0,d=0; i<KERNELM;i++)
		for(int j=0;j<KERNELN;j++)
		{
			maskValue[d++]=float(mask[i][j]);
		}
	if(!convolve2D(dataValue, conv2Value, IMAGEN, IMAGEM, maskValue, KERNELN,KERNELM))
	{
		printf("Fail to do the convolution!!!!\n");
	}
	vector<vector<float> > convResult(IMAGEM, vector<float>(IMAGEN));
	vector<vector<int> > regionMatrix(IMAGEM, vector<int>(IMAGEN));
	for (int i=0,r=0; i<IMAGEM; i++)
		for (int j=0; j<IMAGEN; j++)
		{
			convResult[i][j]=float(conv2Value[r++]);
			regionMatrix[i][j] = float(((int)(convResult[i][j]*100))/100.0)> threshold;
		}
	delete [] dataValue;
	delete [] maskValue;
	delete [] conv2Value;
	/* padding */
	for(int i=0;i<paddingWidth;i++)
		for(int j=0;j<IMAGEN;j++)
			regionMatrix[i][j]=0;
	for(int i=(IMAGEM-paddingWidth)-1;i<IMAGEM;i++)
		for(int j=0;j<IMAGEN;j++)
			regionMatrix[i][j]=0;
	for(int i=0;i<IMAGEM;i++)
		for(int j=0;j<paddingWidth;j++)
			regionMatrix[i][j]=0;
	for(int i=0;i<IMAGEM;i++)
		for(int j=(IMAGEN-paddingWidth)-1;j<IMAGEN;j++)
			regionMatrix[i][j]=0;
	LOGD("regionprops\n");
	int objCount=bwLabel(regionMatrix, IMAGEM, IMAGEN, 8, component);
	if(component.size()==0)
	{
		int out =0;
		LOGE("No more component!!!\n");
		jint *output = new jint[IMAGEM*IMAGEN];
		for(int i=0;i<IMAGEM;i++)
			for(int j=0;j<IMAGEN;j++)
				output[out++]=0;
		env->ReleaseIntArrayElements(ba,im,0);
		jintArray retArray = (env)->NewIntArray(IMAGEM*IMAGEN);
		if (retArray == NULL) {
		    printf("Out of memory....\n");
		    return NULL;
		}
		(env)->SetIntArrayRegion(retArray, 0, IMAGEM*IMAGEN, output);
		//return paddingWidth;
		return retArray;
	}
	Perimeter(regionMatrix,IMAGEM,IMAGEN,component);
	boundingbox(component);
	centroid(component);
	orientation(component);
	LOGD("end regionprops\n");
	LOGD("component.size()=%d\n",component.size());
	bool enter  = true;
	while(enter == true)
	{
		for(int i=component.size()-1;i >= 0;i--)
		{	if(fabs(component[i].Orientation) > angle)
			{
				component.erase(component.begin()+i);
				//printf("%d\n",i);
				enter = true;
				break;
			}
			enter = false;
		}
	}
	LOGD("end while loop\n");
	vector<float> parameter (component.size(),0);
	vector<float> ycentroid (component.size(),0);
	for(int i=0;i< component.size();i++)
	{
		parameter[i] = component[i].Perimeter;
		ycentroid[i] = component[i].Centroid[1]+1;
		//printf("parameter[%d] = %f \t ycentroid[%d] = %f \n",i,parameter[i],i,ycentroid[i]);
	}
	sort(parameter.rbegin(),parameter.rend());
    if (top > parameter.size())
        top = parameter.size();
	float base_thres=0;
	for(int i=0;i<top;i++)
	{
		base_thres+=parameter[i];
	}
	base_thres=base_thres/top;
	vector<int> base_cand (0);
	for(int i=0;i<component.size();i++)
	{
		if(component[i].Perimeter>=base_thres)
		{
			base_cand.push_back(i);
		}

	}
	int baseIdx;
	int tmp=0;
	for(int i=0;i< base_cand.size();i++)
	{
		if(ycentroid[base_cand[i]]>tmp)
		{
			tmp=ycentroid[base_cand[i]];
			baseIdx=base_cand[i];
		}
	}
	//printf("baseIdx = %d\n",baseIdx);
    // get base line information
    float baseY = component[baseIdx].Centroid[1]+1;
    float leftX = component[baseIdx].BoundingBox[0]+1;
    float rightX = leftX + component[baseIdx].BoundingBox[2];
    //threshold for yCentroid condition
    float thres = component[baseIdx].BoundingBox[3];
    LOGD("############# %f , %f , %f , %f\n",baseY,leftX,rightX,thres);
	printf(" %f , %f , %f , %f\n",baseY,leftX,rightX,thres);

// initialization for reducing noise
	vector<int> match (0);
	match.push_back(baseIdx);
	enter = true;
	vector<bool> enterpoint(component.size(),false);
	float lX;
	float rX;
	LOGD("start another whileloop\n");
	while(enter)
	{
		for(int i=0;i< component.size();i++)
		{
			lX = component[i].BoundingBox[0]+1;
			rX = component[i].BoundingBox[2]+lX;
			//y禸いみ翴ぃ禬筁琘rangeヴregion常ぃ程regionㄤよ┪よ–regionrightXぃ瞒程regionleftXび环
			//old ver.
			//if((fabs((component[i].Centroid[1]+1) - baseY) < thres/2) && (rX < leftX || lX > rightX) && ( enterpoint[i] == false))
			if((fabs((component[i].Centroid[1]+1) - baseY) < thres/2) && ((rX > leftX && rX < (leftX + thres)
			&&lX<leftX) || (lX > (rightX-thres) && lX < rightX && rX > rightX) || ((fabs(rX - leftX) < thres/2
					|| fabs(lX - rightX) < thres/2) && component[i].Perimeter > thres )) && ( enterpoint[i] == false))
			{
				match.push_back(i);
				if(lX < leftX)
					leftX = lX;
				if(rX > rightX)
					rightX = rX;
				enterpoint[i] = true;
				break;
			}
			if(i==(component.size()-1))
				enter = false;
		}
	};
	LOGD("finish another whileloop\n");
	vector<int> xx (0);
	vector<int> yy (0);
	if (match.size()!=0)
	{
		for(int i=0; i<match.size();i++)
		{
			for (int y=0;y<component[match[i]].placeX.size();y++)
			{
				xx.push_back(component[match[i]].placeX[y]);
				yy.push_back(component[match[i]].placeY[y]);
				//printf("for match[i] = %d ",match[i]);
				//printf("(%d,%d)\n",component[match[i]].placeX[y],component[match[i]].placeY[y]);
			}
		}
	}
	vector<int> xxs (0);
	int xxcount = *min_element(xx.begin(),xx.end());
	int xxmin = *min_element(xx.begin(),xx.end());
	int xxmax = *max_element(xx.begin(),xx.end());
	while(xxcount <= xxmax)
		xxs.push_back(xxcount++);
	int xxsmin = *min_element(xxs.begin(),xxs.end());
	int xxsmax = *max_element(xxs.begin(),xxs.end());
	double *xxin,*yyin,*xxsin,*xxsscale;
	double *coef;
	//*polyvalue;
	xxin = new double[xx.size()];
	yyin = new double[yy.size()];
	xxsin = new double[xxs.size()];
	xxsscale = new double[xxs.size()];

	for(int i=0;i<xx.size();i++)
	{
		//xxin[i] =  double(int(double(xx[i]-xxmin)/(xxmax-xxmin)*20)-10);
		xxin[i] = double(xx[i]+1);
		//xxin[i] = int((xxin[i]-xxmin)/(xxmax-xxmin) * 255);
		yyin[i] = double(yy[i]+1);
		//LOGD("xxin yyin = %f %f \n",xxin[i],yyin[i]);
	}
	for(int i=0;i<xxs.size();i++)
	{
		xxsin[i]=(xxs[i])+1;
		//xxsscale[i]=double(int(double(xxs[i]-xxsmin)/(xxsmax-xxsmin)*20)-10);
		//xxsin[i]=double(xxs[i]+1);
		//LOGD("###xxsin########## %f\n",xxsin[i]);
	}

	coef = new double[polyDegree+1];
	//polyvalue = new double[xxs.size()];
	vector<double> polyvalue(xxs.size()+1);
	polyfit(xxin, xx.size(), yyin, polyDegree, coef);
	delete [] xxin;
	delete [] yyin;
	//for(int i=0;i< polyDegree+1;i++)
		//LOGD("coef = %f \n",coef[i]);
	polyval(coef,xxsin,polyvalue,xxs.size(),polyDegree+1);
	int co=0,out=0;
	jint *output = new jint[IMAGEM*IMAGEN];
	for(int i=0;i<xxs.size();i++)
	{
		polyvalue[i]=(polyvalue[i]);
		//LOGD("###polyvalue########## %f\n",polyvalue[i]);
		//xxsscale[i]=double(int(double(xxs[i]-xxsmin)/(xxsmax-xxsmin)*20)-10);
		//xxsin[i]=double(xxs[i]+1);
	}
	for(int i=0;i<IMAGEM;i++)
		for(int j=0;j<IMAGEN;j++)
			output[out++]=0;
	for(int co=0;co<xxs.size();co++)
		output[int(polyvalue[co]-1)*IMAGEN+int(xxsin[co])]=1;

	delete [] xxsin;
	delete [] xxsscale;
	delete [] coef;
	LOGD("Finish cleaning the memory \n");
	/* initialization from eyeLinePrmSet */
	env->ReleaseIntArrayElements(ba,im,0);
	jintArray retArray = (env)->NewIntArray(IMAGEM*IMAGEN);
	if (retArray == NULL) {
	    printf("Out of memory....\n");
	    return NULL;
	}

	(env)->SetIntArrayRegion(retArray, 0, IMAGEM*IMAGEN, output);
	delete [] output;
	(env)->DeleteLocalRef(ba);
	//return paddingWidth;
	return retArray;
}




	//for(int i=0;i<xx.size();i++)
	//	printf("(%d,%d)\n",xx[i],yy[i]);
	//for (int i=0;i<xxs.size();i++)
	//	printf("%f ",polyvalue[i]);
	/*for(int i=0;i< component.size();i++)
	{
		printf("Object %d\n",i+1);
		printf("Perimeter = %f\n",component[i].Perimeter);
        for(int j=0;j<component[i].placeX.size();j++)
		{
			printf("(%d,%d)\n",component[i].placeX[j]+1,component[i].placeY[j]+1);
		}
		//for(int k=0;k<component[i].boundX.size();k++)
		//	printf("boundary (%d,%d)\n",component[i].boundX[k]+1,component[i].boundY[k]+1);
		//for(int q=0;q<component[i].PerRecordX.size();q++)
		//	printf("PerRecord (%d,%d)\n",component[i].PerRecordX[q]+1,component[i].PerRecordY[q]+1);
		/*printf("boundingbox = ( %.1f , %.1f , %.1f , %.1f)\n",component[i].BoundingBox[0]+1,component[i].BoundingBox[1]+1,component[i].BoundingBox[2],component[i].BoundingBox[3]);
		printf("Centroid = ( %f , %f )\n",component[i].Centroid[0]+1,component[i].Centroid[1]+1); 
		printf("orientation = %f\n",component[i].Orientation);
	}*/
    /* Save Image RGB Values */
/* 	int co =0; 
	for(int j=0;j<Image1->width*3;j=j+3)
	{
		for(int i=0;i<Image1->height;i++)
		{
			if(i==int(polyvalue[co]-1) && j/3 == int(xxsin[co]-1))
			{
				Image1->imageData[i*Image1->widthStep+j]=255;
				Image1->imageData[i*Image1->widthStep+j+1]=0;
				Image1->imageData[i*Image1->widthStep+j+2]=255;
				co++;
			}
		}
    }
	cvSaveImage("output.jpg",Image1);
	cvNamedWindow("EyeLineResult",1);
    cvShowImage("EyeLineResult",Image1);
    cvWaitKey(0);
    cvReleaseImage(&Image1);
    cvDestroyWindow("EyeLineResult"); */

