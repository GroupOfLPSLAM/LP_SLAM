
#define MEMORYSPEED 10//in void generateshortmemory, describe how fast we memory
#define FORGETSPEED 3//in void forgetprocess, describe how fast we forget
#define MEMORYTHRESHOLD 70//in void generatelongmemory, describe the THRESHOLD of memory

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

#ifndef _MEMORY_H_
#define _MEMORY_H_
#include <chat.h>

#endif

using namespace std;
using namespace Eigen;
#ifndef _LANDMARK_
#define _LANDMARK_
struct LandmarkShort{
    double time;
    vector<std::string> name;
    vector<Eigen::Vector3d> all_pos;
    vector<vector<Eigen::Vector3d>> vertex;// four vertexes of the square.0..3 stands for leftup,rightup,leftdown,rightdown
    Eigen::Vector3d ave_pos;
    vector<int> num;
    int memory_score;//increase when data in a group of short memory appears, and decrease every round. when score reach memorythreshold, short memory become long memory.
};//there are several group in short memory, each group represent similar datas.
struct LandmarkLong{
    int shortvec;
    double time;
    vector<std::string> name;
    vector<int> num;
    vector<Eigen::Vector3d> all_pos;
    Eigen::Vector3d plane;//(a,b,c)means ax+by+cd+1=0
    vector<Eigen::Vector3d> vertex;// four vertexes of the square.0..3 stands for leftup,rightup,leftdown,rightdown
    Eigen::Vector3d ave_pos;
};//frequent groups in short memory will be sent into long memory.
struct LandmarkMap{
    double time;
    std::string name;
    vector<Eigen::Vector3d> all_pos;
    Eigen::Vector3d plane;//(a,b,c)means ax+by+cd+1=0
    vector<Eigen::Vector3d> vertex;// four vertexes of the square.0..3 stands for leftup,rightup,leftdown,rightdown
    Eigen::Vector3d ave_pos;
    bool is_shop;
    std::string pangolin_name;//transform chinese character into ******
    int comb_id;//which combination this landmark is in
};
struct LandmarkComb{
    Eigen::Vector3d plane;//(a,b,c)means ax+by+cd+1=0
    vector<Eigen::Vector3d> vertex;
    vector<LandmarkMap> landmark;
    int is_shop;
};//combination of several LandmarkLonng.stand for  situations that there are several lines of woords in a landmark


#endif
class GPT_API;
//vector<Landmark> LongMemory;
void GenerateShortMemory(GPT_API &myGPT, vector<LandmarkShort> &ShortMemory, vector<string> &MomentName, vector<Vector3d> &MomentPos, vector<vector<Vector3d>> &MomentVertex, double tmptime);//each frame of ocr result will be sent to this function and then form short memory.
void ForgetProcess(vector<LandmarkShort> &ShortMemory);//porcess memory_core in short memory. memory_socre of frequent group is high.
void GenerateLongMemory(GPT_API &myGPT, vector<LandmarkLong> &LongMemory, vector<LandmarkShort> &ShortMemory, int iShort);//frequent groups in short memory will be sent into long memory.
bool IsLandmarkNew(vector<LandmarkShort> &ShortMemory, vector<string> &MomentName, vector<Vector3d> &MomentPos,int iShort, int iMoment);//if a landmark is similar with any group in short memory, return 0. else 1
float string_compare(std::string str1, std::string str2);//0 means two strings are same with each other. the bigger output is , more different two strings are.
Vector3d fitting_points(vector<Vector3d> &all_pos);//given a group of similar point, this funtion return their average position
Vector3d plane_calculate(LandmarkShort &ShortMoment);
void vertex_average(LandmarkShort &ShortMemory, LandmarkLong &LongMemory);