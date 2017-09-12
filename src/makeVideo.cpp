// One of the versions of displayLabelledVideo.cpp from sequenceSLAM package
// Writes a video file from the raw images
// Some of the images weren't readable, hence number of frames might be different in each season

#include<stdio.h>
#include<iostream>
#include<fstream>

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

/*!
@brief Reads input data file into OpenCV matrix
@return Returns the Matrix
*/
cv::Mat readDataFile(std::ifstream& inputFile, char delimiter)
{
    cv::Mat dataMat;
    std::string buffer;
    while(std::getline(inputFile,buffer))
    {
        std::stringstream ss(buffer);
        std::string elem;
        cv::Mat rowMat;
        while(std::getline(ss,elem,delimiter))
        {
            rowMat.push_back(atof(&elem[0]));
        }

        if(dataMat.empty())
        {
            dataMat.push_back(rowMat);
            dataMat = dataMat.t();
        }
        else
        {
            cv::vconcat(dataMat,rowMat.t(),dataMat);
        }
    }
    return dataMat;
}

int main()
{
    int pausePlay = 1;
//    cv::VideoCapture cap1("../../dataset/tunnel-videos/16.mp4");
//    cv::VideoCapture cap1("../../sequenceSLAM/bin/indoor-outdoor-2.mp4");

    cv::VideoWriter writer1("winter.avi",CV_FOURCC('H','2','6','4'),25,cv::Size(640,360));

    int imgCounter = 0;
    int numImages = 27010;//cap1.get(CV_CAP_PROP_FRAME_COUNT);

    std::ifstream labelsFile("predicted_labels-16.txt");
    cv::Mat labels = readDataFile(labelsFile,' ');
    std::cout << "Labels size - " << labels.rows << " x " << labels.cols << std::endl;

    while(imgCounter < numImages) // 5700 snipPoint for 1, 7600 for 16
    {
        char imgName[2000];
//        sprintf(imgName,"/media/sourav/My Passport/slamDatasets/nordland/640x320-color-1fps/spring/images-%05d.png",imgCounter);
        sprintf(imgName,"/home/sourav/dataset/nordland/winter-part1/images-%05d.png",imgCounter);

        cv::Mat img1;
//        cap1 >> img1;

        img1 = cv::imread(imgName);

        if(img1.empty())
        {
//            std::cout << "Read the video with " << imgCounter << " frames" << std::endl;
//            break;
            imgCounter++;
            continue;
        }

//        int label = labels.at<double>(imgCounter);
        imgCounter++;
        if(imgCounter%1000 == 0)
            std::cout << imgCounter << /*" " << label <<*/ std::endl;

//        if(imgCounter < 1980) // 6600 for 1
//            continue;

//        char txt[50];
//        sprintf(txt,"Label - %d",label);
//        cv::putText(img1,txt,cv::Point2i(50,50),2,2,cv::Scalar(0,0,255));
//        sprintf(txt,"Counter - %d",imgCounter);
//        cv::putText(img1,txt,cv::Point2i(50,100),2,2,cv::Scalar(0,0,255));

//        cv::Rect roi(0,0,540,360);
//        img1 = img1(roi).clone();

//        cv::resize(img1,img1,cv::Size(180,90));

        writer1.write(img1);
//        cv::imshow("img1",img1);
        char key = cv::waitKey(pausePlay);
        if(key == 'P' || key == 'p')
            pausePlay = !pausePlay;
    }

    writer1.release();
}
