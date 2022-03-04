#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
using namespace std;
int main()
{
	vector<string>sql_vec;
    char s[] = "Golden Global   View,disk * desk";
    const char *d = ",";
    char *p;
    p = strtok(s,d);
    while(p)
    {
		string str = p;
		sql_vec.push_back(str);
        p=strtok(NULL,d);
    }
	for(int i = 0; i < sql_vec.size(); ++i){
		cout << sql_vec[i] <<endl;
	}
	sql_vec.clear();
    return 0;
}
