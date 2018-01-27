#ifndef MLDATA_H
#define MLDATA_H
#include <std::vector>
#include <tuple>

std::tuple<std::vector<std::vector<float>>, std::vector<int>>
load_data(const std::string& X_path, const std::string& Y_path);

template <typename XType, typename YType>
void dataset_shuffle(XType&, YType&);

template <typename XType, typename YType>
std::tuple<XType, YType, XType, YType> cv_split(XType&&, YType&&, double);

#endif // MLDATA_H
