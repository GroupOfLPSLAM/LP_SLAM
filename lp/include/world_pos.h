#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<unistd.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <Settings.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <cstring>
#include <numeric>
using namespace std;
using namespace Eigen;
using namespace cv;




Vector3d pixel2camera(double x, double y, double depth);

void LoadGPT(const string &strGPTFilename, vector<vector<int>> &vbGPT);

void generate_world_pos(const Mat& frame, const Mat& depth, vector<float> &vPoseQuat, double mDepthMapFactor, vector<vector<int>> &vTextPos, vector<vector<vector<int>>> &vTextVer, vector<Vector3d> &Text_World_Pos, vector<vector<Vector3d>> &Text_Vertex);
