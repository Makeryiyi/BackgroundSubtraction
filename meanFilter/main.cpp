//
//  main.cpp
//  meanFilter
//
//  Created by kuno_lab on 2013/05/24.
//  Copyright (c) 2013年 kuno_lab. All rights reserved.
//

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

struct Arg {
    IplImage *srcImage;
    IplImage *dstImage;
    int size;
    int num;
};

void *filter(void *_arg) {
    Arg *arg = (Arg *)_arg;
    IplImage *srcImage = arg->srcImage;
    IplImage *dstImage = arg->dstImage;
    int size = arg->size;
    int num = arg->num;

    for (int r = 0; r < srcImage->height;r++) {
        for (int c=0;c<srcImage->width;c+=4) {
            int cc = c + num;
            unsigned int rc=0,gc=0,bc=0;
            for (int i = 0;i<size;i++) {
                for (int j = 0;j<size;j++) {
                    int tc = (cc+j-size/2);
                    int tr = (r+i-size/2);
                    if (tc < 0 || tc > srcImage->width || tr<0 || tr>=srcImage->height) {
                    } else {
                        bc += (unsigned int)(unsigned char)srcImage->imageData[tc*3+0+tr*srcImage->widthStep];
                        gc += (unsigned int)(unsigned char)srcImage->imageData[tc*3+1+tr*srcImage->widthStep];
                        rc += (unsigned int)(unsigned char)srcImage->imageData[tc*3+2+tr*srcImage->widthStep];
                    }
                }
            }
            bc /= size*size;
            gc /= size*size;
            rc /= size*size;
            dstImage->imageData[cc*3+0+r*dstImage->widthStep] = (char)(unsigned char)bc;
            dstImage->imageData[cc*3+1+r*dstImage->widthStep] = (char)(unsigned char)gc;
            dstImage->imageData[cc*3+2+r*dstImage->widthStep] = (char)(unsigned char)rc;
        }
    }
    return NULL;
}

void meanFilter(IplImage *srcImage, IplImage *dstImage, int size)
{
    static int NUM = 4;
    pthread_t th[NUM];
    struct Arg arg[4] = {
        {srcImage, dstImage, size, 0},
        {srcImage, dstImage, size, 1},
        {srcImage, dstImage, size, 2},
        {srcImage, dstImage, size, 3}
    };
    for (int i=0;i<NUM;i++) {
        pthread_create(&th[i], NULL, filter, &arg[i]);
    }
    for (int i=0;i<NUM;i++) {
        pthread_join(th[i], NULL);
    }
}

double get_time(struct timeval *time)
{
    return ((double)(time->tv_sec)*1000 + (double)(time->tv_usec)*0.001);
}

int main() {
    IplImage *srcImage = cvLoadImage("/Users/kuno_lab/Pictures/bg.jpg");
    IplImage *dstImage = cvCreateImage(cvGetSize(srcImage), srcImage->depth, srcImage->nChannels);
    
    timeval start,end;
    
    printf("start\n");
    gettimeofday(&start, NULL);
    
    meanFilter(srcImage, dstImage, 51);

    gettimeofday(&end, NULL);

    printf("end\n");
    printf("%lf[ms]", get_time(&end) - get_time(&start));

    cvShowImage("srcImage", srcImage);
    cvShowImage("dstImage", dstImage);
    while (1) {
        if (cvWaitKey() == 'a') break;
    }
    return 0;
}