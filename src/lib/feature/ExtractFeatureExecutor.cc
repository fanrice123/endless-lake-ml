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
    local_thread = std::move(std::thread{[&, cropper] {
        std::unique_lock<std::mutex> lck{buffer_m};
        while (!stop.load(std::memory_order_acquire)) {
            cv.wait(lck, [&] { 
                return status.load(std::memory_order_acquire) == exec_status::READY;
            });
            if (stop.load(std::memory_order_acquire))
                break;
            status.store(exec_status::ONGOING, std::memory_order_release);
            task(cropper, settings);
            status.store(exec_status::EMPTY, std::memory_order_release);
        }
    }});
            
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

std::future<std::vector<float>> 
ExtractFeatureExecutor::operator()(const cv::Mat& img)
{

    if (stop.load(std::memory_order_acquire))
        throw std::logic_error("ExtractFeatureExecuter is suspended but being invoked.");
    if (status.load(std::memory_order_acquire) != exec_status::EMPTY)
        throw std::logic_error("buffer of ExtractFeatureExecuter is not empty but being revised.");

    decltype(task) new_task([&](const cv::Rect& cropper, settings_type& settings) {

            cv::Mat resized_img;
            cv::Mat target_img;
            cv::Mat coin_img, path_img, player_img;
            cv::Mat pathway_img;
            std::vector<contour_type> contours;

            
            // crop image
            cv::Mat roi = img(cropper);

            // resize image
            constexpr int roi_w = 480, roi_h = 840;
            constexpr int roi_area = roi_w * roi_h;
        
            constexpr int box_w = 40, box_h = 40;
            constexpr double threshold_perc = 25 / 100;
            constexpr int box_area = box_w * box_h;
            constexpr int num_box_in_roi = roi_area / box_area;
            constexpr int threshold = static_cast<int>(box_w * box_h * threshold_perc);
            std::vector<float> features(num_box_in_roi * 2, 0.0f);

            cv::resize(roi, resized_img, cv::Size(roi_w, roi_h), 0, 0, CV_INTER_LINEAR);
            cv::Mat kernel = cv::Mat::ones(5, 5, CV_8UC4);

            cv::morphologyEx(resized_img, target_img, cv::MORPH_OPEN, kernel);
            auto& map_coin = settings["coin"];
            auto& map_path = settings["path"];
            auto& map_player = settings["player"];

            cv::inRange(target_img, map_coin["min"], map_coin["max"], coin_img);
            cv::inRange(target_img, map_path["min"], map_path["max"], path_img);
            cv::bitwise_and(coin_img, path_img, pathway_img);
            cv::inRange(target_img, map_player["min"], map_path["max"], player_img);
        
            std::size_t index = 0;
            // extract feature of pathway
            for (int i = 0; i != pathway_img.cols; i += box_w) {
                for (int j = 0; j != pathway_img.rows; j += box_h) {
                    cv::Rect sub_roi_cropper(i, j, box_w, box_h);
                    cv::Mat sub_roi = pathway_img(sub_roi_cropper);
        
                    /*
                    if (cv::countNonZero(sub_roi) > threshold)
                        features[index] = region_type::PATH;
                    */
                    features[index] = cv::countNonZero(sub_roi);
        
                    ++index;
                }
            }

            // extract feature of player
            for (int i = 0; i != player_img.cols; i += box_w) {
                for (int j = 0; j != player_img.rows; j += box_h) {
                    cv::Rect sub_roi_cropper(i, j, box_w, box_h);
                    cv::Mat sub_roi = player_img(sub_roi_cropper);
        
                    /*
                    if (cv::countNonZero(sub_roi) > threshold)
                        features[index] = region_type::PLAYER;
                    */
                    features[index] = cv::countNonZero(sub_roi);
        
                    ++index;
                }
            }

        
            // hopefully NRVO applies here
            return features;

    });

    task = std::move(new_task);


    status.store(exec_status::READY, std::memory_order_release);
    cv.notify_one();

    return task.get_future();
}

