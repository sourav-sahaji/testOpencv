#include<stdio.h>
#include<iostream>
#include<fstream>

#include<opencv2/highgui.hpp>

#include <gtsam/geometry/Pose3.h>
#include <gtsam/geometry/Pose2.h>
#include <gtsam/geometry/Unit3.h>

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
    for(int fileNum=0; fileNum<2; fileNum++)
    {
        // Create the output file
        char outFileName[100];
        std::sprintf(outFileName,"%02d-2dpose.txt",fileNum);
        std::ofstream outFile(outFileName);

        // Read the poses ground truth
        char inFileName[200];
        std::sprintf(inFileName,"/home/sourav/workspace/ORB_SLAM2/Examples/Monocular/results-1/pa/round-3/skip-4400/kfTrajFile-2-%d.txt",fileNum);
        std::cout << "Reading file " << inFileName << std::endl;
        std::ifstream inFile(inFileName);
        // or
//        std::ifstream inFile("kfTrajFile-2-0.txt");

        cv::Mat poseData = readDataFile(inFile,' ');
        poseData.convertTo(poseData,CV_64FC1);
        std::cout << "Matrix Size = " << poseData.rows << " x " << poseData.cols << std::endl;

        // Convert the pose into linear and rotational velocity

//        // Convert data into 4x4 matrix
//        std::vector<cv::Mat> poses4x4;
//        for(int i1=0; i1<poseData.rows; i1++)
//        {
//            cv::Mat p4x4(4,4,CV_64FC1);
//            for(int j1=0; j1<poseData.cols; j1++)
//            {
//                p4x4 = poseData.row(i1).reshape(0,3).clone();
//                double lastRowData[4] = {0,0,0,1.0};
//                cv::Mat lastRow(1,4,CV_64FC1,lastRowData);
//                p4x4.push_back(lastRow);
//            }
//            poses4x4.push_back(p4x4);
//        }

//        // Convert opencv Mat into GTSAM SE3/SE2 pose
//        for(int i1=0; i1<poses4x4.size(); i1++)
        for(int i1=0; i1<poseData.rows;i1++)
        {
//            //        cout << poses4x4[i1] << endl;
//            // Get the R matrix
//            std::vector<gtsam::Point3> points(3);
//            for(int j1=0; j1<3; j1++)
//            {
//                points[j1] = gtsam::Point3(
//                            poses4x4[i1].row(j1).at<double>(0),
//                            poses4x4[i1].row(j1).at<double>(1),
//                            poses4x4[i1].row(j1).at<double>(2)
//                            );
//            }
//            gtsam::Rot3 r3(points[0],points[1],points[2]);

//            // Get the T matrix
//            gtsam::Point3 t3(
//                        poses4x4[i1].col(3).at<double>(0),
//                        poses4x4[i1].col(3).at<double>(1),
//                        poses4x4[i1].col(3).at<double>(2)
//                        );

            // Convert quaternions into r
            double qw = poseData.at<double>(i1,7);
            double qx = poseData.at<double>(i1,4);
            double qy = poseData.at<double>(i1,5);
            double qz = poseData.at<double>(i1,6);

            double tx = poseData.at<double>(i1,1);
            double ty = poseData.at<double>(i1,2);
            double tz = poseData.at<double>(i1,3);


            gtsam::Quaternion q1(qw,qx,qy,qz);
            gtsam::Rot3 r3(q1);
            gtsam::Point3 t3(tx,ty,tz);

            gtsam::Pose3 poseGtsam(r3,t3);
            gtsam::Vector3 rpy = poseGtsam.rotation().rpy();

            // Get the SE2 pose (2D part of pose, assuming no motion in y-direction (perpendicular to ground plane) )
            double pitch = rpy[1];
            double z = t3.z();
            double x = t3.x();

            outFile << poseData.at<double>(i1,0) << " " << z << " " << x << " " << pitch << std::endl;

//            gtsam::Pose2 pose2d(z,x,pitch);
//            gtsam::Matrix mat = pose2d.matrix();
//            for(int k3=0; k3<6; k3++)
//                outFile << mat.coeffRef(k3) << " ";
//            outFile << std::endl;

        }
    }
}
