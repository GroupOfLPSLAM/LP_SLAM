#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sstream>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <chat.h>
using namespace std;
using namespace Eigen;

extern vector<LandmarkMap> TextMap;

extern vector<LandmarkLong> LongMemory;


extern vector<LandmarkShort> ShortMemory;
extern std::mutex mtxmap;
extern vector<LandmarkLong> MemoryForGPT;
extern vector<LandmarkComb> CombMap;
extern bool lpshutdown;
extern std::mutex mtxshutdown;
extern std::mutex mtxGPT;
GPT_API::GPT_API(){
    FILE *fp = fopen(is_shop_path.c_str(), "r");
    if (NULL == fp){
      printf("fail open chat1.txt");
    }
    char str[600];
    while (fgets(str, 600, fp)!= NULL){
        int text_start,text_end;
        char *find = strchr(str, '\n');
        if (find) *find = '\0';
        my_message_is_shop.append(",{\"role\":\"user\",\"content\":\"");
        my_message_is_shop.append(str);
        my_message_is_shop.append("\"}");

    }
    //pre-train
    fclose(fp);
    my_message_is_shop.append(",{\"role\":\"user\",\"content\":\"let's try another,[[");
    
    fp = fopen(is_reasonable_path.c_str(), "r");
    if (NULL == fp){
      printf("fail open is_reasonable.txt");
    }
    while (fgets(str, 600, fp)!= NULL){
        int text_start,text_end;
        char *find = strchr(str, '\n');
        if (find) *find = '\0';
        my_message_is_resonable.append(",{\"role\":\"user\",\"content\":\"");
        my_message_is_resonable.append(str);
        my_message_is_resonable.append("\"}");

    }
    //pre-train
    fclose(fp);
    my_message_is_resonable.append(",{\"role\":\"user\",\"content\":\"the answer should be [[{{TsinghuaUniversity}}]],lets try another: ");
    
    fp = fopen(navigation_path.c_str(), "r");
    if (NULL == fp){
      printf("fail open is_reasonable.txt");
      return ;
    }
    while (fgets(str, 600, fp)!= NULL){
        int text_start,text_end;
        char *find = strchr(str, '\n');
        if (find) *find = '\0';
        my_message_where_to_go.append(",{\"role\":\"user\",\"content\":\"");
        my_message_where_to_go.append(str);
        my_message_where_to_go.append("\"}");

    }
    //pre-train
    fclose(fp);
    
    my_message_where_to_go.append(",{\"role\":\"user\",\"content\":\"the answer should be [[{{一点点奶茶}}]],lets try another,the list is [[");
    
    
}
size_t WriteMemoryCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    size_t num_bytes = size * nmemb;
    std::string* response = static_cast<std::string*>(userdata);
    response->append(ptr, num_bytes);
    return num_bytes;
};
std::string GPT_API::get_chat(std::string my_ask){
  CURL *curl_handle;
  CURLcode res;
  curl_global_init(CURL_GLOBAL_ALL);

  std::string response;
  curl_handle = curl_easy_init();
  if (curl_handle) {
    curl_easy_setopt(curl_handle, CURLOPT_URL, API_URL);
    std::string my_text = std::string("{\"model\":\"gpt-3.5-turbo-0301\",\"messages\":[") + my_ask + std::string("], \"max_tokens\": 2000}");
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, my_text.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    struct curl_slist* headers = nullptr;
    std::string keystr = std::string("Authorization: Bearer ") + API_KEY;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, keystr.c_str() );
    
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10);
    res = curl_easy_perform(curl_handle);

    curl_slist_free_all(headers);
    if (res != CURLE_OK) {
      fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
      return get_chat(my_ask);
    } else {
      return response;
    }

  }
  
  curl_easy_cleanup(curl_handle);

  curl_global_cleanup();
}

int GPT_API::is_shop(std::string name){

    std::string my_message2 = my_message_is_shop;
    my_message2.append(name);
    my_message2.append("]]\"}");
    std::string response = get_chat(my_message2);
    int text_start = response.find(",\"content\":\"") + 12;
    int text_end = response.find("\"},\"finish_reason");
    std::string whole_response = response.substr(text_start, text_end - text_start);
    for (int i = 0; i < whole_response.length(); i++){
        if (whole_response[i] == '0' && whole_response[i-1] == '[') {
          return 0;
        }
        if (whole_response[i] == '1' && whole_response[i-1] == '[') {
          return 1;
        }
    }
    
    return -1;
  
  

}


std::string GPT_API::is_reasonable(vector<std::string> &list){
    std::string my_message2 = my_message_is_resonable;
    my_message2.append(list[0]);
    for (int i=1; i < list.size(); i++){
        my_message2.append(",");
        my_message2.append(list[i]);
    }
    my_message2.append("\"}");
    std::string response = get_chat(my_message2);
    //printf("%s\n",response.c_str());
    
    int text_start = response.find("[[{{") + 4;
    if (text_start == string::npos) return "e";
    int text_end = response.find("}}]]");
    if (text_end == string::npos) return "e";
    if (text_end - text_start > 80) return "e";
    std::string resonable_one = response.substr(text_start, text_end - text_start);
    
    return resonable_one;
  
  

}

void GPT_API::where_to_go(){
    while(1){
    
    std::string my_question;
    cout << "input your instruction here. -help for help:";
    std::getline(std::cin, my_question, '\n');
    if (my_question[0] == '-'){
        if (my_question == "-list")
            {   

/*
                mtxmap.lock();
                if (TextMap.size() == 0){
                    mtxmap.unlock();
                    cout << "sorry, nothing in list" << endl;
                    continue;
                }
                for (int i=0; i < TextMap.size(); i++){
                    cout << i << ':';
                    std::stringstream ss;
                    ss << std::fixed << TextMap[i].time;
                    cout << "time:" << ss.str()<< endl;
                    cout << TextMap[i].name.c_str() << endl;
                    std::string myout = (TextMap[i].is_shop == 1)?"is shop":"isn't shop";
                    cout << myout << endl;
                    cout << TextMap[i].ave_pos << endl;
                    cout << "*******************" << endl;
                }
                mtxmap.unlock();*/


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
                continue;
            }
        else 
        if (my_question == "-help"){
            cout << "-help for help" << endl;
            cout << "-list for list in text map" << endl;
            cout << "-exit for terminate the navigation thread" << endl;
            cout << "if you want to ask me where to go, input your question directly" << endl;
            continue;
        }
        else
        if (my_question == "-exit"){
            cout << "Navigation thread terminated!" << endl;
            break;
        }
        else{
            cout << "no such instructions" << endl;
            continue;
        }
    }
    mtxmap.lock();
    if (TextMap.size() == 0){
        cout << "sorry, nothing in list" << endl;
        mtxmap.unlock();
        continue;
    }
    
    std::string my_message2 = my_message_where_to_go;
    my_message2.append(TextMap[0].name);
    for (int i=1; i < TextMap.size(); i++){
        my_message2.append(",");
        my_message2.append(TextMap[i].name);
    }
    my_message2.append("]],my question is ");
    my_message2.append(my_question);
    my_message2.append("\"}");
    mtxmap.unlock();
    std::string response = get_chat(my_message2);
    //printf("%s\n",response.c_str());
    
    int text_start = response.find("[[{{") + 4;
    int text_end = response.find("}}]]");
    if (text_end == string::npos){
        cout<<"sorry, i dont understand"<<endl;
        continue;
    }
    int i = 0;
    std::string destination = response.substr(text_start, text_end - text_start);
    for (i=0; i < TextMap.size(); i++){
        if (string_compare(TextMap[i].name , destination) < 0.2) break;
    }
    cout<<TextMap[i].name<<endl;
    cout<<TextMap[i].ave_pos<<endl;
    
    
    }
    
  
  

}



void GPT_API::GPT_work(){
    while(1){
        mtxshutdown.lock();
        if (lpshutdown) {
        
            mtxshutdown.unlock();
            mtxmap.lock();
                if (TextMap.size() == 0){
                    mtxmap.unlock();
                    cout << "sorry, nothing in list" << endl;
                    continue;
                }
                for (int i=0; i < TextMap.size(); i++){
                    cout << i << ':';
                    std::stringstream ss;
                    ss << std::fixed << TextMap[i].time;
                    cout << "time:" << ss.str()<< endl;
                    cout << TextMap[i].name.c_str() << endl;
                    std::string myout = (TextMap[i].is_shop == 1)?"is shop":"isn't shop";
                    cout << myout << endl;
                    cout << TextMap[i].ave_pos << endl;
                    cout << "*******************" << endl;
                }
                mtxmap.unlock();
                
            cout << "GPT thread is terminated!!" << endl;
            break;
        }
        mtxshutdown.unlock();
        mtxGPT.lock();
        if (MemoryForGPT.empty()){
            mtxGPT.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;         
        }
        LandmarkLong tmpmem;
        tmpmem.vertex = MemoryForGPT[0].vertex; 
        tmpmem.time = MemoryForGPT[0].time;
        tmpmem.plane = MemoryForGPT[0].plane;
        tmpmem.name.assign(MemoryForGPT[0].name.begin(), MemoryForGPT[0].name.end());
        tmpmem.all_pos.assign(MemoryForGPT[0].all_pos.begin(), MemoryForGPT[0].all_pos.end());
        MemoryForGPT.erase(MemoryForGPT.begin());
        mtxGPT.unlock();
        LandmarkMap tempmap;
        std::string resonable_one = is_reasonable(tmpmem.name);
        if (resonable_one == "e") resonable_one = tmpmem.name[0];
        tempmap.name = resonable_one;  
        tempmap.time = tmpmem.time;
        tempmap.plane = tmpmem.plane;
        tempmap.vertex = tmpmem.vertex;
        tempmap.pangolin_name = ((int)resonable_one[0] < 0) ? "******************":resonable_one;
        tempmap.is_shop = is_shop(tempmap.name);
        tempmap.ave_pos = fitting_points(tmpmem.all_pos);
        tempmap.all_pos.assign(tmpmem.all_pos.begin(), tmpmem.all_pos.end());
        mtxmap.lock();
        TextMap.push_back(tempmap);
        bool new_word = true;
        if (CombMap.empty()){
            LandmarkComb tmp;
            tmp.vertex = tempmap.vertex;
            tmp.plane = tempmap.plane;
            //tmp.is_shop = tempmap.is_shop;
            tmp.landmark.push_back(tempmap);
            new_word = false;
            CombMap.push_back(tmp);
        }
        else
        for (int i=0; i < CombMap.size(); i++)
            if (is_in_comb(CombMap[i], tempmap)){
                cout << CombMap[i].landmark[0].name << " and " << tempmap.name << endl;
                new_word = false;
                LandmarkComb tmp;
                //tmp.landmark.push_back(tempmap);
                tmp.plane = new_plane(CombMap[i], tempmap);
                tmp.vertex = new_vertex(CombMap[i], tempmap, tmp.plane);
                CombMap[i].plane = tmp.plane;
                CombMap[i].vertex = tmp.vertex;
                CombMap[i].landmark.push_back(tempmap);
                //CombMap[i].is_shop = tempmap.is_shop;
                break;
                
            }
        if (new_word){
            LandmarkComb tmp;
            tmp.vertex = tempmap.vertex;
            tmp.plane = tempmap.plane;
            //tmp.is_shop = tempmap.is_shop;
            tmp.landmark.push_back(tempmap);
            CombMap.push_back(tmp);


        }


        mtxmap.unlock();
        
        
        
        
        
    }
    
}
vector<Eigen::Vector3d> GPT_API::new_vertex(LandmarkComb &MyComb, LandmarkMap &MyWord, Eigen::Vector3d &plane){
    Vector3d p0 = MyComb.vertex[0];
    Vector3d p1 = MyComb.vertex[1];
    Vector3d p2 = MyComb.vertex[2];
    Vector3d p3 = MyComb.vertex[3];
    if ((p1 - p0).normalized().dot((p2 - p0).normalized()) > 0.2){
        if ((p1 - p0).normalized().dot((MyComb.vertex[3] - p0).normalized()) < 0.2){
            p2 = MyComb.vertex[3];
            p3 = MyComb.vertex[2];
        } else if ((MyComb.vertex[3] - p0).normalized().dot((p2 - p0).normalized()) < 0.2){
            p1 = MyComb.vertex[3];
            p3 = MyComb.vertex[1];
        } else cout << "vertex of MyComb error !!!!!!!!!!1" << endl << "********************" << endl;
    }//set (p1 - p0) and (p2 - p0) as the axis of the comb plane
    p0 = point2plane(plane, p0);
    p1 = point2plane(plane, p1);
    p2 = point2plane(plane, p2);
    Vector3d axis_1 = p1 - p0;
    Vector3d axis_2 = p2 - p0;
    
    Vector2d comb_point_0(0,0);
    Vector2d comb_point_1(1,0);
    Vector2d comb_point_2(0,1);
    Vector2d comb_point_3(1,1);

    MatrixXd axis(3,2);
    axis(0,0) = axis_1(0);
    axis(1,0) = axis_1(1);
    axis(2,0) = axis_1(2);
    axis(0,1) = axis_2(0);
    axis(1,1) = axis_2(1);
    axis(2,1) = axis_2(2);
    Vector2d word_point_0 = (axis.transpose() * axis).inverse() * axis.transpose() * (MyWord.vertex[0] - p0);
    Vector2d word_point_1 = (axis.transpose() * axis).inverse() * axis.transpose() * (MyWord.vertex[1] - p0);
    Vector2d word_point_2 = (axis.transpose() * axis).inverse() * axis.transpose() * (MyWord.vertex[2] - p0);
    Vector2d word_point_3 = (axis.transpose() * axis).inverse() * axis.transpose() * (MyWord.vertex[3] - p0);
    double xmin, xmax, ymin, ymax;
    xmin = min(min(0.0, word_point_0(0)), min(min(word_point_1(0), word_point_2(0)), word_point_3(0)));
    xmax = max(max(1.0, word_point_0(0)), max(max(word_point_1(0), word_point_2(0)), word_point_3(0)));
    ymin = min(min(0.0, word_point_0(1)), min(min(word_point_1(1), word_point_2(1)), word_point_3(1)));
    ymax = max(max(1.0, word_point_0(1)), max(max(word_point_1(1), word_point_2(1)), word_point_3(1)));
    Vector2d point_leftup(xmin, ymax);
    Vector2d point_rightup(xmax, ymax);
    Vector2d point_leftdown(xmin, ymin);
    Vector2d point_rightdown(xmax, ymin);
    vector<Vector3d> outvertex;
    outvertex.push_back(axis * point_leftup + p0);
    outvertex.push_back(axis * point_rightup + p0);
    outvertex.push_back(axis * point_leftdown + p0);
    outvertex.push_back(axis * point_rightdown + p0);
    return outvertex;

}
Eigen::Vector3d GPT_API::point2plane(Eigen::Vector3d plane, Eigen::Vector3d point){
    double t = (plane.dot(point)+ 1)/(plane.dot(plane));
    Vector3d answer = point - t * plane;
    return answer;
}

bool GPT_API::is_in_comb(LandmarkComb &MyComb, LandmarkMap &MyWord){
    bool is_parallel, is_near_normal, is_near_tangent;
    double CosNormalVector = abs(MyComb.plane.normalized().dot(MyWord.plane.normalized()));
    is_parallel = (CosNormalVector > 0.8);

    Vector3d Average_plane = new_plane(MyComb, MyWord);
    Vector3d mid_comb;
    for (int i = 0; i < 4; i++) mid_comb += MyComb.vertex[i];
    mid_comb /= 4;
    double dis_normal = (MyWord.ave_pos - mid_comb).dot(Average_plane.normalized());//normal distance of comb and word
    is_near_normal = (dis_normal < 0.5);

    double dis_tangent = sqrt((MyWord.ave_pos - mid_comb).norm() * (MyWord.ave_pos - mid_comb).norm() - dis_normal * dis_normal);
    is_near_tangent = (dis_tangent < 2);

    return (is_parallel && is_near_normal && is_near_tangent);


}
Eigen::Vector3d GPT_API::new_plane(LandmarkComb &MyComb, LandmarkMap &MyWord){
    Vector3d Average_plane, mid_pos;//mid_pos means ave_pos of all landmark in landmarkcomb
    for (int i = 0; i < MyComb.landmark.size(); i++){
        Average_plane += MyComb.landmark[i].plane.normalized();
        mid_pos += MyComb.landmark[i].ave_pos;
    }    
    mid_pos += MyWord.ave_pos;
    mid_pos /= (1 + MyComb.landmark.size());
    Average_plane = (Average_plane + MyWord.plane.normalized());
    Average_plane = (-1)/(Average_plane.dot(mid_pos))*Average_plane;
    return Average_plane;
}
Vector3d GPT_API::fitting_points(vector<Vector3d> &all_pos){
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
