#ifndef EXTRACTFEATUREEXECUTOR_H
#define EXTRACTFEATUREEXECUTOR_H
#include <opencv2/opencv.hpp>
#include <vector>
#include <future>
#include <thread>
#include <atomic>
#include <memory>
#include <condition_variable>

enum class exec_status : std::int8_t { EMPTY, READY, ONGOING };

class ExtractFeatureExecutor {
public:
    typedef std::vector<cv::Point> contour_type;
    typedef std::unordered_map<std::string, std::unordered_map<std::string, cv::Scalar>> settings_type;


    ExtractFeatureExecutor();
    ~ExtractFeatureExecutor();

    exec_status get_status() noexcept;
    void start(const cv::Rect&, settings_type&);
    void end();

    template <std::size_t box_h = 40,
              std::size_t box_w = 40, 
              std::size_t roi_h = 840,
              std::size_t roi_w = 480>
    std::future<std::vector<float>> operator()(const cv::Mat&);

private:
    std::packaged_task<std::vector<float>(const cv::Rect&, settings_type&)> task;
    std::atomic<exec_status> status;
    std::atomic_bool stop;
    std::condition_variable cv;
    std::mutex buffer_m;
    std::thread local_thread;
};

template <std::size_t box_h,
          std::size_t box_w, 
          std::size_t roi_h,
          std::size_t roi_w>
std::future<std::vector<float>>  ExtractFeatureExecutor::operator()(const cv::Mat& img)
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
            constexpr int roi_area = roi_w * roi_h;
        
            constexpr double threshold_perc = 25 / 100;
            constexpr int box_area = box_w * box_h;
            constexpr int num_box_in_roi = roi_area / box_area;
            constexpr int threshold = static_cast<int>(box_w * box_h * threshold_perc);
            std::vector<float> features(num_box_in_roi * 2, 0.0f);

            cv::resize(roi, resized_img, cv::Size(roi_w, roi_h), 0, 0, CV_INTER_LINEAR);
            cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE,
                                                       cv::Size(5, 5));

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

#endif
