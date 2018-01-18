#ifndef EXTRACTFEATUREEXECUTOR_H
#define EXTRACTFEATUREEXECUTOR_H
#include <opencv2/opencv.hpp>
#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "count_semaphore.h"

enum class region_type : std::int8_t { WATER, PATH, PLAYER };
enum class ExecStat : std::int8_t { EMPTY, READY, ONGOING };

class ExtractFeatureExecutor {
public:
    typedef std::vector<cv::Point> contour_type;

    ExtractFeatureExecutor();
    ~ExtractFeatureExecutor();

    ExecStat get_status() noexcept;

    std::future<std::vector<region_type>> operator()(const cv::Mat&);

private:
    std::packaged_task<std::vector<region_type>(const cv::Rect&)> task;
    std::atomic<ExecStatus> status;
    std::atomic_bool stop;
    std::condition_variable cv;
    std::mutex buffer_m;
    std::thread local_thread;
};

#endif
