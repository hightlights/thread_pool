#include "ThreadPool.h"

#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <functional>

int parallel_sum(const std::vector<int>& data, int start, int end)
{
  int sum = 0;
  for(int j = start; j < end; ++j){
    sum += data[j];
  }
  return sum;
}

void test()
{
  std::cout << "test for function" << std::endl;
}

int test_ret(int a, int b)
{
  std::cout << "test for ret and args" << std::endl;
  return 0;
}

void temp(){
  
}

int main()
{
  ThreadPool pool(std::string("test"));
  int thread_nums = 5; 
  pool.start(thread_nums);
  std::vector<int> vec(10000000, 1);
  int spec = vec.size() / thread_nums;
  std::vector<std::future<int>> res;
  //push to thread pool
  int i = 0;
  for(int j = 0; j < thread_nums - 1; ++j, i += spec)
  {
    res.push_back(pool.pushTask(parallel_sum , 
                                 vec, i, i + spec));
  }
  int result = parallel_sum(vec, i, vec.size());
  //get results
  for(auto& fu : res){
    result += fu.get();
  }
  std::cout << result << std::endl; 
  LOG("hello %d", 2);
}
