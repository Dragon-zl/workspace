#include <iostream>
#include <vector>
#include <iterator>
using namespace std;

int main(){
    vector<int> vt1(3 , 11);
    vector<int> vt2(4 , 10);

    vector<int> :: iterator it;
    vt1.assign( 3 , 9);

    vt2.assign(vt1.begin() , vt1.end());

    /* it = vt2.insert( vt2.end(), 3 , 10); */
    /* vt2.front(); */
    /* cout << vt2.at(2) << endl; */
    /* vt2.erase(vt2.end() - 1);//删除最后一个元素 */

    cout << "vt2" << endl;
    vt2.resize(5 , 8);
    vt2.swap(vt1);
    /* cout << *(--it) << endl; */

    for(it = vt2.begin(); it != vt2.end(); ++it){
        cout << *it << endl;
    }
    return 0;
}