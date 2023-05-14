#ifndef BASE_THREADPOOL_H
#define BASE_THREADPOOL_H

#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <iostream>
#include <functional>

#include "common.h"

class ThreadPool
{
 public:
  typedef std::unique_ptr<std::thread> ThreadPtr;

  explicit ThreadPool(const std::string& name = std::string());
  ~ThreadPool();

  void start(int numThreads);
  void stop();

  template<typename T>
  void push(T pkg_task);

 private:

  class BaseTaskWrapper
  {
   public:
    BaseTaskWrapper() = default;
    virtual ~BaseTaskWrapper() = default;
    virtual void excute() = 0; 
   private:
    BaseTaskWrapper(const BaseTaskWrapper&) = delete;
    BaseTaskWrapper& operator=(BaseTaskWrapper&) = delete;
  };

  template<typename T>
  class TaskWrapper : public BaseTaskWrapper
  {
   public:
    TaskWrapper(T&& pkg_task);
    void excute() ;
   private:
    T data_;
  };

  typedef std::unique_ptr<BaseTaskWrapper> WapperPtr;
  
  ThreadPool(ThreadPool&) = delete;
  ThreadPool& operator=(ThreadPool&) = delete;
  void run();
  WapperPtr take();
  void static ThreadPtrJoin(ThreadPtr& ptr);

  std::mutex mutex_;
  std::condition_variable cond_;
  std::string name_;
  std::vector<ThreadPtr> threads_;
  // std::deque<Task> queue_;
  // std::deque<PkgTaskPtr> tasks_;
  std::deque<WapperPtr> wapper_tasks_;
  bool running_;
};

//template function should be defined in the same file with declaration.
template<typename T>
void ThreadPool::push(T pkg_task)
{
  if (threads_.empty())
  {
    pkg_task();
  } else {
    WapperPtr task_ptr{new TaskWrapper<T>(std::move(pkg_task))};
    std::lock_guard<std::mutex> lock{mutex_};
    wapper_tasks_.push_back(std::move(task_ptr));
    cond_.notify_one();
  }
}

//Task Wrapper
template<typename T>
ThreadPool::TaskWrapper<T>::TaskWrapper(T&& pkg_task):
             data_(std::move(pkg_task)){};

template<typename T>
void ThreadPool::TaskWrapper<T>:: excute(){
  data_();
}

#endif
