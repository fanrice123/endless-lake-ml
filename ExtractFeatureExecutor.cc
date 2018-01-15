#include "ExtractFeatureExecutor.h"
#include <opencv2/opencv.hpp>
#include <atomic>
#include <vector>
#include <future>


ExtractFeatureExecutor::ExtractFeatureExecutor(const cv::Rect& cropper)
    : stop(false), status(ExecStat::EMPTY), local_thread([this, &cropper] {
        while (!stop.load(std::memory_order_acquire)) {
            cv::Mat resized_img;
            cv::Mat target_img;
            cv::Mat coin_img, path_img, player_img;
            cv::Mat pathway_img;
            std::vector<contour_type> contours;

            cv.wait(buffer_m, [&status] { 
                return status.load(std::memory_order_acquire) == ExecStat::READY;
            });

            status.store(ExecStat::ONGOING, std::memory_order_release);

            // crop image
            cv::Mat roi = buffer(cropper);

            // resize image
            constexpr int roi_w = 480, roi_h = 840;
            constexpr int roi_area = roi_w * roi_h;
        
            constexpr int box_w = 40, box_h = 40;
            constexpr double threshold_perc = 25 / 100;
            constexpr int box_area = box_w * box_h;
            constexpr int num_box_in_roi = roi_area / box_area;
            constexpr int threshold = static_cast<int>(box_w * box_h * threshold_perc);
            std::vector<region_t_> features(threshold, region_t_::WATER);

            cv::resize(roi, resized_img, cv::Size(roi_w, roi_h), 0, 0, CV_INTER_LINEAR);
            cv::Mat kernel = cv::Mat::ones(5, 5, CV_8UC4);

            cv::morphologyEx(resized_img, target_img, MORPH_OPEN, kernel);
            auto& map_coin = settings["coin"];
            auto& map_path = settings["path"];
            auto& map_player = settings["player"];


            cv::inRange(target_img, map_coin["min"], map_coin["max"], coin_img);
            cv::inRange(target_img, map_path["min"], map_path["max"], path_img);
            cv::bitwise_and(coin_img, path_img, pathway_img);
            cv::inRange(target_img, map_player["min"], map_path["max"], player_img);
        
            std::size_t index = 0;
            for (int i = 0; i != pathway_img.cols; i += box_w) {
                for (int j = 0; j != pathway_img.rows; j += box_h) {
                    cv::Rect sub_roi_cropper(i, j, bow_w, box_h);
                    cv::Mat sub_roi = pathway_img(sub_roi_cropper);
                    cv::Mat sub_roi_player = player_img(sub_roi_cropper);
        
                    if (cv::countNonZero(sub_roi) > threshold)
                        features[index] = region_type::PATH;
        
                    if (cv::countNonZero(sub_roi_player) > threshold)
                        features[index] = region_type::PLAYER;
                    ++index;
                }
            }

            status.store(ExecStat::EMPTY, std::memory_order_release);
        
            // hopefully NRVO applies here
            return features;
        }

{
}

ExtractFeatureExecutor::~ExtractFeatureExecutor()
{
    stop.store(true, std::memory_order_release);
    local_thread.join();
}

ExecStat ExtractFeatureExecuter::get_status() noexcept
{
    return status.load(std::memory_order_acquire);
}

std::future<std::vector<contour_type>> 
ExtractFeatureExecuter::operator()(const cv::Mat& img)
{
    if (stop.load(std::memory_order_acquire))
        throw std::logic_error("ExtractFeatureExecuter is suspended but being invoked.");
    if (status.load(std::memory_order_acquire) != ExecStat::EMPTY)
        throw std::logic_error("buffer of ExtractFeatureExecuter is not empty but being revised.");

    buffer = img;
    cv.post();
}

