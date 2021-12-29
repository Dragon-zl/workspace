#include <iostream>
#include <set>
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
}

int lengthOfLongestSubstring :: My_lengthOfLongestSubstring(string str){
    set<char> My_set;   //创建集合
    int index = 0 , len = str.length();
    string max_str = "";
    int maxsize = 0;
    while(index < len){
        while(true){
            char now_char = str[index];
            if( My_set.count(now_char) ) //判断当前索引的字符，是否存在set容器中 ， 存在返回 true
            {
                index = index - My_set.size() + 1;      //index 返回开始的位置+1 的索引
                break;
            }
            else{
                My_set.insert(str[index++]);        //不存在，则插入set容器
            }
            if(index == len){               //判断索引是否溢出
                break;
            }
        }
        if(My_set.size() > maxsize){        //判断此时的容器的长度是否为最大值
            max_str.clear();
            maxsize = My_set.size();        //保存 子字符串
            int j = 0;
            for(set<char>::iterator i = My_set.begin() ; i != My_set.end() ; i++){
                max_str += *i;
            }
        }
        My_set.clear();     //每次有重复元素，或者索引到最后了，清空容器
    }
    cout << max_str.c_str() << endl;
    return maxsize;
}