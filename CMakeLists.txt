
cmake_minimum_required(VERSION 3.14)
project(ORB_SLAM3)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()

#SET(PADDLE_LIB "/home/ime/Paddle-2.4.1/build/paddle_inference_install_dir")
SET(PADDLE_LIB "*******INSERT*YOUR*PADDLE&PATH*HERE**************")
#SET(OpenCV_DIR "~/opencv-4.7.0/buildnew")
SET(OpenCV_DIR "***INSERT*YOUR*OPENCV*PATH*HERE********")
#SET(LIB_DIR "/home/ime/Paddle-2.4.1/build/paddle_inference_install_dir")
SET(LIB_DIR "*****INSERT*YOUR*PADDLE*LIB*DIR*HERE********")
#SET(CUDA_LIB "/usr/local/cuda-11.4/lib64")
SET(CUDA_LIB "******INSERT*YOUR*CUDA*LIB*DIR*HERE")
#SET(CUDNN_LIB "/usr/lib/x86_64-linux-gnu")
SET(CUDNN_LIB "***********INSERT*YOUR*CUDNN*LIB*DIR*HERE**********")
SET(TENSORRT_DIR "" CACHE PATH "Compile demo with TensorRT")
set(DEMO_NAME "ppocr")
MESSAGE("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -Wall   -O3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall   -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")

# Check C++11 or C++0x support

set(CMAKE_CXX_STANDARD 11)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

find_package(OpenCV 4.7.0 REQUIRED)
   if(NOT OpenCV_FOUND)
      message(FATAL_ERROR "OpenCV > 4.6 not found.")
   endif()

MESSAGE("OPENCV VERSION:")
MESSAGE(${OpenCV_VERSION})

find_package(Eigen3 3.1.0 REQUIRED)
find_package(Pangolin REQUIRED)
find_package(realsense2)
find_package(Boost REQUIRED COMPONENTS
serialization timer thread chrono)
include_directories(
${PROJECT_SOURCE_DIR}
${PROJECT_SOURCE_DIR}/include
${PROJECT_SOURCE_DIR}/include/CameraModels
${PROJECT_SOURCE_DIR}/Thirdparty/Sophus
${EIGEN3_INCLUDE_DIR}
${Pangolin_INCLUDE_DIRS}
include/
/usr/local/lib
lp/include
include/c++/9/
/usr/lib/x86_64-linux-gnu
${Boost_INCLUDE_DIRS}
${Boost_INCLUDE_DIRS}/state/lib/
)

include_directories("${PADDLE_LIB}/paddle/include")
include_directories("${PADDLE_LIB}")
link_directories("${PADDLE_LIB}/paddle/lib")
include_directories(${OpenCV_INCLUDE_DIRS})
message(WARNING ${OpenCV_INCLUDE_DIRS})

include_directories("${PADDLE_LIB}/third_party/install/protobuf/include")
include_directories("${PADDLE_LIB}/third_party/install/glog/include")
include_directories("${PADDLE_LIB}/third_party/install/gflags/include")
include_directories("${PADDLE_LIB}/third_party/install/xxhash/include")
include_directories("${PADDLE_LIB}/third_party/install/zlib/include")
include_directories("${PADDLE_LIB}/third_party/install/onnxruntime/include")
include_directories("${PADDLE_LIB}/third_party/install/paddle2onnx/include")
include_directories("${PADDLE_LIB}/third_party/boost")
include_directories("${PADDLE_LIB}/third_party/eigen3")
include_directories("${PADDLE_LIB}/../../paddle/fluid/inference/api")

include_directories("${CMAKE_SOURCE_DIR}/paddle_ocr/")


link_directories("${PADDLE_LIB}/third_party/install/zlib/lib")

link_directories("${PADDLE_LIB}/third_party/install/protobuf/lib")
link_directories("${PADDLE_LIB}/third_party/install/glog/lib")
link_directories("${PADDLE_LIB}/third_party/install/gflags/lib")
link_directories("${PADDLE_LIB}/third_party/install/xxhash/lib")
link_directories("${PADDLE_LIB}/third_party/install/onnxruntime/lib")
link_directories("${PADDLE_LIB}/third_party/install/paddle2onnx/lib")
link_directories("${PADDLE_LIB}/paddle/lib")

set(MATH_LIB ${PADDLE_LIB}/third_party/install/openblas/lib/libopenblas${CMAKE_STATIC_LIBRARY_SUFFIX})

    set(DEPS
    ${PADDLE_LIB}/paddle/lib/libpaddle_inference${CMAKE_SHARED_LIBRARY_SUFFIX})
set(DEPS ${DEPS}
    ${MATH_LIB} ${MKLDNN_LIB}
    glog gflags protobuf z xxhash
    )
if(EXISTS "${PADDLE_LIB}/third_party/install/snappystream/lib")
    set(DEPS ${DEPS} snappystream)
endif()
if (EXISTS "${PADDLE_LIB}/third_party/install/snappy/lib")
    set(DEPS ${DEPS} snappy)
endif()

set(DEPS ${DEPS} ${CUDA_LIB}/libcudart${CMAKE_SHARED_LIBRARY_SUFFIX})
set(DEPS ${DEPS} ${CUDNN_LIB}/libcudnn${CMAKE_SHARED_LIBRARY_SUFFIX})

set(EXTERNAL_LIB "-ldl -lrt -lgomp -lz -lm -lpthread")
set(DEPS ${DEPS} ${EXTERNAL_LIB})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
LINK_directories("/usr/locla/lib"
${Boost_INCLUDE_DIRS}/state/lib/
)

AUX_SOURCE_DIRECTORY(${PROJECT_SOURCE_DIR}/paddle_ocr/src SRCS)
add_library(${PROJECT_NAME} SHARED
src/System.cc
src/Tracking.cc
src/LocalMapping.cc
src/LoopClosing.cc
src/ORBextractor.cc
src/ORBmatcher.cc
src/FrameDrawer.cc
src/Converter.cc
src/MapPoint.cc
src/KeyFrame.cc
src/Atlas.cc
src/Map.cc
src/MapDrawer.cc
src/Optimizer.cc
src/Frame.cc
src/KeyFrameDatabase.cc
src/Sim3Solver.cc
src/Viewer.cc
src/ImuTypes.cc
src/G2oTypes.cc
src/CameraModels/Pinhole.cpp
src/CameraModels/KannalaBrandt8.cpp
src/OptimizableTypes.cpp
src/MLPnPsolver.cpp
src/GeometricTools.cc
src/TwoViewReconstruction.cc
src/Config.cc
src/Settings.cc
include/System.h
include/Tracking.h
include/LocalMapping.h
include/LoopClosing.h
include/ORBextractor.h
include/ORBmatcher.h
include/FrameDrawer.h
include/Converter.h
include/MapPoint.h
include/KeyFrame.h
include/Atlas.h
include/Map.h
include/MapDrawer.h
include/Optimizer.h
include/Frame.h
include/KeyFrameDatabase.h
include/Sim3Solver.h
include/Viewer.h
include/ImuTypes.h
include/G2oTypes.h
include/CameraModels/GeometricCamera.h
include/CameraModels/Pinhole.h
include/CameraModels/KannalaBrandt8.h
include/OptimizableTypes.h
include/MLPnPsolver.h
include/GeometricTools.h
include/TwoViewReconstruction.h
include/SerializationUtils.h
include/Config.h
include/Settings.h
lp/src/string.cpp
lp/src/is_shop.cpp
lp/src/lp.cpp
lp/src/memory.cpp
lp/src/world_pos.cpp
lp/src/OCR.cpp
lp/include/chat.h
lp/include/memory.h
lp/include/OCR.h
lp/include/world_pos.h
lp/include/lp.h

paddle_ocr/src/args.cpp
paddle_ocr/src/clipper.cpp
paddle_ocr/src/ocr_cls.cpp
paddle_ocr/src/ocr_det.cpp
paddle_ocr/src/ocr_rec.cpp
paddle_ocr/src/paddleocr.cpp
paddle_ocr/src/paddlestructure.cpp
paddle_ocr/src/postprocess_op.cpp
paddle_ocr/src/preprocess_op.cpp
paddle_ocr/src/structure_layout.cpp
paddle_ocr/src/structure_table.cpp
paddle_ocr/src/utility.cpp
paddle_ocr/include/args.h
paddle_ocr/include/clipper.h
paddle_ocr/include/ocr_cls.h
paddle_ocr/include/ocr_det.h
paddle_ocr/include/ocr_rec.h
paddle_ocr/include/paddleocr.h
paddle_ocr/include/paddlestructure.h
paddle_ocr/include/postprocess_op.h
paddle_ocr/include/preprocess_op.h
paddle_ocr/include/structure_layout.h
paddle_ocr/include/structure_table.h
paddle_ocr/include/utility.h
)

add_subdirectory(Thirdparty/g2o)

target_link_libraries(${PROJECT_NAME}
${OpenCV_LIBS}
${EIGEN3_LIBS}
${DEPS}
${Pangolin_LIBRARIES}
${PROJECT_SOURCE_DIR}/Thirdparty/DBoW2/lib/libDBoW2.so
${PROJECT_SOURCE_DIR}/Thirdparty/g2o/lib/libg2o.so
/usr/local/lib
libboost_serialization.so.1.75.0
-lcrypto
libcurl.so
)

# If RealSense SDK is found the library is added and its examples compiled
if(realsense2_FOUND)
    include_directories(${PROJECT_NAME}
    ${realsense_INCLUDE_DIR}
    )
    target_link_libraries(${PROJECT_NAME}
    ${realsense2_LIBRARY}
    )
endif()


# Build examples

# RGB-D examples


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/Examples/RGB-D)
add_executable(rgbd_tum
        Examples/RGB-D/rgbd_tum.cc)
target_link_libraries(rgbd_tum ${PROJECT_NAME} 
libcurl.so 
${OpenCV_LIBS}
${DEPS})





