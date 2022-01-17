/*
题目描述:
	给定一个整数数组 nums 和一个整数目标值 target，请你在该数组中找出 和为目标值 target  的那 两个 整数，并返回它们的数组下标。你可以假设每种输入只会对应一个答案。但是，数组中同一个元素在答案里不能重复出现。你可以按任意顺序返回答案。
	
示例1：
	输入：nums = [2,7,11,15], target = 9
    输出：[0,1]
    解释：因为 nums[0] + nums[1] == 9 ，返回 [0, 1] 。
示例2：
	输入：nums = [3,2,4], target = 6
	输出：[1,2]
示例 3：
	输入：nums = [3,3], target = 6
	输出：[0,1]
*/

/*
解题：
	使用允许插入相同键 的无需键值容器 unordered_map
代码：
*/
#include <unordered_map>
#include <vector>
#include <iostream>
using namespace std;
class Solution {
public:
    
    vector<int> twoSum(vector<int>& nums, int target) {
        vector<int> result;
        unordered_map<int , int > Map;
        int i;
	   //将vector 数组中的所有元素，插入unordered_map
        for( i = 0; i < nums.size() ; i++){
            Map.insert( {nums[i] , i} );
        }
        //使用unordered_map IO函数 equal_range ：返回一对迭代器，即可相同键的范围
        for( i = 0; i < nums.size() ; i++){
            auto range = Map.equal_range(target - nums[i]);
            //遍历返回的迭代器
            for(auto it = range.first ; it != range.second ; it++){
                if( it->second != i ){	//判断该迭代器所在的键 是否为当前键
                    result.push_back(i);
                    result.push_back(it->second);
                    return result;
                }
            }
        }
        return result;
    }
};
