#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <functional>

#include "common.h"

template<typename T>
class TryLockGuard
{
 public:
  TryLockGuard(T& mtx) : mtx_(mtx), is_locked_(false)
  {
    if(mtx_.try_lock()) {
      is_locked_ = true;
    }
  }

  ~TryLockGuard() 
  {
    if(is_locked_) {
      mtx_.unlock();
    }
  }

  bool isLocked() const
  {
    return is_locked_;
  }

  explicit operator bool() const
  { 
    return isLocked(); 
  }

 private:
  T& mtx_;
  bool is_locked_;
};//TryLockGuard

class ThreadPool;

class Thread
{
 public:
  Thread(ThreadPool& pool);
  ~Thread();
  Thread(const Thread&) = delete;
  Thread& operator= (const Thread&) = delete;
  Thread(const Thread&&) = delete;
  Thread& operator= (const Thread&&) = delete;
 
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
    TaskWrapper(T task)
      :data_(std::move(task)){};
    ~TaskWrapper()=default;
    void excute()
    {
      data_();
    }

   private:
    T data_;
  };

 public:
  using TaskWrapperPtr = std::unique_ptr<BaseTaskWrapper>;
  using ThreadTD = std::thread::id;
 private:

  class TasksQueue
  {
   public:
    TasksQueue() = default;
    ~TasksQueue() = default;

    void pushBack(TaskWrapperPtr task_ptr);
    TaskWrapperPtr tryPopBack();
    TaskWrapperPtr PopBack();
    bool empty();

   private:
    std::mutex mtx_;
    std::condition_variable cndt_var_;
    std::deque<TaskWrapperPtr> que_;
  };

private:
  void loopInThisThread();

 public:
  template <typename Func, typename... Args>
  void pushTask(Func callable, Args&&... args);

  template <typename Func, typename... Args>
  auto pushTaskReturnFuture(Func task, Args&&... args);

  void stealTask();
  TaskWrapperPtr getOneTask();

  void waitStart();

  ThreadTD getID(){
    return thread_id_;
  }
 private:
  std::thread thread_;
  ThreadTD thread_id_;
  TasksQueue task_que_;
  ThreadPool& pool_;
};//class Thread

template<typename Func, typename... Args>
void Thread::pushTask(Func callable, Args&& ... args) {
  using RetureType = decltype(callable(std::forward<Args>(args)...));
  std::function<RetureType()> func{std::bind(callable, std::forward<Args>(args)...)};
  TaskWrapperPtr task_ptr{new TaskWrapper<decltype(func)>(std::move(func))};
  task_que_.pushBack(std::move(task_ptr));
}

template <typename Func, typename... Args>
auto Thread::pushTaskReturnFuture(Func callable, Args&&... args) {
  using RetureType = decltype(callable(std::forward<Args>(args)...));
  std::function<RetureType()> func{std::bind(callable, std::forward<Args>(args)...)};
  std::packaged_task<RetureType()> pkg_task{std::move(func)};
   auto future = pkg_task.get_future();
  TaskWrapperPtr task_ptr{new TaskWrapper<decltype(pkg_task)>(std::move(pkg_task))};
  task_que_.pushBack(std::move(task_ptr));
  return future;
}

// Threadpool
// usage:
class ThreadPool 
{
 public:
  ThreadPool(unsigned int thread_num = 1);
  ~ThreadPool() {
    stop();
  }

  template <typename Func, typename... Args>
  void pushTask(Func task, Args&&... args);

  template <typename Func, typename... Args>
  auto pushTaskReturnFuture(Func task, Args&&... args);

  void start();
 private:
  friend class Thread;
  using ThreadPtr = std::unique_ptr<Thread>;
  using ThreadVec = std::vector<ThreadPtr>;
  
 private: 
 void stop() {
  stop_ = true;
}

void roundDisptID() {
  if(dispt_task_id_ == (threads_.size() - 1)) {
    dispt_task_id_ = 0;
  } else {
    dispt_task_id_++;
  }
}

 private:
  std::atomic_uint dispt_task_id_;
  ThreadVec threads_;
  std::atomic_bool stop_;
  std::condition_variable start_;
  std::mutex start_mtx_;
};

// dispatch task by the way of round robin. 
template <typename Func, typename... Args>
void ThreadPool::pushTask(Func task, Args&&... args) {
  threads_[dispt_task_id_]->pushTask(std::move(task), std::forward<Args>(args)...);
  roundDisptID();
}

template <typename Func, typename... Args>
auto ThreadPool::pushTaskReturnFuture(Func task, Args&&... args) {
  auto future = threads_[dispt_task_id_]->pushTaskReturnFuture(std::move(task), std::forward<Args>(args)...);
   roundDisptID();
  return future;
}
#endif
