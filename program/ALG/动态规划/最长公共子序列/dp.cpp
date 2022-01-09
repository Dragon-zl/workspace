#include <iostream>

using namespace std;

#define max(a , b)  (a > b ? a : b)
class Longest_com_sub
{
private:
    int max_Longest_com_sub = 0;
public:
    int Get_Longest_com_sub(string str1 , string str2){
        int len1 = str1.length() , len2 = str2.length();
        int dp[len1 + 1][len2 + 1] = {0};
        int i , j;
        for( i = 1 ; i < len1 + 1 ; i++){
            for( j = 1 ; j < len2 + 1 ; j++){
                if( str1[i -1 ] == str2[ j - 1] ){
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                }
                else{
                    dp[i][j] = max(dp[i - 1][j] , dp[i][j - 1]);
                }
            }
        }
        return dp[i - 1][j - 1];
    }
};
int main(){
    string str1 , str2;
    cin >> str1;
    cin >> str2;
    Longest_com_sub  L_C_B;
    cout << L_C_B.Get_Longest_com_sub(str1 , str2) << endl;
    return 0;
}