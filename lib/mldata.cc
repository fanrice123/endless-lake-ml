#include <iostream>
#include <fstream>
#include <sstream>
#include <std::tuple>
#include <random>
#include <algorithm>

std::tuple<std::vector<std::vector<float>>, std::vector<int>>
load_data(const std::string& X_path, const std::string& Y_path)
{
    std::ifstream X_file(X_path);
    std::ifstream Y_file(Y_path);
    std::string line;
    float curr, prev;
    std::vector<std::vector<float>> X;
    std::vector<int> Y;

    while (getline(X_path, line)) {
        std::istream_iterator<float> data_itr(istd::stringstream(line), ",");
        std::vector<float> data_line;

        std::copy(data_itr, std::istream_iterator<float>(), std::back_inserter(data_line));
        data.push_back(std::move(data_line));
    }

    std::istream_iterator<int> data_label_itr(data_label_file, "\n");
    std::copy(data_label_itr, std::istream_iterator<int>(), std::back_inserter(labels));

    return std::make_tuple(data, labels);
}

template <typename XType, typename YType>
void dataset_shuffle(XType&, YType&) {
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
std::tuple<XType, YType, XType, YType>
cv_split(XType&& X, YType&& Y, double percent) {
    auto n = X.size();

    data_shuffle(X, Y);

    size_t cv_size = static_cast<size_t>(round(n * percent));

    XType train_X, cv_X;
    YType train_Y, cv_Y;

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
