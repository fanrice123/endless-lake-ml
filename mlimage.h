#ifndef MLIMAGE_H
#define MLIMAGE_H
#include <opencv2/opencv.hpp>
#include <vector>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <map>
#include <cstdint>
#include <future>
#include <thread>
#include "ExtractFeatureExecutor.h"

using region_type = region_t_

class MlImageProcessor {
public:
    typedef std::map<std::string, std::map<std::string, cv::Scalar>> settings_type;
    typedef std::vector<cv::Point> contour_type;

    MlImageProcessor(const std::string&);

    void set_roi(const cv::Mat&);
    std::vector<region_type> extract_feature(const cv::Mat&);
    std::future<std::vector<region_type>> extract_feature_async(const cv::Mat&);
private:
    void load_settings(const std::string&);

    cv::Rect cropper;
    settings_type settings;
    ExractFeatureExecutor do_extract_feature;
};

#endif
