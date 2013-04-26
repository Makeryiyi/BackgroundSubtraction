//
//  bgAvg.cpp
//  OpenCVTest
//
//  Created by kuno_lab on 2013/03/27.
//  Copyright (c) 2013年 kuno_lab. All rights reserved.
//

#include <cv.h>
#include <highgui.h>

#define ACC_FRAME 100

IplImage *IavgF, *IdiffF, *IprevF, *IhiF, *IlowF;

IplImage *Iscratch, *Iscratch2;

IplImage *Igray1, *Igray2, *Igray3;
IplImage *Ilow1, *Ilow2, *Ilow3;
IplImage *Ihi1, *Ihi2, *Ihi3;

IplImage *Imaskt;

float Icount;

void AllocateImages(IplImage *I);
void accumulateBackground(IplImage *I);
void backgroundDiff(IplImage *I, IplImage *Imask);
void DeallocateImages();
void createModelsfromStats();
void setHighThreshold(float scale);
void setLowThreshold(float scale);

int main(int argc, char **argv)
{
    IplImage *new_bg = cvLoadImage("/Users/kuno_lab/Pictures/bg.jpg", CV_LOAD_IMAGE_COLOR);
    IplImage *frame, *bg_img = NULL, *mask;
    CvCapture *capture = cvCreateCameraCapture(0);
    mask = cvCreateImage(cvGetSize(new_bg), new_bg->depth, 1);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, new_bg->width);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, new_bg->height);
    
    
    int flag = 0;

    
    
    while (1) {
        frame = cvQueryFrame(capture);
        
        char c = cvWaitKey(50);
        
        switch (c) {
            case 's':
                DeallocateImages();
                AllocateImages(frame);
                
                printf("Start accumulate Background\n");
                for (int i=0; i<ACC_FRAME; i++) {
                    accumulateBackground(cvQueryFrame(capture));
                }
                createModelsfromStats();
                printf("Finish accumulate Background\n");
                
                flag = 1;
                break;
            case 'c' :

                break;
            default:
                break;
        }

        if (flag) {
            backgroundDiff(frame, mask);
        }
        
        cvShowImage("frame", mask);
        
        if (c == 27) break;
    }
    DeallocateImages();
    cvReleaseImage(&new_bg);
    cvReleaseImage(&bg_img);
    cvDestroyAllWindows();
    
    return 0;
}

void AllocateImages(IplImage *I)
{
    CvSize size = cvGetSize(I);
    
    IavgF   = cvCreateImage(size, IPL_DEPTH_32F, 3);
    IdiffF  = cvCreateImage(size, IPL_DEPTH_32F, 3);
    IprevF  = cvCreateImage(size, IPL_DEPTH_32F, 3);
    IhiF    = cvCreateImage(size, IPL_DEPTH_32F, 3);
    IlowF   = cvCreateImage(size, IPL_DEPTH_32F, 3);
    Ilow1   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Ilow2   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Ilow3   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Ihi1   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Ihi2   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Ihi3   = cvCreateImage(size, IPL_DEPTH_32F, 1);
    cvZero(IavgF);
    cvZero(IdiffF);
    cvZero(IprevF);
    cvZero(IhiF);
    cvZero(IlowF);
    Icount = 0.00001;
    
    Iscratch    = cvCreateImage(size, IPL_DEPTH_32F, 3);
    Iscratch2   = cvCreateImage(size, IPL_DEPTH_32F, 3);
    Igray1      = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Igray2      = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Igray3      = cvCreateImage(size, IPL_DEPTH_32F, 1);
    Imaskt      = cvCreateImage(size, IPL_DEPTH_8U, 1);
    cvZero(Iscratch);
    cvZero(Iscratch2);
}

void accumulateBackground(IplImage *I)
{
    static int first = 1;
    cvCvtScale(I, Iscratch, 1, 0);
    if (!first) {
        cvAcc(Iscratch, IavgF);
        cvAbsDiff(Iscratch, IprevF, Iscratch2);
        cvAcc(Iscratch2, IdiffF);
        Icount += 1.0;
    }
    first = 0;
    cvCopy(Iscratch, IprevF);
}

void createModelsfromStats()
{
    cvConvertScale(IavgF, IavgF, (double)(1.0/Icount));
    cvConvertScale(IdiffF, IdiffF, (double)(1.0/Icount));

    cvAddS(IdiffF, cvScalar(1.0, 1.0, 1.0), IdiffF);
    setHighThreshold(6.0);
    setLowThreshold(7.0);
}

void setHighThreshold(float scale)
{
    cvConvertScale(IdiffF, Iscratch, scale, 0);
    cvAdd(Iscratch, IavgF, IhiF);
    cvSplit(IhiF, Ihi1, Ihi2, Ihi3, 0);
}

void setLowThreshold(float scale)
{
    cvConvertScale(IdiffF, Iscratch, scale, 0);
    cvSub(IavgF, Iscratch, IlowF);
    cvSplit(IlowF, Ilow1, Ilow2, Ilow3, 0);
}

void backgroundDiff(IplImage *I, IplImage *Imask)
{
    cvCvtScale(I, Iscratch, 1, 0);
    cvCvtColor(Iscratch, Iscratch, CV_BGR2RGB);
    cvSplit(Iscratch, Igray1, Igray2, Igray3, 0);

    // チャンネル1
    cvInRange(Igray1, Ilow1, Ihi1, Imask);
    IplImage *img = cvCreateImage(cvGetSize(Igray1), IPL_DEPTH_8U, 1);
    cvCvtScale(Igray1, img, 1, 0);
//    cvShowImage("1", img);
    
    // チャンネル2
    cvInRange(Igray2, Ilow2, Ihi2, Imaskt);
    cvOr(Imask, Imaskt, Imask);
    cvCvtScale(Igray2, img, 1, 0);
//    cvShowImage("2", img);
    
    // チャンネル3
    cvInRange(Igray3, Ilow3, Ihi3, Imaskt);
    cvOr(Imask, Imaskt, Imask);
    cvCvtScale(Igray3, img, 1, 0);
//    cvShowImage("3", img);
    cvSubRS(Imask, cvScalar(255), Imask);
}

void DeallocateImages()
{
    cvReleaseImage(&IavgF);
    cvReleaseImage(&IdiffF);
    cvReleaseImage(&IprevF);
    cvReleaseImage(&IhiF);
    cvReleaseImage(&IlowF);
    cvReleaseImage(&Ilow1);
    cvReleaseImage(&Ilow2);
    cvReleaseImage(&Ilow3);
    cvReleaseImage(&Ihi1);
    cvReleaseImage(&Ihi2);
    cvReleaseImage(&Ihi3);
    cvReleaseImage(&Iscratch);
    cvReleaseImage(&Iscratch2);
    cvReleaseImage(&Igray1);
    cvReleaseImage(&Igray2);
    cvReleaseImage(&Igray3);
    cvReleaseImage(&Imaskt);
}


/*

#include <cv.h>
#include <highgui.h>
#include <ctype.h>
#include <stdio.h>

int
main (int argc, char **argv)
{
    int i, c, counter;
    int INIT_TIME = 100;
    int w = 0, h = 0;
    double B_PARAM = 1.0 / 50.0;
    double T_PARAM = 1.0 / 200.0;
    double Zeta = 10.0;
    CvCapture *capture = 0;
    IplImage *frame = 0;
    IplImage *av_img, *sgm_img;
    IplImage *lower_img, *upper_img, *tmp_img;
    IplImage *dst_img, *msk_img;
    
    CvFont font;
    char str[64];
    
    // (1)コマンド引数によって指定された番号のカメラに対するキャプチャ構造体を作成する
    capture = cvCreateCameraCapture(0);
    
    // (2)1フレームキャプチャし，キャプチャサイズを取得する．
    frame = cvQueryFrame (capture);
    w = frame->width;
    h = frame->height;
    
    // (3)作業用の領域を生成する
    av_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_32F, 3);
    sgm_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_32F, 3);
    tmp_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_32F, 3);
    lower_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_32F, 3);
    upper_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_32F, 3);
    
    dst_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_8U, 3);
    msk_img = cvCreateImage (cvSize (w, h), IPL_DEPTH_8U, 1);
    
    // (4)背景の輝度平均の初期値を計算する
    printf ("Background statistics initialization start\n");
    
    cvSetZero (av_img);
    for (i = 0; i < INIT_TIME; i++) {
        frame = cvQueryFrame (capture);
        cvAcc (frame, av_img);
    }
    cvConvertScale (av_img, av_img, 1.0 / INIT_TIME);
    
    // (5)背景の輝度振幅の初期値を計算する
    cvSetZero (sgm_img);
    for (i = 0; i < INIT_TIME; i++) {
        frame = cvQueryFrame (capture);
        cvConvert (frame, tmp_img);
        cvSub (tmp_img, av_img, tmp_img);
        cvPow (tmp_img, tmp_img, 2.0);
        cvConvertScale (tmp_img, tmp_img, 2.0);
        cvPow (tmp_img, tmp_img, 0.5);
        cvAcc (tmp_img, sgm_img);
    }
    cvConvertScale (sgm_img, sgm_img, 1.0 / INIT_TIME);
    
    printf ("Background statistics initialization finish\n");
    
    // (6)表示用ウィンドウを生成する
    cvInitFont (&font, CV_FONT_HERSHEY_COMPLEX, 0.7, 0.7);
    cvNamedWindow ("Input", CV_WINDOW_AUTOSIZE);
    cvNamedWindow ("Substraction", CV_WINDOW_AUTOSIZE);
    
    // (7)取得画像から背景を分離するループ
    counter = 0;
    while (1) {
        frame = cvQueryFrame (capture);
        cvConvert (frame, tmp_img);
        
        // (8)背景となりうる画素の輝度値の範囲をチェックする
        cvSub (av_img, sgm_img, lower_img);
        cvSubS (lower_img, cvScalarAll (Zeta), lower_img);
        cvAdd (av_img, sgm_img, upper_img);
        cvAddS (upper_img, cvScalarAll (Zeta), upper_img);
        cvInRange (tmp_img, lower_img, upper_img, msk_img);
        
        // (9)輝度振幅を再計算する
        cvSub (tmp_img, av_img, tmp_img);
        cvPow (tmp_img, tmp_img, 2.0);
        cvConvertScale (tmp_img, tmp_img, 2.0);
        cvPow (tmp_img, tmp_img, 0.5);
        
        // (10)背景と判断された領域の背景の輝度平均と輝度振幅を更新する
        cvRunningAvg (frame, av_img, B_PARAM, msk_img);
        cvRunningAvg (tmp_img, sgm_img, B_PARAM, msk_img);
        
        // (11)物体領域と判断された領域では輝度振幅のみを（背景領域よりも遅い速度で）更新する
        cvNot (msk_img, msk_img);
        cvRunningAvg (tmp_img, sgm_img, T_PARAM, msk_img);
        
        // (12)物体領域のみを出力画像にコピーする（背景領域は黒）
        cvSetZero (dst_img);
        cvCopy (frame, dst_img, msk_img);
        
        // (13)処理結果を表示する
        snprintf (str, 64, "%03d[frame]", counter);
        cvPutText (dst_img, str, cvPoint (10, 20), &font, CV_RGB (0, 255, 100));
        cvShowImage ("Input", frame);
        cvShowImage ("Substraction", dst_img);
        
        counter++;
        
        c = cvWaitKey (10);
        if (c == '\x1b')
            break;
    }
    
    cvDestroyWindow ("Input");
    cvDestroyWindow ("Substraction");
    cvReleaseImage (&frame);
    cvReleaseImage (&dst_img);
    cvReleaseImage (&av_img);
    cvReleaseImage (&sgm_img);
    cvReleaseImage (&lower_img);
    cvReleaseImage (&upper_img);
    cvReleaseImage (&tmp_img);
    cvReleaseImage (&msk_img);
    
    return 0;
}

*/