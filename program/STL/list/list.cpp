#include <iostream>
#include <list>
#include <iterator>
using namespace std;
int main(){

    list<int> lt1(3 , 66);
    list<int> lt2(3 , 55);

    lt2.push_back(99);
    list<int> :: iterator it1;
    list<int> :: iterator it2;
    it1 = lt1.begin();
    it2 = lt2.end();
    //合并
    /* lt1.splice( ++it , lt2); */

    lt1.splice(++it1 , lt2 , lt2.begin() , it2);

    lt1.unique();
    /* lt1.push_back(88);
    lt1.remove(66); */

    
    cout << "it1" << endl;
    for(it1 = lt1.begin(); it1 != lt1.end(); ++it1){
        cout << *it1 << endl;
    }
    return 0;
}