#include <iostream>
#include <string>
using namespace std;
int KMP_Src(string  S , string P , int S_index , int P_index){
    while(P_index > 0 && S[S_index] == P[--P_index]){
    }
    while(P_index > 0){
        
    }
}
int KMP(string  S , string P){
    int S_len = S.length() , P_len = P.length();
    int S_src = 0 , P_left = 0;
    while(S_src < S_len){
        if(S[S_src] == P[P_left]){   //匹配到模式串的第一个字符
            int S_index = S_src + 1;
            int P_index = 1;
            while(P_index < P_len && S[S_index++] == P[P_index++]){
            }
            if(P_index == P_len){
                return S_src;
            }
            else{
                KMP_Src(S , P , --S_index , --P_index)
            }
        }
        S_src++;
    }
    return -1;
}
int main(){
    int index;
    string S , P;
    cin >> S;
    cin >> P;
    if((index = KMP(S , P)) < 0 ){
        cout << "不存在\n";
    }
    else{
        cout << "位置：" << index << endl;
    }
    return 0;
}