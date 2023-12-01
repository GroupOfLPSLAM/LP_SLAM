#include<iostream>
#include<algorithm>
#include<chrono>
#include<unistd.h>
#include <cmath>
#include <cstring>
#include <memory.h>
#include <lp.h>
extern vector<LandmarkLong> LongMemory;
extern vector<LandmarkMap> TextMap;
extern std::mutex mtxmap;
extern std::mutex mtxGPT;
extern vector<LandmarkLong> MemoryForGPT;
using namespace Eigen;
using namespace std;



void GenerateShortMemory(GPT_API &myGPT, vector<LandmarkShort> &ShortMemory, vector<string> &MomentName, vector<Vector3d> &MomentPos, vector<vector<Vector3d>> &MomentVertex, double tmptime){
    for (int iMoment = 0; iMoment < MomentName.size(); iMoment++){
        if (MomentName[iMoment].length() < 2) continue;//short name are seen as recognization error
        bool is_new = true;
        int iShort;
        for (iShort = 0; iShort < ShortMemory.size(); iShort++)
            if (!IsLandmarkNew(ShortMemory, MomentName, MomentPos, iShort, iMoment)){
                is_new = false;
                break;
            }
        if (is_new){//see a new ocr result that is totally different with those beforee
            LandmarkShort templandmark;
            templandmark.time = tmptime;
            templandmark.name.push_back( MomentName[iMoment]);
            templandmark.memory_score = MEMORYSPEED;
            templandmark.num.push_back(1) ;
            templandmark.all_pos.push_back(MomentPos[iMoment]);
            templandmark.vertex.push_back(MomentVertex[iMoment]);
            ShortMemory.push_back(templandmark);
        }
        else{//see an ocr result that has already in short memory        
            vector<string>::iterator it = find (ShortMemory[iShort].name.begin(), ShortMemory[iShort].name.end(), MomentName[iMoment]);
            if (it == ShortMemory[iShort].name.end()){
                    ShortMemory[iShort].name.push_back(MomentName[iMoment]);
                    ShortMemory[iShort].num.push_back(1);
            }else{
                ShortMemory[iShort].num[it - ShortMemory[iShort].name.begin()] += 1;

            }
                
            ShortMemory[iShort].all_pos.push_back(MomentPos[iMoment]);
            ShortMemory[iShort].vertex.push_back(MomentVertex[iMoment]);
            ShortMemory[iShort].time = tmptime;
            if (ShortMemory[iShort].memory_score != -1) ShortMemory[iShort].memory_score += MEMORYSPEED;
            GenerateLongMemory(myGPT, LongMemory, ShortMemory, iShort);
        }
    }
}


void ForgetProcess(vector<LandmarkShort> &ShortMemory){
    for (int i = 0; i < ShortMemory.size(); i++){
        if (ShortMemory[i].memory_score < 0) continue;//has became long memory, never forget
        if (ShortMemory[i].memory_score < FORGETSPEED){
            ShortMemory[i].memory_score = 0;//totally forget
            continue;
        }
        ShortMemory[i].memory_score -= FORGETSPEED;//gradually forget
    }
}
void GenerateLongMemory(GPT_API &myGPT, vector<LandmarkLong> &LongMemory, vector<LandmarkShort> &ShortMemory, int iShort){
    bool abiggername = false;//when become longmemory, there should be a name that occurs more than five times.
    for (int i = 0; i < ShortMemory[iShort].num.size(); i++){
        if (ShortMemory[iShort].num[i] >= 5) {
            abiggername = true;
            break;
        }
    }
    abiggername = true;
    if ((ShortMemory[iShort].memory_score > MEMORYTHRESHOLD) && abiggername){//this short memory just become long memory
        ShortMemory[iShort].memory_score = -1;//turn into long memory

        LandmarkLong templandmark;
        for (int i = 0; i < ShortMemory[iShort].num.size(); i++){
            //if (ShortMemory[iShort].num[i] >= 5){
            templandmark.name.push_back(ShortMemory[iShort].name[i]);
            templandmark.num.push_back(ShortMemory[iShort].num[i]);
            //}
        }       
        templandmark.shortvec = iShort;
        templandmark.all_pos.assign(ShortMemory[iShort].all_pos.begin(), ShortMemory[iShort].all_pos.end());
        templandmark.plane = plane_calculate(ShortMemory[iShort]);
        vertex_average(ShortMemory[iShort], templandmark);
        templandmark.time = ShortMemory[iShort].time;
        mtxGPT.lock();
        MemoryForGPT.push_back(templandmark);
        mtxGPT.unlock();
        LongMemory.push_back(templandmark);/*
        LandmarkMap tempmap;
        std::string resonable_one = myGPT.is_reasonable(LongMemory[LongMemory.size() - 1].name);
        if (resonable_one == "e") resonable_one = LongMemory[LongMemory.size() - 1].name[0];
        tempmap.name = resonable_one;   
        tempmap.pangolin_name = ((int)resonable_one[0] < 0) ? "******************":resonable_one;
        tempmap.all_pos.assign(ShortMemory[iShort].all_pos.begin(), ShortMemory[iShort].all_pos.end());
        tempmap.is_shop = myGPT.is_shop(tempmap.name);
        tempmap.ave_pos = fitting_points(tempmap.all_pos);
        mtxmap.lock();
        TextMap.push_back(tempmap);
        mtxmap.unlock();*/
        
    }/*
    else if (ShortMemory[iShort].memory_score == -1)//have been remembered
        for (int i = 0; i < LongMemory.size(); i++)
            if (LongMemory[i].name[0] == ShortMemory[iShort].name[0] ){
                LongMemory[i].all_pos.assign(ShortMemory[iShort].all_pos.begin(), ShortMemory[iShort].all_pos.end()); 
                LongMemory[i].name.assign(ShortMemory[iShort].name.begin(), ShortMemory[iShort].name.end());
                mtxmap.lock();
                TextMap[i].all_pos.assign(LongMemory[i].all_pos.begin(), LongMemory[i].all_pos.end());  
                mtxmap.unlock();  
            
                
                break;
            }*/
    
}

bool IsLandmarkNew(vector<LandmarkShort> &ShortMemory, vector<string> &MomentName, vector<Vector3d> &MomentPos, int iShort, int iMoment){
    Vector3d average_axis = fitting_points(ShortMemory[iShort].all_pos);
    ShortMemory[iShort].ave_pos = average_axis;
    float dis = (average_axis - MomentPos[iMoment]).norm();
    
        float word_dis = 0;
    for (int i=0; i < ShortMemory[iShort].name.size(); i++){
        word_dis += string_compare(ShortMemory[iShort].name[i], MomentName[iMoment]);

    }
    word_dis /= ShortMemory[iShort].name.size();
        if (word_dis < 0.5) return false;
    //if (word_dis < 0.3) cout << "*************" << endl << dis << endl;
    //if (dis < 1.5 && word_dis < 0.4) return false;
    //dis means two landmark are close to eachother
    //word_dis means two landmarks have similar name
    return true;

}



Vector3d fitting_points(vector<Vector3d> &all_pos){
    int decrease_time = 5;// every time we remove 1/5=20% of the points
    vector<Vector3d> temp_pos(all_pos);
    int erase_num = all_pos.size() / decrease_time;//every time the number of pos we erase
    for (int ii = 0; ii < decrease_time - 1; ii++){
        Vector3d average_axis  (0.0, 0.0, 0.0);
        for (int i = 0;i < temp_pos.size(); i++)
            average_axis += temp_pos[i];
        average_axis /= temp_pos.size();
        int max_vec = 0;
        int i;
        for (int j = 0; j < erase_num; j++){
            for ( i = 1; i < temp_pos.size(); i++)
                if ((average_axis - temp_pos[i]).norm() > (average_axis - temp_pos[max_vec]).norm())    max_vec = i;
            temp_pos.erase(temp_pos.begin() + max_vec);   
        }
    }
    Vector3d average_axis  (0.0, 0.0, 0.0);
    for (int i = 0;i < temp_pos.size(); i++)
        average_axis += temp_pos[i];
    average_axis /= temp_pos.size();
    return average_axis;
    
}
Vector3d plane_calculate(LandmarkShort &ShortMoment){
    MatrixXd A(ShortMoment.vertex.size() * 4, 3);
    VectorXd b(ShortMoment.vertex.size() * 4);
    for (int i = 0; i < ShortMoment.vertex.size(); i++){
        A(i*4, 0) = ShortMoment.vertex[i][0](0);
        A(i*4, 1) = ShortMoment.vertex[i][0](1);
        A(i*4, 2) = ShortMoment.vertex[i][0](2);
        A(i*4 + 1, 0) = ShortMoment.vertex[i][1](0);
        A(i*4 + 1, 1) = ShortMoment.vertex[i][1](1);
        A(i*4 + 1, 2) = ShortMoment.vertex[i][1](2);
        A(i*4 + 2, 0) = ShortMoment.vertex[i][2](0);
        A(i*4 + 2, 1) = ShortMoment.vertex[i][2](1);
        A(i*4 + 2, 2) = ShortMoment.vertex[i][2](2);
        A(i*4 + 3, 0) = ShortMoment.vertex[i][3](0);
        A(i*4 + 3, 1) = ShortMoment.vertex[i][3](1);
        A(i*4 + 3, 2) = ShortMoment.vertex[i][3](2);
        b(i*4) = -1;
        b(i*4 + 1) = -1;
        b(i*4 + 2) = -1;
        b(i*4 + 3) = -1;
    }
    return (A.transpose() * A).inverse() * A.transpose() * b;
};
void vertex_average(LandmarkShort &ShortMemory, LandmarkLong &LongMemory){
    Vector3d p0, p1, p2, p3;
    for (int i = 0; i < ShortMemory.vertex.size(); i++){
        p0 += ShortMemory.vertex[i][0];
        p1 += ShortMemory.vertex[i][1];
        p2 += ShortMemory.vertex[i][2];
        p3 += ShortMemory.vertex[i][3];
    }
    p0 /= ShortMemory.vertex.size();
    p1 /= ShortMemory.vertex.size();
    p2 /= ShortMemory.vertex.size();
    p3 /= ShortMemory.vertex.size();
    LongMemory.vertex.push_back(p0);
    LongMemory.vertex.push_back(p1);
    LongMemory.vertex.push_back(p2);
    LongMemory.vertex.push_back(p3);
}



