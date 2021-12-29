#include <iostream>
#include <unordered_set>
using namespace std;
class  lengthOfLongestSubstring{
    public:
    int My_lengthOfLongestSubstring(string str);
};
int main(){
    lengthOfLongestSubstring fun ;
    string str;
    getline(cin , str);//获取终端用户输入的字符串，不会过滤空格和tab
    int len = fun.My_lengthOfLongestSubstring(str);
    cout << "Maxsize :" <<  len << endl;
    return 0;
}
int lengthOfLongestSubstring :: My_lengthOfLongestSubstring(string str){
    int len = str.length();
    unordered_set<char>  My_set;      //定义窗口
    int max_size = 0;
    int i = 0 , j = 0;
    while(i < len && j < len){
        char temp = str[j];
        /*
        unordered_set<char> :: iterator  it  = My_set.find(temp);
        unordered_set<char> :: iterator  it_begin;*/
        if(  it  == My_set.end() ){      //如果不存在，就插入 容器
            My_set.insert(str[j++]);
        }
        else{                           //如果容器中存在该元素，依次不断删除插入的元素，直到能够插入
            /*
            do{
                it_begin = My_set.begin();
                it = My_set.find(temp);
                My_set.erase(it_begin);
            }while(it_begin != it);*/
            My_set.erase(str[i++]);
        }
        if( My_set.size() > max_size){
            max_size = My_set.size();
        }
    }
    return max_size;
}