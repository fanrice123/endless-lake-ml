#include "mlimage.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <cstdint>
#include <future>
#include <condition_variable>

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
    return extract_feature_async(img).get();
}

std::future<std::vector<contour_type>> 
MlImageProcessor::extract_feature_async(const cv::Mat& img)
{
    return do_extract_feature(img);
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

