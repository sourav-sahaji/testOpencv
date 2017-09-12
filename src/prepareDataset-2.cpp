#include<stdio.h>
#include<iostream>
#include<fstream>

#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>

using namespace std;
using namespace cv;

/*!
@brief Reads input data file into OpenCV matrix
@return Returns the Matrix
*/
Mat readDataFile(ifstream& inputFile, char delimiter)
{
    Mat dataMat;
    string buffer;
    while(getline(inputFile,buffer))
    {
        stringstream ss(buffer);
        string elem;
        Mat rowMat;
        while(getline(ss,elem,delimiter))
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
            vconcat(dataMat,rowMat.t(),dataMat);
        }
    }
    return dataMat;
}


int main()
{
    // Initiate global counter
    int gc = 0;

    // Labels file
    std::ofstream labelsFileRaw("labelsRaw.txt");
    std::ofstream labelsFileProc("labelsProc.txt");

    for(int fileNum=9; fileNum<11; fileNum++)
    {
        // R distance between frames
        char outFileName[200];
        std::sprintf(outFileName,"%02d-r.txt",fileNum);
        std::ofstream outFile(outFileName);

        // Read the poses ground truth
        char inFileName[200];
        std::sprintf(inFileName,"/home/sourav/dataset/KITTY/2d-poses/%02d-2dpose.txt",fileNum);
        std::cout << "Reading file " << inFileName << std::endl;
        std::ifstream inFile(inFileName);

        cv::Mat poseData = readDataFile(inFile,' ');
        std::cout << "Matrix Size = " << poseData.rows << " x " << poseData.cols << std::endl;

        std::cout << "Filtering data..." << endl;
        // Filter data corresponding to pure translational motion
//        cv::Mat pureTransMask = cv::Mat::zeros(poseData.rows,poseData.cols,CV_8UC1);
//        for(int i1=1; i1<poseData.rows; i1++)
//        {
//            double zOld = poseData.at<double>(i1-1,1);
//            double xOld = poseData.at<double>(i1-1,2);
//            double pitchOld = poseData.at<double>(i1-1,3);

//            double z = poseData.at<double>(i1,1);
//            double x = poseData.at<double>(i1,2);
//            double pitch = poseData.at<double>(i1,3);

//            if(abs(pitch-pitchOld) > 0.005)
//            {
//                pureTransMask.at<uchar>(i1) = true;
//            }
//        }

        std::cout << "Generating R values..." << endl;
        // Generating distance values between current and next few frames (now at constant theta due to filtering)
        for(int i1=1; i1<poseData.rows; i1++)
        {
//            if(pureTransMask.at<uchar>(i1))
//                continue;
//            else
            {
                double zOld = poseData.at<double>(i1-1,1);
                double xOld = poseData.at<double>(i1-1,2);
                double pitchOld = poseData.at<double>(i1-1,3);

                // Calculate r for next 10 frames
                for(int j1=0; j1<10 && i1+j1<poseData.rows; j1++)
                {
                    double z = poseData.at<double>(i1+j1,1);
                    double x = poseData.at<double>(i1+j1,2);
                    double pitch = poseData.at<double>(i1+j1,3);

                    double r = std::sqrt( pow((z-zOld),2) + pow((x-xOld),2) );
                    double theta = pitch - pitchOld;

                    outFile << i1-1 << " " << i1+j1 << " " << r << " " << theta << endl;

                    if(r < 6)
                    {
                        // Process label to match it to array [0.5 1.0 ... 5.5 6.0]
                        int label = std::floor( ( (r * 10.0) + 2.0 ) / 5.0 );
                        int labelTheta = round( (theta * 50.0));

                        if(label == 0)
                            continue;

                        labelsFileProc << label << " " << labelTheta << std::endl;
                        labelsFileRaw << r << " " << theta << std::endl;

                        char inImgName[200];
                        std::sprintf(inImgName,"/home/sourav/dataset/KITTY/sequences/%02d/image_0/%06d.png",fileNum,i1-1);
                        cv::Mat img1=imread(inImgName);

                        std::sprintf(inImgName,"/home/sourav/dataset/KITTY/sequences/%02d/image_0/%06d.png",fileNum,i1+j1);
                        cv::Mat img2=imread(inImgName);

                        // Some adequate cropping
                        cv::Rect roi(243,0,740,370);
                        img1 = img1(roi).clone();
                        img2 = img2(roi).clone();

                        // BGR to Gray
                        cv::cvtColor(img1,img1,CV_BGR2GRAY);
                        cv::cvtColor(img2,img2,CV_BGR2GRAY);

                        // Some resizing
                        cv::resize(img1,img1,cv::Size(128,64));
                        cv::resize(img2,img2,cv::Size(128,64));

                        std::sprintf(inImgName,"../bin/train/imgRef-%08d.png",gc);
                        cv::imwrite(inImgName,img1);

                        std::sprintf(inImgName,"../bin/train/imgWrp-%08d.png",gc);
                        cv::imwrite(inImgName,img2);

                        gc++;

#if(VISUALS==1)
                        // Visualize the images filtered above
                        if(theta > 0.1)
                        {
                            cv::imshow("img1",img1);
                            cv::imshow("img2",img2);
                            cv::waitKey(0);
                        }
#endif
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}
