#include "Executor.hxx"

#include <NGenXXLog.hxx>

#pragma mark - Worker

NGenXX::Core::Concurrent::Worker::Worker() : Worker(1000uz)
{
}

NGenXX::Core::Concurrent::Worker::Worker(size_t sleepMicroSecs) : sleepMicroSecs{sleepMicroSecs}
{
#if defined(__cpp_lib_jthread)
    this->thread = std::jthread([this](std::stop_token stoken) {
        while (!stoken.stop_requested())
#else
    this->thread = std::thread([this]() {
        while (!shouldStop.load())
#endif
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            // Wait for tasks or stop signal
            this->cv.wait(lock, [this
#if defined(__cpp_lib_jthread)
                , &stoken
#endif
                ]() {
                return !this->taskQueue.empty() || 
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
                ;
            });
            
            if (
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
            ) 
            {
                break;
            }

            if (taskQueue.empty()) [[unlikely]] 
            {
                continue;
            }

            auto func = std::move(taskQueue.front());
            taskQueue.pop();
            lock.unlock();

            if (func) [[likely]] 
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Worker@{} run task on thread:{}", reinterpret_cast<uintptr_t>(this), currentThreadId());
                func();
            }
        }
    });
}

NGenXX::Core::Concurrent::Worker::~Worker()
{
#if defined(__cpp_lib_jthread)
    // jthread automatically handles stop request and join
#else
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->shouldStop = true;
    }
    this->cv.notify_one();
    
    if (this->thread.joinable()) 
    {
        this->thread.join();
    }
#endif
}

void NGenXX::Core::Concurrent::Worker::sleep()
{
    NGenXX::Core::Concurrent::sleep(this->sleepMicroSecs);
}

NGenXX::Core::Concurrent::Worker& NGenXX::Core::Concurrent::Worker::operator>>(TaskT&& task)
{
    {
        std::lock_guard<std::mutex> lock(this->mutex);
        this->taskQueue.emplace(std::move(task));
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Worker@{} taskCount:{}", reinterpret_cast<uintptr_t>(this), this->taskQueue.size());
    }
    
    this->cv.notify_one();
    
    return *this;
}

#pragma mark - Executor

NGenXX::Core::Concurrent::Executor::Executor() : Executor(0uz, 1000uz)
{
}

NGenXX::Core::Concurrent::Executor::Executor(size_t workerPoolCapacity, size_t sleepMicroSecs) : workerPoolCapacity{workerPoolCapacity}, sleepMicroSecs{sleepMicroSecs}
{
    if (this->workerPoolCapacity == 0)
    {
        this->workerPoolCapacity = countCPUCore();
    }
    this->workerPool.reserve(this->workerPoolCapacity);
}

NGenXX::Core::Concurrent::Executor::~Executor()
{
    this->workerPool.clear();
}

NGenXX::Core::Concurrent::Executor& NGenXX::Core::Concurrent::Executor::operator>>(TaskT&& task)
{
    std::lock_guard<std::mutex> lock(this->mutex);

    if (this->workerPool.size() < this->workerPoolCapacity)
    {
        auto worker = std::make_unique<Worker>(this->sleepMicroSecs);
        this->workerPool.emplace_back(std::move(worker));
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor created new worker, poolSize:{} poolCapacity:{} cpuCores:{}", 
                        this->workerPool.size(), this->workerPoolCapacity, countCPUCore());
    }
    
    *(this->workerPool.at(this->workerIndex)) >> std::move(task);
    
    this->workerIndex = (this->workerIndex + 1) % this->workerPoolCapacity;
    
    return *this;
}