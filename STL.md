# STL 

## 有序容器

### SET

| 方法                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| bool  count(const Key& key)    (C++20)                       | 检查容器中是否存在有 key 键等价的元素                        |
| insert(const value_type& value)                              | 插入元素                                                     |
| clear()                                                      | 擦除容器中的所有元素                                         |
| erase( iterator pos ); \|\| erase( const Key& key ); \|\| iterator erase( const_iterator first, const_iterator last ); | 擦除指定键或者指定迭代器的元素，也可以擦除某个迭代器范围的多个元素 |

### unordered_map

| 方法        | 说明                               | 示例                                                         |
| ----------- | ---------------------------------- | ------------------------------------------------------------ |
| insert      | 插入元素                           | My_mp.insert({ 3 , "Three"})                                 |
| clear       | 擦除容器中的所有元素               | My_mp.clear()                                                |
| equal_range | 返回查找的相同键的范围(一对迭代器) | auto range = My_mp.equal_range(3);  for(auto it = range.first ; it != range.second ; it++){}  示例网址：[C++ unordered_multimap equal_range()用法及代码示例 - 纯净天空 (vimsky.com)](https://vimsky.com/examples/usage/unordered_multimap-equal_range-function-in-c-stl.html) |
|             |                                    |                                                              |
|             |                                    |                                                              |
|             |                                    |                                                              |

