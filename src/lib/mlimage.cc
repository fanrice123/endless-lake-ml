#include "mlimage.h"
#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <cstdint>
#include <future>
#include <condition_variable>
#include "./feature/ExtractFeatureExecutor.h"
#include <iostream>

MlImageProcessor::MlImageProcessor(const std::string& setting_path)
{
    load_settings(setting_path);
}

void MlImageProcessor::set_roi(const cv::Mat& img)
{
    cv::Mat thr;
    cv::cvtColor(img, thr, cv::COLOR_BGR2GRAY);
    threshold(thr, thr, 25, 255, cv::THRESH_BINARY);

    std::vector<contour_type> contours;

    cv::findContours(thr, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat img2 = img;

    const contour_type* largest_contour_ptr;
    double largest_area = 0.0;

    int i = 0, index = 0;;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);

        if (area > largest_area) {
            largest_area = area;
            largest_contour_ptr = &contour;
            index = i;
        }
        ++i;
    }
    cv::drawContours(img2, contours, index, cv::Scalar(0, 255, 0));
    cv::imshow("Test", img2);
    cv::waitKey(0);

    do_extract_feature.start(cv::boundingRect(*largest_contour_ptr), settings);

}

std::vector<float> MlImageProcessor::extract_feature(const cv::Mat& img)
{
    return extract_feature_async(img).get();
}

std::future<std::vector<float>> 
MlImageProcessor::extract_feature_async(const cv::Mat& img)
{
    return do_extract_feature(img);
}

void MlImageProcessor::load_settings(const std::string& setting_path)
{
    rapidjson::Document d;
    std::ifstream fs("setting.json");
    rapidjson::IStreamWrapper isw(fs);

    d.ParseStream(isw);
    const rapidjson::Value& val = d["rgb"];

    for (auto& setting : val.GetObject()) {
        settings_type::mapped_type subsettings;

        for (auto& subsetting: setting.value.GetObject()) {
            auto subsetting_name = subsetting.name.GetString();
            settings_type::mapped_type::mapped_type rgb;

            auto itr = subsetting.value.Begin();
            int r = itr->GetInt();
            int g = itr->GetInt();
            int b = itr->GetInt();

            subsettings[subsetting_name] = cv::Scalar(b, g, r);
        }
        settings[setting.name.GetString()] = std::move(subsettings);
    }
}

