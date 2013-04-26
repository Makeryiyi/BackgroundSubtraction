//
//  main.c
//  OpenCVTest
//
//  Created by kuno_lab on 2013/03/15.
//  Copyright (c) 2013年 kuno_lab. All rights reserved.
//
#include <cv.h>
#include <highgui.h>

#define WHITE !0
#define BLACK 0

void frame_diff(IplImage *src, IplImage *bg, IplImage *new_bg);
void frame_diff_color(IplImage *src, IplImage *bg, IplImage *new_bg);
void showGrayImage(const char *name, IplImage *img);
void noise_reduce(IplImage *image, int noise_color, int num);

int main(int argc, char **argv)
{
    IplImage *new_bg = cvLoadImage("/Users/kuno_lab/Pictures/bg.jpg", CV_LOAD_IMAGE_COLOR);
    IplImage *frame, *bg_img = NULL;
    CvCapture *capture = cvCreateCameraCapture(0);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, new_bg->width);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, new_bg->height);
    
    
    int flag = 0;
    
    while (1) {
        frame = cvQueryFrame(capture);
        
        char c = cvWaitKey(1);
        
        switch (c) {
            case 's':
                if (bg_img) {
                    cvReleaseImage(&bg_img);
                }
                bg_img = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
                cvCopy(frame, bg_img, NULL);
                showGrayImage("background-gray", bg_img);
                break;
            case 'c' :
                flag = !flag;
                break;
            default:
                break;
        }
        
        showGrayImage("frame-gray", frame);

        if (flag) {
            frame_diff_color(frame, bg_img, new_bg);
        }

        cvShowImage("frame", frame);
        
        if (c == 27) break;
    }
    cvReleaseImage(&new_bg);
    cvReleaseImage(&bg_img);
    cvDestroyAllWindows();
    
    return 0;
}

void frame_diff(IplImage *src, IplImage *bg, IplImage *new_bg)
{
    IplImage *src_gray;
    IplImage *mask, *bg_gray;
    mask = cvCreateImage(cvGetSize(src), src->depth, 1);
    bg_gray = cvCreateImage(cvGetSize(src), src->depth, 1);
    src_gray = cvCreateImage(cvGetSize(src), src->depth, 1);

    
    // 絶対値差分画像を取得
    cvCvtColor(src, src_gray ,CV_BGR2GRAY);
    cvCvtColor(bg, bg_gray ,CV_BGR2GRAY);
    cvAbsDiff(src_gray, bg_gray, mask);

    // 差分画像を2値化
    cvThreshold(mask, mask, 10, 255, CV_THRESH_BINARY);
    // ノイズ除去
    noise_reduce(mask, WHITE, 2);
    noise_reduce(mask, BLACK, 2);
    cvShowImage("MASK", mask);
    
    // 背景部分に背景を挿入
    cvNot(mask, mask);
    cvCopy(new_bg, src, mask);

    cvReleaseImage(&src_gray);
    cvReleaseImage(&bg_gray);
    cvReleaseImage(&mask);
}

void frame_diff_color(IplImage *src, IplImage *bg, IplImage *new_bg)
{
    IplImage *color_sub;
    IplImage *gray1, *gray2, *gray3;
    IplImage *mask;
    color_sub = cvCreateImage(cvGetSize(src), src->depth, src->nChannels);
    mask = cvCreateImage(cvGetSize(src), src->depth, 1);
    gray1 = cvCreateImage(cvGetSize(src), src->depth, 1);
    gray2 = cvCreateImage(cvGetSize(src), src->depth, 1);
    gray3 = cvCreateImage(cvGetSize(src), src->depth, 1);

    // カラー絶対値差分画像を取得
    cvAbsDiff(src, bg, color_sub);

    // カラー絶対値差分画像をチャンネルごとに2値化
    cvSplit(color_sub, gray1, gray2, gray3, NULL);
    cvThreshold(gray1, gray1, 15, 255, CV_THRESH_BINARY);
    cvThreshold(gray2, gray2, 15, 255, CV_THRESH_BINARY);
    cvThreshold(gray3, gray3, 15, 255, CV_THRESH_BINARY);

    // 2値化した差分の論理和を取りマスクを得る
    cvOr(gray1, gray2, gray2);
    cvOr(gray2, gray3, mask);

    // ノイズ除去
//    noise_reduce(mask, WHITE, 2);
//    noise_reduce(mask, BLACK, 2);
//    cvShowImage("MASK", mask);

    // 背景部分に背景を挿入
    cvNot(mask, mask);
    cvCopy(new_bg, src, mask);

    cvReleaseImage(&color_sub);
    cvReleaseImage(&gray1);
    cvReleaseImage(&gray2);
    cvReleaseImage(&gray3);
    cvReleaseImage(&mask);
}

void showGrayImage(const char *name, IplImage *img)
{
    IplImage *gray_img = cvCreateImage(cvGetSize(img), img->depth, 1);
    cvCvtColor(img, gray_img, CV_BGR2GRAY);

    cvShowImage(name, gray_img);
    cvReleaseImage(&gray_img);
}

void noise_reduce(IplImage *image, int noise_color, int num)
{
    if (noise_color == WHITE) {
        for (int i=0; i<num; i++)
            cvErode(image, image);
    }

    for (int i=0; i<num; i++)
        cvDilate(image, image);

    if (noise_color == BLACK) {
        for (int i=0; i<num; i++)
            cvErode(image, image);
    }
}
