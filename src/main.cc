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

int main()
{
  int thread_nums = 5; 
  ThreadPool pool(thread_nums);
  //pool.start(thread_nu);
  std::vector<int> vec(10000000, 1);
  int spec = vec.size() / thread_nums;
  std::vector<std::future<int>> res;
  //push to thread pool
  pool.start();
  int i = 0;
  for(int j = 0; j < thread_nums - 1; ++j, i += spec)
  {
    auto future = pool.pushTaskReturnFuture(parallel_sum, vec, i, i + spec);
    res.push_back(std::move(future));
  }
  int result = parallel_sum(vec, i, vec.size());
  //get results
  for(auto& fu : res){
    result += fu.get();
  }
  std::cout << result << std::endl; 
  // std::this_thread::sleep_for(std::chrono::milliseconds(300000));
  // LOG("hello %d", 2);
}
