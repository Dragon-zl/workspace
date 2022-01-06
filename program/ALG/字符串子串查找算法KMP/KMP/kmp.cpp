#include <iostream>
using namespace std;
class  My_KMP{
    public:
    My_KMP(){
    }
    void Get_next( string str , int * next){
        next[0] = -1;
        int i = 0 , j = -1;
        while( i < str.length()){
            if(j == -1 || str[i] == str[j]){
                i++;
                j++;
                next[i] = j;
            }
            else{
                j = next[j];
            }
        }
    }
    int search(string str , string pattern , int * next){
        int i = 0 , j = 0 , str_len = str.length() , patt_len = pattern.length();
        while( i < str_len && j < patt_len ){
            if( j == -1 || str[i] == pattern[j]){
                i++;
                j++;
            }
            else{
                j = next[j];
            }
        }
        if(j == pattern.length()){
            return i - j;
        }
        else{
            return -1;
        }
    }
};
int main(){
    My_KMP my_Kmp;
    string  pattern , str;
    cin >> str;
    cin >> pattern;
    int next[pattern.length()] = {0};
    my_Kmp.Get_next( pattern , next);
    for(int i = 0 ; i < pattern.length() ; i++){
        cout << next[i] << " ";
    }
    cout << "\n";
    cout << my_Kmp.search( str , pattern , next) << endl;
    return 0;
}