#include <lp.h>
using namespace std;
using namespace Eigen;
using namespace cv;
using namespace cv::dnn;
using namespace PaddleOCR;


    vector<LandmarkShort> ShortMemory;
vector<LandmarkLong> LongMemory;
vector<LandmarkLong> MemoryForGPT;
vector<LandmarkMap> TextMap;
vector<LandmarkComb> CombMap;
std::mutex mtxlp;
std::mutex mtxmap;
Mat lpimg;
Mat lpdepth;
double lptime;
vector<float> lpvPoseQuat;
bool lpisnew = false;
bool lpshutdown = false;
std::mutex mtxshutdown;
std::mutex mtxGPT;



int lp(std::thread * wheretogo, std::thread * threadGPT){
    vector<string> vstrImageFilenamesRGB;
    vector<string> vstrImageFilenamesD;
    vector<double> vTimestamps;
    vector<float> vPoseQuat;
    extern struct_setting lp_setting;
    //关联文件的路径
    float mDepthMapFactor = lp_setting.depthMapFactor;
    //从关联文件中加载这些信息

    int nImages = vstrImageFilenamesRGB.size();
    
    /*
    vector<string> ShortMemoryName;
    vector<vector<Vector3d>> ShortMemoryPos;
    vector<int> ShortMemoryNum;//-1 means it has turned into long memory,0 means totally forgotton
    */
//***************************************************************************************
//setting for ocr
/*
    cv::dnn::Net detector_net = cv::dnn::readNetFromONNX("./lp/OCR/DB_TD500_resnet50.onnx");
    cv::dnn::Net recognizer_net = cv::dnn::readNetFromONNX("./lp/OCR/crnn_cs_CN.onnx");
    detector_net.setPreferableBackend(DNN_BACKEND_CUDA);
    detector_net.setPreferableTarget(DNN_TARGET_CUDA);
    recognizer_net.setPreferableBackend(DNN_BACKEND_CUDA);
    recognizer_net.setPreferableTarget(DNN_TARGET_CUDA);
    
    OCR myOCR(detector_net, recognizer_net);*/
    
    
    //google::ParseCommandLineFlags(&argc, &argv, true);
    
    
    
    //heavy model
    FLAGS_det_model_dir = "./paddle_ocr/inference/ch_ppocr_server_v2.0_det_infer";
    FLAGS_rec_model_dir="./paddle_ocr/inference/ch_ppocr_server_v2.0_rec_infer";
    FLAGS_cls_model_dir="./paddle_ocr/inference/ch_ppocr_mobile_v2.0_cls_infer";
    
    /*
    //super-light model
    FLAGS_det_model_dir = "./paddle_ocr/inference/ch_PP-OCRv2_det_infer";
    FLAGS_rec_model_dir="./paddle_ocr/inference/ch_PP-OCRv2_rec_infer";
    FLAGS_cls_model_dir="./paddle_ocr/inference/ch_ppocr_mobile_v2.0_cls_infer";*/
    
    
    PPOCR myOCR = PPOCR();
    
    
//*********************************************************************************    
    
    GPT_API myGPT;
    bool first = true;
    
    int edge_threshold = 10;
    wheretogo = new thread(&GPT_API::where_to_go, myGPT);
    threadGPT = new thread(&GPT_API::GPT_work, myGPT);



    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
    double tmptime;
//*******************************************************************************
//main loop
    while(1){
        mtxshutdown.lock();
        if (lpshutdown) {

                cout << "******************SHORTBELOW***********"<<endl;
                int cnt = 1;
                int ccnt = 0;
                for (int i=0; i < ShortMemory.size(); i++){
                    cout <<cnt << ":" << endl; 
                    cnt ++;
                    cout <<  "text && num:" << endl;
                    for (int j=0; j < ShortMemory[i].name.size(); j++){
                        cout << "\t\t" << ShortMemory[i].name[j].c_str()<< "\t\t" << ShortMemory[i].num[j] << endl;
                        ccnt += ShortMemory[i].num[j];
                    }
                        
                }
                cout << "TOTALSHORT =" << ccnt;
                 cout <<"**************LONGBELOW***************"<<endl;
                cnt = 1;
                for (int i=0; i < LongMemory.size(); i++){
                    int max_num = 0;
                    for (int j = 0; j < ShortMemory[LongMemory[i].shortvec].num.size(); j++){
                        if (max_num < ShortMemory[LongMemory[i].shortvec].num[j])
                            max_num = ShortMemory[LongMemory[i].shortvec].num[j];
                    }
                    if (max_num < 10) continue;
                    cout <<cnt << ":" << endl; 
                    cnt ++;
                    cout <<  "text:" << endl;
                    for (int j=0; j < ShortMemory[LongMemory[i].shortvec].name.size(); j++){
                       // if (ShortMemory[LongMemory[i].shortvec].num[j] < 5) continue;
                        cout << "\t\t" << ShortMemory[LongMemory[i].shortvec].name[j].c_str() << "\t\t" << ShortMemory[LongMemory[i].shortvec].num[j] << endl;
                    }
                        
                }
            break;
        }
        mtxshutdown.unlock();
        mtxlp.lock();
        if (!lpisnew){
            mtxlp.unlock();

		    std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        vPoseQuat.assign( lpvPoseQuat.begin(), lpvPoseQuat.end());
        tmptime =  lptime;
        Mat frame = lpimg.clone();
        Mat depth = lpdepth.clone();
        lpisnew = false;
        mtxlp.unlock();
        
        
        
        vector<Vector3d> Text_World_Pos;
        vector<vector<Vector3d>> Text_Vertex;
        
        //cout << image_path << endl;
        //cout << OCR_name << endl;

        vector<string> vstrOCR;
        vector<vector<int>> vTextPos;
        vector<vector<vector<int>>> vTextVer;//n*4*2
        //***********************************************************************
        //some ocr
        //myOCR.generate_OCR(frame, vstrOCR, vTextPos);  
        std::vector<OCRPredictResult> ocr_result = myOCR.ocr(frame, FLAGS_det, FLAGS_rec, FLAGS_cls);
       
        for (int i = 0; i < ocr_result.size(); i++){
        if (ocr_result[i].box[0][0] < edge_threshold || ocr_result[i].box[1][0] < edge_threshold || ocr_result[i].box[2][0] < edge_threshold || ocr_result[i].box[3][0] < edge_threshold || 
                ocr_result[i].box[0][0] > frame.size[1] - edge_threshold || ocr_result[i].box[1][0] > frame.size[1] - edge_threshold || ocr_result[i].box[2][0] > frame.size[1] - edge_threshold || ocr_result[i].box[3][0] > frame.size[1] - edge_threshold ||
                ocr_result[i].box[0][1] < edge_threshold || ocr_result[i].box[1][1] < edge_threshold || ocr_result[i].box[2][1] < edge_threshold || ocr_result[i].box[3][1] < edge_threshold ||
                ocr_result[i].box[0][1] > frame.size[0] - edge_threshold || ocr_result[i].box[1][1] > frame.size[0] - edge_threshold || ocr_result[i].box[2][1] > frame.size[0] - edge_threshold || ocr_result[i].box[3][1] > frame.size[0] - edge_threshold) {

                   // putText(frame, recognitionResult, quadrangle[3], FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
                    // std::cout << i << ": '" << recognitionResult << "'" << "Edge Abortion!" << std::endl;
                    continue;
                }
        
            //cout << "&" << ocr_result[i].text << endl;
            vstrOCR.push_back(ocr_result[i].text);
            std::vector<int> tmp;
            std::vector<int> tmppos;
            std::vector<std::vector<int>> tmpver;
            tmppos.push_back((int)((float)ocr_result[i].box[0][0] + (float)ocr_result[i].box[1][0] + (float)ocr_result[i].box[2][0] + (float)ocr_result[i].box[3][0])/4);
            tmppos.push_back((int)((float)ocr_result[i].box[0][1] + (float)ocr_result[i].box[1][1] + (float)ocr_result[i].box[2][1] + (float)ocr_result[i].box[3][1])/4);
            vTextPos.push_back(tmppos);
            double tt = 1;
            tmp.push_back(tmppos[0] + tt * (ocr_result[i].box[0][0] - tmppos[0]));
            tmp.push_back(tmppos[1] + tt * (ocr_result[i].box[0][1] - tmppos[1]));
            tmpver.push_back(tmp);
            tmp.clear();
            tmp.push_back(tmppos[0] + tt * (ocr_result[i].box[1][0] - tmppos[0]));
            tmp.push_back(tmppos[1] + tt * (ocr_result[i].box[1][1] - tmppos[1]));
            tmpver.push_back(tmp);
            tmp.clear();
            tmp.push_back(tmppos[0] + tt * (ocr_result[i].box[2][0] - tmppos[0]));
            tmp.push_back(tmppos[1] + tt * (ocr_result[i].box[2][1] - tmppos[1]));
            tmpver.push_back(tmp);
            tmp.clear();
            tmp.push_back(tmppos[0] + tt * (ocr_result[i].box[3][0] - tmppos[0]));
            tmp.push_back(tmppos[1] + tt * (ocr_result[i].box[3][1] - tmppos[1]));
            tmpver.push_back(tmp);
            tmp.clear();
            vTextVer.push_back(tmpver);
        }
        
        if (vstrOCR.empty()) continue;
        
        
        
        //*************************************************************************
        //other things
        generate_world_pos(frame, depth, vPoseQuat, mDepthMapFactor, vTextPos, vTextVer, Text_World_Pos, Text_Vertex);
        
        


        
        GenerateShortMemory(myGPT, ShortMemory, vstrOCR, Text_World_Pos, Text_Vertex, tmptime);
        
        ForgetProcess(ShortMemory);
        

    
    }
    wheretogo -> join();
    threadGPT -> join();
    
}














