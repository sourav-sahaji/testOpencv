#include<stdio.h>
#include<iostream>
#include<fstream>

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

void warpLinear(Mat& img1, Mat& img1Warped, int& xShift, int w1, int h1);
void warpRot(Mat& img1, Mat& img1Warped, int& rot, int w1, int h1);

int main()
{
    // Read the list of video files
    ifstream srcFile("../bin/srcList.txt");
    if(!srcFile.is_open())
    {
        cerr << "Source file not found" << endl;
        exit(-1);
    }

    vector<string> srcVids;
    string buffer;
    while(getline(srcFile,buffer))
    {
        stringstream ss(buffer);
        srcVids.push_back(ss.str());
    }

    ofstream labelsFile("../bin/labels.txt");

    // Set the transalational offset
    int xShift = 15;
    int rot = 0;
    // Output labels
    Mat labels;
    // Number of images from each video
    int maxImagesPerVideo = 200;
    // Size of cropping window
    int w1 = 256, h1 = 256;

    // Capture the videos one by one
    for(int i1=0; i1<srcVids.size(); i1++)
    {
        cout << "Reading video sequence: \n" << srcVids[i1] << endl;

        VideoCapture cap1(srcVids[i1]);
        int numFrames = cap1.get(CV_CAP_PROP_FRAME_COUNT);
        cout << "Number of frames in this video = " << numFrames << endl;

        // Uniformly sample the frames in video,
        // i.e., skip frames based on total video frames and max frames to be extracted
        int skip = numFrames / maxImagesPerVideo;
        int frameCount = -1;
        // Get the images
        for(int j1=0; j1<maxImagesPerVideo && j1 < numFrames; j1++)
        {
            frameCount++;
            Mat img1, img1Warped;

            cap1 >> img1;

            if(img1.empty())
            {
                cerr << "No image found" << endl;
                break;
            }

            frameCount += skip;
            cap1.set(CV_CAP_PROP_POS_FRAMES,frameCount);

//            warpLinear(img1,img1Warped,xShift,w1,h1);
            warpRot(img1,img1Warped,rot,w1,h1);

            char imgName1[20],imgName2[20];
            sprintf(imgName1,"../bin/train/imgRef-%04d.png",labels.rows);
            sprintf(imgName2,"../bin/train/imgWarp-%04d.png",labels.rows);

            imshow("img1",img1);
            imshow("img2",img1Warped);
            waitKey(1);

            // Preprocessing before writing images
            cvtColor(img1,img1,CV_BGR2GRAY);
            cvtColor(img1Warped,img1Warped,CV_BGR2GRAY);

            resize(img1,img1,Size(32,32));
            resize(img1Warped,img1Warped,Size(32,32));

            imwrite(imgName1,img1);
            imwrite(imgName2,img1Warped);

            // Hard code - change!!!!!
            labels.push_back(rot-90);
            labelsFile << rot-90 << endl;
        }
    }
}

void warpLinear(Mat& img1, Mat& img1Warped, int& xShift, int w1, int h1)
{
    RNG range(getTickCount());

    // Create a rect window for random crop
    int x1 = range.uniform(0,img1.cols-w1);
    int y1 = range.uniform(0,img1.rows-h1);
    if(x1<xShift)
        x1+=xShift;
    Rect roi(x1,y1,w1,h1);

    // Get the source roi
    Mat img1Roi = img1(roi);

    // Create a transformation matrix
    float matData[6] = {1.0, 0.0, xShift, 0.0, 1.0, 0.0};
    Mat Xform(2,3,CV_32FC1,matData);

    // Apply transformation
    warpAffine(img1Roi,img1Warped,Xform,Size(img1Roi.cols,img1Roi.rows));

    // Fill the empty part of warped image with original image
    Rect fillRoi(roi.x - xShift,roi.y,xShift,roi.height);
    Mat fillPart = img1(fillRoi);//.rowRange(imgCenter.y-128,imgCenter.y+128).colRange(imgCenter.x-128-xShift,imgCenter.x-128);
    fillPart.copyTo(img1Warped.colRange(0,xShift));

    // Shift for certain discrete values
    xShift += 20;
    if(xShift > 80)
        xShift = 20;

    img1Roi.copyTo(img1);
}


void warpRot(Mat& img1, Mat& img1Warped, int& rot, int w1, int h1)
{
    RNG range(getTickCount());

    // Create a rect window for random crop
    int x1 = range.uniform(0,img1.cols-w1);
    int y1 = range.uniform(0,img1.rows-h1);

    Rect roi(x1,y1,w1,h1);

    // Get the source roi
    Mat img1Roi = img1(roi);

    switch (rot) {
    case 90:
        transpose(img1Roi,img1Warped);
        flip(img1Warped,img1Warped,1);
        break;
    case 180:
        flip(img1Roi,img1Warped,-1);
        break;
    case 270:
        transpose(img1Roi,img1Warped);
        flip(img1Warped,img1Warped,0);
        break;
    default:
        img1Warped = img1Roi.clone();
        break;
    }


    // Shift for certain discrete values
    rot += 90;
    if(rot == 360)
        rot = 0;

    img1Roi.copyTo(img1);
}
