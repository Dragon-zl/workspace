#include <iostream>
#include <string>
#include <iterator>
#include <regex>
using namespace std;
int main(){
    //定义字符串，C++中，字符 " 需要转义 即：\"
    string str =  "Some people, when confronted with a problem, think "
        "\"I know, I'll use regular expressions.\" "
        "Now they have two problems.";
    
    //忽略大小写
    regex  self_regex("REGULAR EXPRESSIONS" , std::regex_constants::ECMAScript | std::regex_constants::icase);
    if ( regex_search(str, self_regex)) {
        cout << "Text contains the phrase 'regular expressions'\n";
    }
    else{
        cout << "Text contains non-existent the phrase 'regular expressions'\n";
    }
    //------------------------------------------------------
    regex  word_regex("(\\w+)");
    auto  words_begin = 
        std::sregex_iterator( str.begin() , str.end() , word_regex);
    
    

    return 0;
}