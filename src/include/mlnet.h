#ifndef MLNET_H
#define MLNET_H
#include <caffe2/core/workspace.h>
#include <vector>

class MlNet {
public:
    typedef caffe2::TIndex TIndex;
    enum class fill_type { Xavier, MSRA, Constant };
    MlNet() = default;
    MlNet(const std::string&);

    void add_database_input(const std::string& data,
                            const std::string& label,
                            const std::string& db_path,
                            const std::string& db_type,
                            int batch_size);

    
    template <typename TensorType, typename XValueType>
    TensorType* add_blob_input(const std::string&, 
                          const std::vector<std::vector<XValueType>>&,
                          int,
                          int);
    void add_FC_op(const std::string& blob_in, 
                   const std::string& blob_out,
                   TIndex dim_in, 
                   TIndex dim_out, 
                   fill_type type);
    void add_ReLU_op(const std::string& blob_in, const std::string& blob_out);
    void add_Softmax_op(const std::string& blob_in, const std::string& blob_out);
    void add_training_op(const std::string& pred,
                         const std::string& label,
                         const std::string& xent);
    void add_LR_op(const std::string& rate, float alpha, float gamma);
    void add_stop_gradient_op(const std::string& blob);

private:
    void add_gradient_op();
    std::tuple<std::vector<caffe2::OperatorDef>,
               std::unordered_map<std::string, std::pair<int, int>>>
    collect_gradient_ops();
    void add_weighted_sum_op(caffe2::NetDef& target,
                             const std::vector<std::string>& inputs,
                             const std::string& sum);
    void add_TensorProtos_db_input_op(const std::string& reader, 
                                      const std::string& name, 
                                      const std::string& label, 
                                      int batch_size);

    caffe2::Workspace workspace;
    caffe2::NetDef param_init_net, net;
};

template <typename TensorType, typename XValueType>
TensorType* MlNet::add_blob_input(const std::string& name,
                      const std::vector<std::vector<XValueType>>& X, 
                      int height, 
                      int width)
{
    std::vector<XValueType> reshaped;
    reshaped.reserve(X.size() * X[0].size());
    
    for (const auto& line : X) {
        std::memcpy(reshaped.data(), line.data(), line.size());
    }

    auto data = workspace.CreateBlob(name)->GetMutable<TensorType>();

    int n = X.size();
    int c = 1;
    int h = height;
    int w = width;
    caffe2::TensorCPU input({n, c, h, w}, reshaped, nullptr);

    data->CopyFrom(input);
    return data;
}

#endif // MLNET_H
