#include <iostream>
#include <stdexcept>
#include <caffe2/core/init.h>
#include <caffe2/core/blob.h>
#include <caffe2/core/workspace.h>
#include <caffe2/core/db.h>
#include "mldata.h"
#include "mlnet.h"
#include <vector>
#include <string>
using namespace std;

CAFFE2_DEFINE_string(x_path, "", "file path of dataset.");
CAFFE2_DEFINE_string(y_path, "", "file path of dataset label.");

void parse_arg(int*, char **argv[]);

template <std::size_t roi_h_n = 21, std::size_t roi_w_n = 12, typename XType, typename YType>
void create_db(const string& db_type, const string& db_name, XType&& X, YType&& Y);

shared_ptr<MlNet> create_mlp(const std::string& net_name,
                             const std::string& x, 
                             const std::string& y, 
                             const std::string& db_path,
                             const std::string& db_type, 
                             int batch_size);

int main(int argc, char *argv[])
{
    parse_arg(&argc, &argv);

    
    auto&& [features, labels] = load_data(FLAGS_x_path, FLAGS_y_path);

    auto&& [X, Y] = balance_dataset(std::move(features), std::move(labels));

    auto&& [train_features, train_labels, test_features, test_labels] = cv_split(X, Y, 0.4);

    create_db("minidb", "endless_lake_train.minidb", train_features, train_labels);
    create_db("minidb", "endless_lake_test.minidb", test_features, test_labels);


    // model description:
    // width: 12 boxes; height: 21 boxes

    // so layer 1 :  3x3 kernel
    auto net = create_mlp("mlp", "data", "action", ".", "minidb", 300);
    

    return 0;
}

void parse_arg(int* argcp, char **argvp[])
{
    caffe2::GlobalInit(argcp, argvp);
    if (!ifstream(FLAGS_x_path)) {
        cerr << "path of dataset not provided." << endl;
        exit(1);
    }
    if (!ifstream(FLAGS_y_path)) {
        cerr << "path of dataset label not provided." << endl;
        exit(1);
    }

}

template <std::size_t roi_h_n, std::size_t roi_w_n, typename XType, typename YType>
void create_db(const string& db_type, const string& db_name, XType&& X, YType&& Y)
{
    auto db = caffe2::db::CreateDB(db_type, db_name, caffe2::db::WRITE);
    auto transaction = db->NewTransaction();
    for (int i = 0; i != X.size(); ++i) {
        caffe2::TensorProtos features_label;
        auto feature_proto = features_label.add_protos();
        feature_proto->add_dims(roi_h_n);
        feature_proto->add_dims(roi_w_n);
        for (float feature : X[i])
            feature_proto->add_float_data(feature);
        auto label_proto = features_label.add_protos();
        label_proto->add_dims(1);
        label_proto->add_float_data(Y[i]);
        std::string features_label_str;
        if (!features_label.SerializeToString(&features_label_str))
            throw std::runtime_error("features and label proto serialization failed.");
        transaction->Put(std::to_string(i), features_label_str);
    }
    
}

shared_ptr<MlNet> create_mlp(const std::string& net_name,
                 const std::string& x,
                 const std::string& y, 
                 const std::string& db_path,
                 const std::string& db_type, 
                 int batch_size)
{
    auto net = make_shared<MlNet>(net_name);
    net->add_database_input(x, y, db_path, db_type, batch_size);
    net->add_FC_op(x, "fc1", 21 * 12, 50, MlNet::fill_type::MSRA);
    net->add_ReLU_op("fc1", "fc1_act");
    net->add_FC_op("fc1_act", "fc2", 50, 10, MlNet::fill_type::MSRA);
    net->add_ReLU_op("fc2", "fc2_act");
    net->add_Softmax_op("fc2_act", "pred");
    net->add_stop_gradient_op(x);

    return net;
}
