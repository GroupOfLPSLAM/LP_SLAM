#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<unistd.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <cstring>
#include <numeric>
#include <world_pos.h>
using namespace std;
using namespace Eigen;
using namespace cv;


void generate_world_pos(const Mat& frame, const Mat& depth, vector<float> &vPoseQuat, double mDepthMapFactor, vector<vector<int>> &vTextPos, vector<vector<vector<int>>> &vTextVer, vector<Vector3d> &Text_World_Pos, vector<vector<Vector3d>> &Text_Vertex){
        cv::Mat imRGB, imD;
        imRGB = frame.clone();
        imD = depth.clone();
        // Isometry3d Twc(Quaterniond(vPoseQuat[ni][3], vPoseQuat[ni][4], vPoseQuat[ni][5], vPoseQuat[ni][6]));
        Isometry3d Twc(Quaterniond(vPoseQuat[6], vPoseQuat[3], vPoseQuat[4], vPoseQuat[5]));
        Twc.pretranslate(Vector3d(vPoseQuat[0], vPoseQuat[1], vPoseQuat[2]));


        cv::Mat imgDepth;

        if((fabs(mDepthMapFactor-1.0f)>1e-5) || imD.type()!=CV_32F)
        imD.convertTo(  //将图像转换成为另外一种数据类型,具有可选的数据大小缩放系数
            imgDepth,            //输出图像
            CV_32F,             //输出图像的数据类型
            1/mDepthMapFactor);   //缩放系数


        for (int i = 0; i < vTextPos.size(); i++){
            int x = vTextPos[i][0];
            int y = vTextPos[i][1];
            float depth = imgDepth.at<float>(y, x);
            Vector3d v_c = pixel2camera(x, y, depth);
            Vector3d v_w = Twc * v_c;
            Text_World_Pos.push_back(v_w);
            vector<Vector3d> tmp;
            for (int j = 0; j < 4; j++){
                x = vTextVer[i][j][0];
                y = vTextVer[i][j][1];
                depth = imgDepth.at<float>(y, x);
                v_c = pixel2camera(x, y, depth);
                v_w = Twc * v_c;
                tmp.push_back(v_w);
            }
            Text_Vertex.push_back(tmp);
        }
}



Vector3d pixel2camera(double x, double y, double depth) {

    
    extern struct_setting lp_setting;

    return Vector3d(
            ((x - lp_setting.cx) * depth / lp_setting.fx),
            ((y - lp_setting.cy) * depth / lp_setting.fy),
            depth
    );
}




