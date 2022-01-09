#include <iostream>
#include <math.h>
using namespace std;
#define Min(a , b)  (a < b ? a : b)

class Edit_distance{
    private:
    int Min_edit_distance = INFINITY;   //初始化为无穷大
    public:
    int Get_Min(int a ,int b ,int c){
        return Min( Min(a , b) , c);
    }
    int Get_Min_edit_distance(string src , string dst)
    {
        int src_len = src.length() , dst_len = dst.length();
        int dp[src_len + 1][dst_len + 1];
        for(int i = 0 ; i < src_len + 1; i++){
            dp[i][0] = i;
        }
        for(int j = 0 ; j < dst_len + 1; j++){
            dp[0][j] = j;
        }
        for( int i = 1 ; i < src_len + 1 ; i++){
            for( int j = 1 ; j < dst_len + 1; j++){
                int flag = ( src[i - 1] == dst[j -1] ) ?  0 : 1;
                dp[i][j] = Get_Min( dp[i - 1][j]    + 1,        //删除
                                    dp[i][j-1]      + 1,        //增加  
                                    dp[i - 1][j -1] + flag);    //替换
            }
        }
        return dp[src_len][dst_len];
    }

};

int main(){
    string src , dst;
    cin >> src >> dst;
    Edit_distance  ED;
    cout << ED.Get_Min_edit_distance(src , dst) << endl;
    return 0;
}