#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <tuple>
#include <random>
#include <algorithm>
#include "mlscrcap.h"
#include "mlimage.h"
#include "mltimer.h"
#include "mlinput.h"
using namespace std;

tuple<vector<vector<float>>, vector<int>>
load_data(const string& data_path, const string& data_label_path);

template <typename Data_Con, typename Data_Label_Con>
tuple<Data_Con, Data_Label_Con, Data_Con, Data_Label_Con>
cv_split(Data_Con&& data, Data_Label_Con&& labels, double percent);

int main(int argc, char *argv[])
{

    auto&& [data, label] = load_data(argv[1], argv[2]);

    


}

tuple<vector<vector<float>>, vector<int>>
load_data(const string& data_path, const string& data_label_path)
{
    ifstream data_file(data_path);
    ifstream data_label_file(data_label_path);
    string line;
    float curr, prev;
    vector<vector<float>> data;
    vector<int> labels;

    while (getline(data_path, line)) {
        istream_iterator<float> data_itr(istringstream(line), ",");
        vector<float> data_line;

        copy(data_itr, istream_iterator<float>(), back_inserter(data_line));
        data.push_back(std::move(data_line));
    }

    istream_iterator<int> data_label_itr(data_label_file, "\n");
    copy(data_label_itr, istream_iterator<int>(), back_inserter(labels));

    return make_tuple(data, labels);
}

template <typename Data_Con, typename Data_Label_Con>
tuple<Data_Con, Data_Label_Con, Data_Con, Data_Label_Con>
cv_split(Data_Con&& data, Data_Label_Con&& labels, double percent) {
    random_device rd;
    mt19937_64 engine(rd());
    auto n = data.size();
    uniform_int_distribution<size_t> random(0, n - 1);

    for (size_t i = n - 1; i > 0; --i) {
        using std::swap;
        auto j = random(engine);
        swap(data[i], data[j]);
        swap(labels[i], labels[j]);
    }

    size_t cv_size = static_cast<size_t>(round(n * percent));

    Data_Con train_data, cv_data;
    Data_Label_Con train_labels, cv_labels;

    for (size_t i = 0; i != cv_size; ++i) {
        cv_data.push_back(std::move(data[i]));
        cv_labels.push_back(std::move(labels[i]));
    }

    for (size_t i = cv_size; i != n; ++i) {
        train_data.push_back(std::move(data[i]));
        train_labels.push_back(std::move(labels[i]));
    }

    return make_tuple(train_data, train_labels, cv_data, cv_labels);
}
