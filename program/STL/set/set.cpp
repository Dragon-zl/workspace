#include <iostream>
#include <set>
using namespace std;

int main(){

    set<int> st;
    st.lower_bound(1);
    st.upper_bound(1);
    st.key_comp();

    return 0;
}