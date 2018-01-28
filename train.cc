#include <iostream>
#include <stdexcept>
#include <caffe2/core/init.h>
#include <caffe2/core/blob.h>
#include <caffe2/core/workspace.h>
#include <gflags/gflags.h>
#include "mldata.h"
using namespace std;

DEFINE_string(dataset_path, "", "file path of dataset.");
DEFINE_string(dataset_label_path, "", "file path of dataset label.");

void parse_arg(int*, char **argv[]);

int main(int argc, char *argv[])
{
    parse_arg(&argc, &argv);

    
    auto&& [X, Y] = load_data(argv[1], argv[2]);

    for (const auto& x_l : X) {
        for (const auto& x : x_l)
            cout << x << ' ';
        cout << endl;
    }
    cout << endl;

    for (const auto& y : Y)
        cout << y << ' ';
    cout << endl;
    auto&& [train_data, train_labels, cv_data, cv_labels] = cv_split(X, Y, 0.4);
    /*
    auto XY = load_data(argv[1], argv[2]);
    auto Z = cv_split(get<0>(XY), get<1>(XY), 0.4);
    */



    caffe2::Workspace workspace;

    cout << workspace.RootFolder() << endl;
    
    return 0;
}

void parse_arg(int* argcp, char **argvp[])
{
    gflags::ParseCommandLineFlags(argcp, argvp, true);
    if (!ifstream(FLAGS_dataset_path)) {
        cerr << "path of dataset not provided." << endl;
        exit(1);
    }
    if (!ifstream(FLAGS_dataset_label_path)) {
        cerr << "path of dataset label not provided." << endl;
        exit(1);
    }

}

