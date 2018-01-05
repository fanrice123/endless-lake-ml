#ifndef MLIMAGE_H
#define MLIMAGE_H
#include <opencv2/opencv.hpp>
#include <deque>
#include <vector>
#include <type_traits>
#include <mutex>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <map>
#include <algorithm>
#include <execution>
#include <cstdint>

enum class region_type : std::int8_t { WATER, PATH, PLAYER };

class MlImageProcessor {
public:
    typedef std::map<std::string, std::map<std::string, cv::Scalar>> settings_type;
    typedef std::vector<cv::Point> contour_type;

    MlImageProcessor(CbType&&, const std::string&);

    void set_roi(const cv::Mat&);
    std::vector<contour_type> extract_feature(const cv::Mat&) const;
private:
    void load_settings(const std::string&);

    cv::Rect cropper;
    settings_type settings;
};

MlImageProcessor::MlImageProcessor(const std::string& setting_path)
{
    load_settings(setting_path);
}

void MlImageProcessor::set_roi(const cv::Mat& img)
{
    Mat thr;
    cv::cvtColor(img, thr, COLOR_BGR2GRAY);
    threshold(thr, thr, 25, 255, THRESH_BINARY);

    std::vector<contour_type> contours;

    cv::findContours(thr, contours, RETR_CCOMP, CHAIN_APPROX_SIMPLE);

    contour_type* largest_contour_ptr;
    double largest_area = 0.0;

    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);

        if (area > largest_area) {
            largest_area = area;
            largest_contour_ptr = &contour;
        }
    }

    cropper = cv::boundingRect(*largest_contour_area);

}

std::vector<contour_type> MlImageProcessor::extract_feature(const cv::Mat& img)
{
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
    std::vector<region_type> features(threshold, region_type::WATER);

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

    // hopefully NRVO applies here
    return features;
}

void MlImageProcessor::load_settings(const std::string& setting_path)
{
    rapidjson::Document d;
    ifstream fs("setting.json");
    rapidjson::IStreamWrapper isw(fs);

    d.ParseStream(isw);
    const rapidjson::Value& val = d["rgb"];

    for (auto& setting : val.GetObject()) {
        settings_type::mapped_type subsettings;

        for (auto& subsetting: setting.GetObject()) {
            auto& subsetting_name = subsetting.name.GetString();
            settings_type::mapped_type::mapped_type rgb;

            std::size_t i = 0;
            auto itr = subsetting.Begin();
            int r = itr->GetInt();
            int g = itr->GetInt();
            int b = itr->GetInt();

            subsettings[subsetting_name] = cv::Scalar(b, g, r);
        }
        settings[setting.name.GetString()] = std::move(subsettings);
    }
}


#endif
