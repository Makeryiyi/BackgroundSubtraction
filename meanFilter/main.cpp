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
#include <pthread.h>
#include <unistd.h>
#include <time.h>

void meanFilter(IplImage *srcImage, IplImage *dstImage, int size) {
    int step = srcImage->width-size;
    if (step < 0) step = srcImage->width;
    
    for (int rr = 0; rr < srcImage->height;rr++) {
        for (int c=0;c<srcImage->width;c++) {
            unsigned int r=0,g=0,b=0;
            for (int i = 0;i<size;i++) {
                for (int j = 0;j<size;j++) {
                    int tc = (c+j-size/2);
                    int tr = (rr+i-size/2);
                    if (tc < 0 || tc > srcImage->width || tr<0 || tr>=srcImage->height) {
                    } else {
                        b += (unsigned int)(unsigned char)srcImage->imageData[tc*3+0+tr*srcImage->widthStep];
                        g += (unsigned int)(unsigned char)srcImage->imageData[tc*3+1+tr*srcImage->widthStep];
                        r += (unsigned int)(unsigned char)srcImage->imageData[tc*3+2+tr*srcImage->widthStep];
                    }
                }
            }
            b /= size*size;
            g /= size*size;
            r /= size*size;
            dstImage->imageData[c*3+0+rr*dstImage->widthStep] = (char)(unsigned char)b;
            dstImage->imageData[c*3+1+rr*dstImage->widthStep] = (char)(unsigned char)g;
            dstImage->imageData[c*3+2+rr*dstImage->widthStep] = (char)(unsigned char)r;
        }
    }
}

int main() {
    IplImage *srcImage = cvLoadImage("/Users/kuno_lab/Pictures/bg.jpg");
    IplImage *dstImage = cvCreateImage(cvGetSize(srcImage), srcImage->depth, srcImage->nChannels);
    
    clock_t start, end;

    printf("start\n");
    start = clock();

    meanFilter(srcImage, dstImage, 20);

    end = clock();
    printf("end\n");
    
    printf("%lf[s]\n", (((double)end-(double)start)/(double)CLOCKS_PER_SEC));

    cvShowImage("srcImage", srcImage);
    cvShowImage("dstImage", dstImage);
    while (1) {
        if (cvWaitKey() == 'a') break;
    }
    return 0;
}