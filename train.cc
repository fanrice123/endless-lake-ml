#include <iostream>
#include <caffe2/core/init.h>
#include <caffe2/core/blob.h>
#include <caffe2/core/workspace.h>
#include "mldata.h"
using namespace std;

int main(int argc, char *argv[])
{
    cout << argv[1] << endl;
    cout << argv[2] << endl;
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
