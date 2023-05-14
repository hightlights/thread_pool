#include "ThreadPool.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>

ThreadPool::ThreadPool(const std::string& name)
  : mutex_(),
    cond_(),
    name_(name),
    running_(false)
{
}

ThreadPool::~ThreadPool()
{
  if (running_)
  {
    stop();
  }
}

void ThreadPool::start(int numThreads)
{
  assert(threads_.empty());
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
  {
    threads_.push_back(std::move(ThreadPtr(new std::thread(
           std::bind(&ThreadPool::run, this)))));
  }
}

void ThreadPool::ThreadPtrJoin(ThreadPtr& p){
  p->join();
}

void ThreadPool::stop()
{
  {
  std::lock_guard<std::mutex> lock{mutex_};
  running_ = false;
  cond_.notify_all();
  }
  for(auto& thread_ptr : threads_)
  {
    thread_ptr->join();
  }
}


ThreadPool::WapperPtr ThreadPool::take()
{
  std::unique_lock<std::mutex> lck(mutex_);
  //in case of spurious wakeup
  while (wapper_tasks_.empty() && running_)
  {
    cond_.wait(lck);
  }
  std::unique_ptr<BaseTaskWrapper> task_ptr;
  if(!wapper_tasks_.empty())
  {
    task_ptr = std::move(wapper_tasks_.front());
    wapper_tasks_.pop_front();
  }
  return std::move(task_ptr);
}

void ThreadPool::run()
{
  try
  {
    while (running_)
    {
      WapperPtr task_ptr(std::move(take()));
      if(nullptr != task_ptr)
      {
        task_ptr->excute();
      }
    }
  }
  catch (const std::exception& ex)
  {
    fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
    fprintf(stderr, "reason: %s\n", ex.what());
    abort();
  }
  catch (...)
  {
    fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
    throw; // rethrow
  }
}


