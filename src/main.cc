#include "ThreadPool.h"

#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <functional>

std::vector<int> vec(1000,0);

void work(int i){
   for(int j = i; j < i + 100; ++j){
      vec[j] = j;
   }
}

int main(){
   ThreadPool pool(std::string("test")); 
   pool.start(3);
   for(int i = 0; i < 1000; i += 100){
      std::packaged_task<void()> task(std::bind(work, i));
      pool.push(std::move(task));
   }
   std::this_thread::sleep_for(std::chrono::milliseconds(1000));
   pool.stop();   
   for(int i = 5; i < 1000; i += 100) {
      std::cout << vec[i] << " : " << i << std::endl;
   }
}
