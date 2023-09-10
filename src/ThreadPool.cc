#include "ThreadPool.h"

#include <assert.h>
#include <stdio.h>
#include <algorithm>


Thread::Thread(ThreadPool& pool): 
pool_(pool)
{
  thread_ = std::thread(std::bind(&Thread::loopInThisThread, this));
}

Thread::~Thread() {
  if(thread_.joinable()) {
    thread_.join();
  }
}

void Thread::TasksQueue::
pushBack(Thread::TaskWrapperPtr task_ptr)
{
  std::lock_guard<std::mutex> lock(mtx_);
  que_.push_back(std::move(task_ptr));
  cndt_var_.notify_all();
}

//pop fail will return a null_ptr instead of being blocked.
Thread::TaskWrapperPtr
Thread::TasksQueue::tryPopBack()
{
  TaskWrapperPtr task;
  TryLockGuard<std::mutex> lock(mtx_);
  if (lock && !que_.empty())
  {
    task = std::move(que_.front());
    que_.pop_front();
  }
  return task;
}

//pop fail will be blocked.
Thread::TaskWrapperPtr
Thread::TasksQueue::PopBack()
{
  TaskWrapperPtr task;
  std::lock_guard<std::mutex> lock(mtx_);
  if(!que_.empty()) {
    task = std::move(que_.front());
    que_.pop_front();
    mtx_.unlock();
  }
  return task;
}

bool Thread::TasksQueue::empty() {
  std::lock_guard<std::mutex> lock(mtx_);
  return que_.empty();
}

Thread::TaskWrapperPtr 
Thread::getOneTask() {
  return task_que_.tryPopBack();
}

void Thread::stealTask() {
  if(pool_.stop_) {
    return;
  }

  while(task_que_.empty()) {
    bool is_steal_success = false;
    for(const auto& thread : pool_.threads_) {
      TaskWrapperPtr task = thread->getOneTask();
      LOG("Thread %d(id) steal task from thread %d(id)", getID(), thread->getID());
      if(task) {
        task->excute();
        is_steal_success = true;
      }
      if(!task_que_.empty()) {
        return;
      }
    }
    if(false == is_steal_success) {
      return;
    }
  }
}

// 1. tryPopBack()  and execute tasks in loop, if there are tasks.
// 2. if there are no tasks in the task queue, stealing task from other threads.
// 3. if steal task fail, block in the thread local task queue.
void Thread::loopInThisThread()
{
  //blocked, be waited up by pool_.start();
  thread_id_ = std::this_thread::get_id();
  waitStart();
  while(!pool_.stop_){
    while(!task_que_.empty() && !pool_.stop_) {
      TaskWrapperPtr task = task_que_.tryPopBack();
      if(task) {
        task->excute();
      }
    }
    stealTask();
    if(!pool_.stop_) {
      TaskWrapperPtr task = task_que_.PopBack();
      if(task) {
        task->excute();
      }
    }
  }
}

void Thread::waitStart() {
  std::unique_lock<std::mutex> lock(pool_.start_mtx_);
  pool_.start_.wait(lock);
}

ThreadPool::ThreadPool(unsigned int thread_num/* trhead_num = 1 */) 
:dispt_task_id_(0), stop_(false) 
{
  if (thread_num > std::thread::hardware_concurrency()) {
    thread_num = std::thread::hardware_concurrency();
  }
  for(int i = 0; i < thread_num; ++i){
    ThreadPtr thread_ptr{std::make_unique<Thread>(*this)};
    threads_.push_back(std::move(thread_ptr));
  }
}

void ThreadPool::start() {
  start_.notify_all();
}
