#include<iostream>
#include<fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include <cmath>
#include <cstring>
#include <OCR.h>
#include <sys/time.h>
#include <unistd.h>
using namespace std;
using namespace cv;
using namespace cv::dnn;

OCR::OCR():                      //输出结果的最大数量
    detector(dbPath),
    recognizer(rePath)
    {
    float binThresh = 0.3;                                      //二值图的置信度阈值
    float polyThresh  = 0.5 ;                                   //文本多边形阈值
    double unclipRatio = 2.0;     //检测到的文本区域的未压缩比率，gai比率确定输出大小
    uint maxCandidates = 200;                           //输出结果的最大数量
    detector.setBinaryThreshold(binThresh)
            .setPolygonThreshold(polyThresh)
            .setUnclipRatio(unclipRatio)
            .setMaxCandidates(maxCandidates);
    detector.getNetwork_().setPreferableBackend(DNN_BACKEND_CUDA);
    detector.getNetwork_().setPreferableTarget(DNN_TARGET_CUDA);
    std::ifstream vocFile;
    vocFile.open(samples::findFile(vocPath));
    CV_Assert(vocFile.is_open());
    String vocLine;
    std::vector<String> vocabulary;
    while (std::getline(vocFile, vocLine)) {
        vocabulary.push_back(vocLine);
    }
    recognizer.setVocabulary(vocabulary);
    recognizer.setDecodeType("CTC-greedy");
    recognizer.getNetwork_().setPreferableBackend(DNN_BACKEND_CUDA);
    recognizer.getNetwork_().setPreferableTarget(DNN_TARGET_CUDA);

    // 设置检测参数
    double detScale = 1.0 / 255.0;
    Size detInputSize = Size(width, height);
    Scalar detMean = Scalar(122.67891434, 116.66876762, 104.00698793);
    detector.setInputParams(detScale, detInputSize, detMean);

    // 设置识别参数
    double recScale = 1.0 / 127.5;
    Scalar recMean = Scalar(127.5);
    Size recInputSize = Size(100, 32);
    recognizer.setInputParams(recScale, recInputSize, recMean);
            
}
        
        
OCR::OCR(cv::dnn::Net detector_net, cv::dnn::Net recognizer_net):                   
    detector(detector_net),
    recognizer(recognizer_net)
    {
    float binThresh = 0.3;                                      //二值图的置信度阈值
    float polyThresh  = 0.5 ;                                   //文本多边形阈值
    double unclipRatio = 2.0;     //检测到的文本区域的未压缩比率，gai比率确定输出大小
    uint maxCandidates = 200;                           //输出结果的最大数量
    detector.setBinaryThreshold(binThresh)
            .setPolygonThreshold(polyThresh)
            .setUnclipRatio(unclipRatio)
            .setMaxCandidates(maxCandidates);
    std::ifstream vocFile;
    vocFile.open(samples::findFile(vocPath));
    CV_Assert(vocFile.is_open());
    String vocLine;
    std::vector<String> vocabulary;
    while (std::getline(vocFile, vocLine)) {
        vocabulary.push_back(vocLine);
    }
    recognizer.setVocabulary(vocabulary);
    recognizer.setDecodeType("CTC-greedy");

    // 设置检测参数
    double detScale = 1.0 / 255.0;
    Size detInputSize = Size(width, height);
    Scalar detMean = Scalar(122.67891434, 116.66876762, 104.00698793);
    detector.setInputParams(detScale, detInputSize, detMean);

    // 设置识别参数
    double recScale = 1.0 / 127.5;
    Scalar recMean = Scalar(127.5);
    Size recInputSize = Size(100, 32);
    recognizer.setInputParams(recScale, recInputSize, recMean);
            
}


void OCR::generate_OCR(const Mat& frame, vector<string> &vstrOCR, vector<vector<int>> &vTextPos) {

    if (frame.empty())
    {
        std::cout << "图像加载失败"  <<std::endl;
        return ;            
    }
    struct timeval t1, t2;
    // 推理
    gettimeofday(&t1, NULL);
    std::vector< std::vector<Point> > detResults;
    cv::cuda::GpuMat GPUframe;
    GPUframe.upload(frame);
    gettimeofday(&t2, NULL);
    cout<<"upload:"<<1000*(t2.tv_sec - t1.tv_sec) + (t2.tv_usec-t1.tv_usec)/1000 << endl;
    gettimeofday(&t1, NULL);
    detector.detect(frame, detResults);
    gettimeofday(&t2, NULL);
    cout<<"detect:" << 1000*(t2.tv_sec - t1.tv_sec) + (t2.tv_usec-t1.tv_usec)/1000 << endl;
    if (detResults.size() > 0) {
        //文本识别
        cv::cuda::GpuMat GPUrecInput;
        if (!imreadRGB) {
            cvtColor(GPUframe, GPUrecInput, cv::COLOR_BGR2GRAY);
        } else {
            GPUrecInput = GPUframe;
        }
        std::vector< std::vector<Point> > contours;
        for (uint i = 0; i < detResults.size(); i++)
        {
            const auto& quadrangle = detResults[i];
            CV_CheckEQ(quadrangle.size(), (size_t)4, "");       //j检测Mat是否为Vector

            contours.emplace_back(quadrangle);                      //插入数据到向量

          /*  cout << quadrangle[0].x << " " << quadrangle[0].y << " " 
                 << quadrangle[1].x << " " << quadrangle[1].y << " " 
                 << quadrangle[2].x << " " << quadrangle[2].y << " " 
                 << quadrangle[3].x << " " << quadrangle[3].y << endl;*/

            std::vector<Point2f> quadrangle_2f;
            for (int j = 0; j < 4; j++)
                quadrangle_2f.emplace_back(quadrangle[j]);

            // 转换和裁剪图像
            Mat cropped;
            Mat recInput;
            GPUrecInput.download(recInput);
            fourPointsTransform(recInput, &quadrangle_2f[0], cropped);

            std::string recognitionResult = recognizer.recognize(cropped);

            // if (quadrangle[0].x < 40 || quadrangle[0].x < 40 || quadrangle[0].x < 40 || quadrangle[0].x < 40 || 
            //     quadrangle[0].x > 600 || quadrangle[1].x > 600 || quadrangle[2].x > 600 || quadrangle[3].x > 600 ||
            //     quadrangle[0].y < 40 || quadrangle[0].y < 40 || quadrangle[0].y < 40 || quadrangle[0].y < 40 ||
            //     quadrangle[0].y > 440 || quadrangle[1].y > 440 || quadrangle[2].y > 440 || quadrangle[3].y > 440) {

            //         putText(frame, recognitionResult, quadrangle[3], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
            //         continue;
            //     }


            if (quadrangle[0].x < edge_threshold || quadrangle[1].x < edge_threshold || quadrangle[2].x < edge_threshold || quadrangle[3].x < edge_threshold || 
                quadrangle[0].x > frame.size[0] - edge_threshold || quadrangle[1].x > frame.size[0] - edge_threshold || quadrangle[2].x > frame.size[0] - edge_threshold || quadrangle[3].x > frame.size[0] - edge_threshold ||
                quadrangle[0].y < edge_threshold || quadrangle[0].y < edge_threshold || quadrangle[0].y < edge_threshold || quadrangle[0].y < edge_threshold ||
                quadrangle[0].y > frame.size[1] - edge_threshold || quadrangle[1].y > frame.size[1] - edge_threshold || quadrangle[2].y > frame.size[1] - edge_threshold || quadrangle[3].y > frame.size[1] - edge_threshold) {

                   // putText(frame, recognitionResult, quadrangle[3], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
                    // std::cout << i << ": '" << recognitionResult << "'" << "Edge Abortion!" << std::endl;
                    continue;
                }

            //putText(frame, recognitionResult, quadrangle[3], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);

            //std::cout << i << ": '" << recognitionResult << "'" << std::endl;

            float center_x = ( quadrangle[0].x + quadrangle[1].x + quadrangle[2].x + quadrangle[3].x ) / 4;
            float center_y = ( quadrangle[0].y + quadrangle[1].y + quadrangle[2].y + quadrangle[3].y ) / 4;




            vstrOCR.push_back(recognitionResult);
            vector<int> temppos;
            temppos.push_back(center_x);
            temppos.push_back(center_y);
            vTextPos.push_back(temppos);
            
            
            
        }
    } else {
        std::cout << "No Text Detected." << std::endl;
    }
}   
void OCR::fourPointsTransform(const Mat& frame, const Point2f vertices[], Mat& result)
{
    const Size outputSize = Size(100, 32);

    Point2f targetVertices[4] = {
        Point(0, outputSize.height - 1),
        Point(0, 0),
        Point(outputSize.width - 1, 0),
        Point(outputSize.width - 1, outputSize.height - 1)
    };
    Mat rotationMatrix = getPerspectiveTransform(vertices, targetVertices);

    warpPerspective(frame, result, rotationMatrix, outputSize);

}


bool OCR::sortPts(const Point& p1, const Point& p2)
{
    return p1.x < p2.x;
}

