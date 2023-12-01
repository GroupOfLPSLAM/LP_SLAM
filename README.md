# LP-SLAM

# 1. INTRODUCTION
LP-SLAM is a language-perceptive RGB-D SLAM based on ORB-SLAM3. So make sure ORB-SLAM3 works before you try LP-SLAM. 

# 2. Prerequisites
Ubuntu 20.04 is recommanded. A real-time OCR thread will be running during the SLAM, so there should be a graphic card in your computer. We run the LP-SLAM on Nvidia GeForce RTX3080ti.


## C++11 or C++0x Compiler
We use the new thread and chrono functionalities of C++11.


## Pangolin
**Required  0.6.0**.
## OpenCV
We use [OpenCV](http://opencv.org) to manipulate images and features. Dowload and install instructions can be found at: http://opencv.org.**Tested with OpenCV 4.7.0**.
When compile opencv, make sure you turn on flags such as with-cuda . You should downloaad and build opencv_contrib and clair its path while building opencv. We use opencv-contrib 4.7.0. 



## Eigen3
Required by g2o (see below). Download and install instructions can be found at: http://eigen.tuxfamily.org. **Required at least 3.1.0**.

## Boost
**Required  1.75.0**.

## DBoW2 and g2o (Included in Thirdparty folder)
We use modified versions of the [DBoW2](https://github.com/dorian3d/DBoW2) library to perform place recognition and [g2o](https://github.com/RainerKuemmerle/g2o) library to perform non-linear optimizations. Both modified libraries (which are BSD) are included in the *Thirdparty* folder.

## Python
Required to calculate the alignment of the trajectory with the ground truth. **Required Numpy module**.

## libcurl
Required to access the API of ChatGPT. You should also aply for an API code of the chatgpt. If you are in China Mainland, you should also get a VPN.

sudo apt-get install libcurl4-openssl-dev

After install libcurl, you should located your libcurl.so in your computer, and include_directories in CMakeLists.txt

## GPU driving, CUDA and CUDNN
the  version depends on your graphic card. 

## paddlepaddle & paddleOCR
Required to recognize words in scene. 

The link of paddlepaddle in github: https://github.com/PaddlePaddle/Paddle. Build it. We use release version 2.4.1 of paddlepaddle. Remember to turn -DWITH_GPU=ON if you want to run real-time LP-SLAM.

The link of paddleOCR in github:  https://github.com/PaddlePaddle/PaddleOCR. We use 2.6 version. Don't build it. Your should copy PaddleOCR/deploy/cpp_infer into LP-SLAM/paddle_ocr. We will build paddleocr with lp_slam together. 

# 3.Building LP-SLAM

1. Clone the repository
```
https://github.com/GroupOfLPSLAM/LP_SLAM.git
```

2. build thirdparty
We edit the script 'build.sh' to build the Thirdparty libraries. Please make sure you have installed all required dependencies.Execute:
```
cd ORB_SLAM3
chmod +x build.sh
./build.sh
```
3. edit cmakelist.txt
There are several places asking you to replace your lib path in CMakeLists.txt. Replace them.

4. edit openai api key
In lp_slam/lp/include/chat.h, you should fill in your openai api key in #define API_KEY=""

# 4. Running LP-SLAM with dataset
LP_SLAM only works in RGBD mode.
```
cd LP_SLAM
./Examples/RGB-D/rgbd_tum Vocabulary/ORBvoc.txt Examples/RGB-D/***.yaml pathofyourdataset pathofyourassociate.txt
```


By the way, wchar like chinese charactor is not support in pangolin. they will be replaced with "**************"
