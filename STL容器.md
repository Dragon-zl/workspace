# STL 

## 有序容器

### SET

| 方法                                                         | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| bool  count(const Key& key)    (C++20)                       | 检查容器中是否存在有 key 键等价的元素                        |
| insert(const value_type& value)                              | 插入元素                                                     |
| clear()                                                      | 擦除容器中的所有元素                                         |
| erase( iterator pos ); \|\| erase( const Key& key ); \|\| iterator erase( const_iterator first, const_iterator last ); | 擦除指定键或者指定迭代器的元素，也可以擦除某个迭代器范围的多个元素 |

