#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include<unistd.h>
#include <mutex>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <cmath>
#include <cstring>
#include <thread>
#include <numeric>
#ifndef _MEMORY_H
#define _MEMORY_H
#include <memory.h>
#endif
#ifndef _LP_CHAT_H
#define _LP_CHAT_H
#include <chat.h>
#endif
#include <world_pos.h>

#include <sys/time.h>
#include <OCR.h>

#include <include/args.h>
#include <include/paddleocr.h>
#include <include/paddlestructure.h>
using namespace std;
using namespace Eigen;
using namespace cv;
using namespace cv::dnn;




int lp(std::thread * wheretogo, std::thread *threadGPT);
