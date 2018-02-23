#include "ExtractFeatureExecutor.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <vector>
#include <future>
#include <utility>

ExtractFeatureExecutor::ExtractFeatureExecutor()
    : stop(true), status(exec_status::EMPTY)
{}

void ExtractFeatureExecutor::start(const cv::Rect& cropper, settings_type& settings)
{

    stop.store(false, std::memory_order_release);
    status.store(exec_status::EMPTY, std::memory_order_release);
    
    local_thread = std::thread([&, cropper] {
        std::unique_lock<std::mutex> lck{buffer_m};
        while (!stop.load(std::memory_order_acquire)) {
            cv.wait(lck, [&] { 
                return status.load(std::memory_order_acquire) == exec_status::READY ||
                       stop.load(std::memory_order_acquire);
            });
            if (stop.load(std::memory_order_acquire))
                break;
            status.store(exec_status::ONGOING, std::memory_order_release);
            task(cropper, settings);
            status.store(exec_status::EMPTY, std::memory_order_release);
        }
    });
            
}

void ExtractFeatureExecutor::end()
{
    stop.store(true, std::memory_order_release);
    cv.notify_one();
    if (local_thread.joinable())
        local_thread.join();
}

ExtractFeatureExecutor::~ExtractFeatureExecutor()
{
    end();
}

exec_status ExtractFeatureExecutor::get_status() noexcept
{
    return status.load(std::memory_order_acquire);
}

