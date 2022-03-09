#include<iostream>
#include<vector>
#include<assert.h>
using namespace std;

void  siftup(vector<int>&heap, int i){
    assert(i >= 0 && i<heap.size());
    size_t j = (i - 1)/2;
    while(j >= 0){
        if(heap[j] < heap[i]){
            break;
        }
        swap(heap[i], heap[j]);
        i = j;
        j = (i-1)/2;
    }
}

int main(){

    vector<int> nums;
    int i = 0;
    while(1){
        int tmp;
        cin >> tmp ;
        if(tmp == 0){
            break;
        }
        nums.push_back(tmp);
        siftup(nums, i);
        ++i;
    }
    for(int i = 0; i<nums.size(); ++i){
        cout << nums[i] << " ";
    }
    cout << endl;
    return 0;
}