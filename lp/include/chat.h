
#ifndef _CHAT_H
#define _CHAT_H_
#include <memory.h>
#endif
#include <math.h>
#include <Eigen/Core>
#include <mutex>
#include <Eigen/Geometry>
#include <iostream>
#include <cstring>
#include <thread>
#include <iomanip>
#define API_KEY "*****REPLACE***YOUR***OPENAI****APIKEY***HERE"
#define API_URL "https://api.openai.com/v1/chat/completions"

using namespace std;
using namespace Eigen;

size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
#ifndef _GPT_API_CLASS_
#define _GPT_API_CLASS_
class GPT_API{
private:
    std::string is_shop_path = "./lp/pretrain/is_shop.txt";
    std::string is_reasonable_path = "./lp/pretrain/is_reasonable.txt";
    std::string navigation_path = "./lp/pretrain/where_to_go.txt";
    std::string my_message_is_shop = "{\"role\":\"assistant\",\"content\":\"i will give you a word in shape of [[word1]],and you should return a [[0]] and [[1]].if word1 is probably the name of a shop, such as HUAWEI, you should return [[1]]. if my input is[[TOILET]], you should return [[0]],meanwhile,there could also be chinese input like [[请切拍打 ]],you should understand its chinese meaning and judge if it is a name of a shop.understand?\"}";
    std::string my_message_is_resonable = "{\"role\":\"assistant\",\"content\":\"i will give you a list of word in shape of word1,word2,word3.....,most of them have spelling problem, and only one of them is reasonable.you should show me the most reasonable one in shape of [[{{word}}]].for example, if my input is happy, happ, apphy, bapph, your answer should be [[{{happyd}}]],understand?\"}"; 
    std::string my_message_where_to_go = "{\"role\":\"assistant\",\"content\":\"i will give you a list of plase in shape of [[place1, place2, place3,...]], and i will ask you a question related with these place.you should return a place or a list of place and their x-y-z axis, in shape of [[{{place1}},{{x1,y1,z1}},{{place2}},{{x2,y2,z2}},{{place3}},{{x3,y3,z3}}]]from the list i give you,which means where i should go.for example, the list is [[stop, supermarket, toilet]],my question is where can i buy something to eat,you should return [[{{supermarket}}]],understand?\"}";  
public:
    GPT_API();
    std::string get_chat(std::string my_ask);//input a question and return answers of chatgpt
    int is_shop(std::string name);//if name is a shop return 1
    std::string is_reasonable(vector<std::string> &list);//input a list of word and some or all of them have spelling problem. return the correct word
    void where_to_go();//input a question, and chatgpt search the list and find the most resonable place.


    void GPT_work();//GPT thread, make curl perform separate with lp thread.

    Vector3d fitting_points(vector<Vector3d> &all_pos);//given a group of similar point, this funtion return their average position
    
    bool is_in_comb(LandmarkComb &MyComb, LandmarkMap &MyWord);//return 1 if word is in landmark combnition
    Eigen::Vector3d new_plane(LandmarkComb &MyComb, LandmarkMap &MyWord);//return the average plane of comb and new word
    Eigen::Vector3d point2plane(Eigen::Vector3d plane, Eigen::Vector3d point);//calculate the projection of the point to the plane
    vector<Eigen::Vector3d> new_vertex(LandmarkComb &MyComb, LandmarkMap &MyWord, Eigen::Vector3d &plane);//return the new vertex of the comb and the word

};
#endif

