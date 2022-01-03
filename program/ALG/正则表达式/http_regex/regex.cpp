#include <iostream>
#include <regex>

using namespace std;

int main(){

    string http_head = "GET /home.html HTTP/1.1";

    regex  http_reg("GET.+HTTP/1.1");

    smatch  result;
    if( regex_match( http_head , result , http_reg ) ){
        cout << "成功\n"; 
    }else{
        cout << "失败\n";
    }
    string :: const_iterator iter_begin = http_head.cbegin();
    string :: const_iterator iter_end   = http_head.cend();

    if( regex_search(iter_begin , iter_end , result  , http_reg) ){
        cout << result[0] << endl;
    }

    return 0;
}