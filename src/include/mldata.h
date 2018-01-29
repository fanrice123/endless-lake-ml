#ifndef MLDATA_H
#define MLDATA_H
#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tuple>
#include <vector>
#include <random>
#include <iterator>
#include <algorithm>
#include <type_traits>

template <template <typename> typename XType=std::vector,
          template <typename> typename XLineType=std::vector,
          typename XValueType=float,
          template <typename> typename YType=std::vector,
          typename YValueType=int>
std::tuple<XType<XLineType<XValueType>>, YType<YValueType>>
load_data(const std::string& X_path, const std::string& Y_path);

template <typename XType, typename YType>
void dataset_shuffle(XType&, YType&);

template <typename XType, typename YType>
std::tuple<std::decay_t<XType>,
           std::decay_t<YType>, 
           std::decay_t<XType>, 
           std::decay_t<YType>>
cv_split(XType&& X, YType&& Y, double percent);

template <template <typename> typename XType=std::vector,
          template <typename> typename XLineType=std::vector,
          typename XValueType=float,
          template <typename> typename YType=std::vector,
          typename YValueType=int>
std::tuple<XType<XLineType<XValueType>>, YType<YValueType>>
load_data(const std::string& X_path, const std::string& Y_path)
{
    std::ifstream X_file(X_path);
    std::ifstream Y_file(Y_path);
    std::string line;
    XType<XLineType<XValueType>> X;
    YType<YValueType> Y;

    while (std::getline(X_file, line)) {
        std::stringstream X_line_stream(line);
        XLineType<XValueType> X_line;
        std::string feature;

        while (std::getline(X_line_stream, feature, ',')) {
            std::stringstream feature_stream(feature);
            YValueType feature_val;
            feature_stream >> feature_val;
            X_line.push_back(feature_val);
        }
        X.push_back(std::move(X_line));
    }

    std::istream_iterator<YValueType> Y_itr(Y_file);
    std::copy(Y_itr, std::istream_iterator<int>(), std::back_inserter(Y));

    return std::make_tuple(X, Y);
}

template <typename XType, typename YType>
void dataset_shuffle(XType& X, YType& Y) {
    std::random_device rd;
    std::mt19937_64 engine(rd());
    auto n = X.size();
    std::uniform_int_distribution<size_t> random(0, n - 1);

    for (size_t i = n - 1; i > 0; --i) {
        using std::swap;
        auto j = random(engine);
        swap(X[i], X[j]);
        swap(Y[i], Y[j]);
    }
}

template <typename XType, typename YType>
std::tuple<std::decay_t<XType>,
           std::decay_t<YType>, 
           std::decay_t<XType>, 
           std::decay_t<YType>>
cv_split(XType&& X, YType&& Y, double percent) {
    auto n = X.size();

    dataset_shuffle(X, Y);

    size_t cv_size = static_cast<size_t>(round(n * percent));

    std::decay_t<XType> train_X, cv_X;
    std::decay_t<YType> train_Y, cv_Y;

    for (size_t i = 0; i != cv_size; ++i) {
        cv_X.push_back(std::move(X[i]));
        cv_Y.push_back(std::move(Y[i]));
    }

    for (size_t i = cv_size; i != n; ++i) {
        train_X.push_back(std::move(X[i]));
        train_Y.push_back(std::move(Y[i]));
    }

    return std::make_tuple(train_X, train_Y, cv_X, cv_Y);
}

#endif // MLDATA_H
