#include<iostream>
#include<fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn/dnn.hpp>
#include "opencv2/imgcodecs.hpp"
#include <cstring>
#include <numeric>
#include <opencv2/imgproc.hpp>
using namespace std;
using namespace cv;
using namespace cv::dnn;

class OCR{
public:


    String vocPath = "./lp/OCR/alphabet_3944.txt";              //字典文件
    String dbPath = "./lp/OCR/DB_TD500_resnet50.onnx";
    String rePath = "./lp/OCR/crnn_cs_CN.onnx";
    
    
private:
    int height = 480;                                                   //输出图片长宽
    int width = 640;
    
    int edge_threshold = 40;
    
    int imreadRGB = 1;         //0：以灰度图读取图像   1：以彩色图读取图像
    TextDetectionModel_DB detector;//DB模型权重文件
    TextRecognitionModel recognizer; //文字识别模型文件
public:
    OCR();
    OCR(cv::dnn::Net detector_net, cv::dnn::Net recognizer_net);
    void generate_OCR(const Mat& frame, vector<string> &vstrOCR, vector<vector<int>> &vTextPos);
    bool sortPts(const Point& p1, const Point& p2);
    void fourPointsTransform(const Mat& frame, const Point2f vertices[], Mat& result);
};





