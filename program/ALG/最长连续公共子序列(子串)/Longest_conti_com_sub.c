#include <iostream>
using namespace std;
#define max(a,b) (a>b? a:b)
class Longest_conti_com_sub{
    private:
        int max_sub = 0;
    public:
        int Get_Longest_conti_com_sub( string str1 , string str2){
            int dp[str1.length() + 1][str2.length() + 1] = {0};
            for( int i = 1 ; i < str1.length() + 1 ; i++){
                for(int j = 1 ; j < str2.length() + 1 ; j++){
                    if(str1[i - 1] == str2[j - 1]){
                        dp[i][j] = dp[i-1][j-1] + 1;
                        max_sub  = max(max_sub , dp[i][j]); 
                    }
                    else{
                        dp[i][j] = 0;
                    }
                }
            }
            return max_sub;
        }
};
int main(){
    Longest_conti_com_sub  lccs;
    string str1 , str2;
    cin >> str1;
    cin >> str2;
    cout << lccs.Get_Longest_conti_com_sub(str1 , str2) << endl;
    return 0;
}