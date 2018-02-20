#include "mlnet.h"
#include <caffe2/core/workspace.h>
#include <caffe2/core/operator_gradient.h>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {
    caffe2::OperatorDef* add_op(caffe2::NetDef& net,
                                const std::string& name,
                                const std::vector<std::string>& blobs_in,
                                const std::vector<std::string>& blobs_out);

    caffe2::OperatorDef* add_input(caffe2::NetDef& net, const std::string& reader);

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name);

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 const std::string& value);

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 int value);

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 float value);
    
    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name, 
                                 const std::vector<MlNet::TIndex>& values);

    template <typename VType = int>
    caffe2::OperatorDef* add_Fill_op(caffe2::NetDef& net,
                                     MlNet::fill_type type, 
                                     const std::vector<MlNet::TIndex>&,
                                     const std::string&,
                                     VType value = 0);
    caffe2::OperatorDef* add_gradient_op(caffe2::NetDef& net, caffe2::OperatorDef& op);

    caffe2::OperatorDef* add_gradient_ops(caffe2::NetDef& net, 
                                          caffe2::OperatorDef& op, 
                                          std::unordered_map<std::string, 
                                                             std::pair<int,
                                                                       int>>& split_inputs,
                                          std::unordered_map<std::string, 
                                                             std::string>& pass_replace, 
                                          std::unordered_set<std::string>& stop_inputs)
    {
        static auto op_has_output = [](const caffe2::OperatorDef& op, auto& set_names)
        {
            for (const auto& output : op.output()) {
                if (set_names.find(output) != set_names.end()) {
                    return true;
                }
            }
            return false;
        };

        caffe2::OperatorDef* grad = net.add_op();
        if (op.type() == "StopGradient" || op_has_output(op, stop_inputs)) {
            for (const auto& input : op.input()) {
                stop_inputs.insert(input);
            }
        } else {
            grad = add_gradient_op(net, op);
            if (!grad)
                std::cerr << "No gradient for operator " << op.type() << std::endl;

        }
        if (grad) {
            grad->set_is_gradient_op(true);
            for (auto i = 0; i != grad->output_size(); ++i) {
                auto output = grad->output(i);
                if (split_inputs.count(output) && split_inputs[output].first > 0) {
                    grad->set_output(i, output + 
                                        "_sum_" + 
                                        std::to_string(split_inputs[output].first));
                }
            }
            for (auto i = 0; i != grad->input_size(); ++i) {
                auto input = grad->input(i);
                if (pass_replace.count(input)) {
                    grad->set_input(i, pass_replace[input]);
                    pass_replace.erase(input);
                }
            }
        }

            // merge split gradient with sum
        for (auto& p : split_inputs) {
            if (p.second.first == 0) {
                std::vector<std::string> inputs;
                for (auto i = 0; i != p.second.second; ++i) {
                    auto input = p.first + "_sum_" + std::to_string(i);
                    if (pass_replace.count(input)) {
                        auto in = pass_replace[input];
                        pass_replace.erase(input);
                        input = in;
                    }
                    inputs.push_back(input);
                }
                add_op(net, "Sum", inputs, {p.first});
                --p.second.first;
            }
        }
        return grad;
    }

    const std::unordered_set<std::string> trainable_ops({
        "Add",
        "AffineScale",
        "AveragedLoss",
        "AveragePool",
        "BackMean",
        "Concat",
        "Conv",
        "Diagonal",
        "Dropout",
        "EnsureCPUOutput",
        "FC",
        "LabelCrossEntropy",
        "LRN",
        "MaxPool",
        "Mul",
        "RecurrentNetwork",
        "Relu",
        "Reshape",
        "Slice",
        "Softmax",
        "SpatialBN",
        "SquaredL2",
        "SquaredL2Channel",
        "StopGradient",
        "Sum"
    });

    const std::unordered_set<std::string> non_trainable_ops({
        "Accuracy",
        "Cast",
        "Cout",
        "ConstantFill",
        "Iter",
        "Scale",
        "TensorProtosDBInput",
        "TimePlot",
        "ShowWorst"
    });

}

MlNet::MlNet(const std::string& net_name)
{
    param_init_net.set_name(net_name + "_init");
    net.set_name(net_name);
}

void MlNet::add_database_input(const std::string& data,
                               const std::string& label,
                               const std::string& db_path,
                               const std::string& db_type,
                               int batch_size)
{
    auto reader = data + "_reader";
    auto *op = add_op(param_init_net, "CreateDB", {}, {reader});
    add_arg(op, "db_type", db_type);
    add_arg(op, "db", db_path);
    add_input(net, reader);
    add_TensorProtos_db_input_op(reader, data, label, batch_size);
    add_op(net, "StopGradient", {data}, {data});
}

void MlNet::add_FC_op(const std::string& blob_in, 
                      const std::string& blob_out, 
                      TIndex dim_in,
                      TIndex dim_out,
                      fill_type type)
{
    auto w = blob_out + "_w";
    auto b = blob_out + "_b";
    add_Fill_op(param_init_net, type, {dim_out, dim_in}, w);
    add_Fill_op(param_init_net, type, {dim_out}, b);
    add_op(net, "FC", {blob_in, w, b}, {blob_out});
}

void MlNet::add_ReLU_op(const std::string& blob_in, const std::string& blob_out)
{
    add_op(net, "Relu", {blob_in}, {blob_out});
}

void MlNet::add_Softmax_op(const std::string& blob_in, const std::string& blob_out)
{
    add_op(net, "Softmax", {blob_in}, {blob_out});
}
void MlNet::add_training_op(const std::string& pred,
                            const std::string& label, 
                            const std::string& xent)
{
    add_op(net, "LabelCrossEntropy", {pred, label}, {xent});
    add_op(net, "AveragedLoss", {xent}, {"loss"});
    add_op(net, "Accuracy", {pred, label}, {"accuracy"});

    add_gradient_op();
}

void MlNet::add_LR_op(const std::string& rate, float alpha, float gamma)
{
    auto *op_itr = add_op(param_init_net, "ConstantFill", {}, {"iter"});
    add_arg(op_itr, "shape", {1});
    add_arg(op_itr, "value", 0);
    add_arg(op_itr, "dtype", caffe2::TensorProto_DataType_INT64);
    op_itr->mutable_device_option()->set_device_type(caffe2::CPU);

    net.add_external_input("iter");
    add_op(net, "Iter", {"iter"}, {"iter"});

    auto *op_lrn = add_op(net, "LearningRate", {"iter"}, {rate});
    add_arg(op_lrn, "policy", "step");
    add_arg(op_lrn, "stepsize", 1);
    add_arg(op_lrn, "base_lr", -alpha);
    add_arg(op_lrn, "gamma", gamma);

    add_Fill_op(param_init_net, fill_type::Constant, {1}, "ONE", 1.f);
    add_input(net, "ONE");

    std::vector<std::string> params;

    std::unordered_set<std::string> external_inputs(net.external_input().begin(),
                                                    net.external_input().end());
    for (const auto& op : net.op()) {
        auto& output = op.output();
        if (trainable_ops.find(op.type()) != trainable_ops.end()) {
            for (const auto& input : op.input()) {
                if (external_inputs.find(input) != external_inputs.end()) {
                    if (std::find(output.begin(), output.end(), input) == output.end()) {
                        params.push_back(input);
                    }
                }
            }
        }
    }
    
    for (auto& param : params) {
        add_weighted_sum_op(net, {param, "ONE", param + "_grad", "LR"}, param);
    }

}

void MlNet::add_stop_gradient_op(const std::string& blob)
{
    add_op(net, "StopGradient", {blob}, {blob});
}

void MlNet::add_gradient_op()
{
    std::unordered_map<std::string, std::string> pass_replace;
    std::unordered_set<std::string> stop_inputs;

    auto&& [ops, split_inputs] = collect_gradient_ops();
    for (auto& op : ops) {
        add_gradient_ops(net, op, split_inputs, pass_replace, stop_inputs);
    }
}

std::tuple<std::vector<caffe2::OperatorDef>,
           std::unordered_map<std::string, std::pair<int, int>>>
MlNet::collect_gradient_ops()
{
    std::unordered_map<std::string, std::pair<int, int>> split_inputs;
    std::unordered_set<std::string> external_inputs(net.external_input().begin(),
                                                    net.external_input().end());

    std::vector<caffe2::OperatorDef> gradient_ops;
    std::unordered_map<std::string, int> input_count;
    for (auto& op : net.op()) {
        if (trainable_ops.find(op.type()) != trainable_ops.end()) {
            gradient_ops.push_back(op);
            for (auto& input : op.input()) {
                auto& output = op.output();
                if (std::find(output.begin(), output.end(), input) == output.end()) {
                    input_count[input]++;
                    if (input_count[input] > 1) {
                        split_inputs[input] = {input_count[input], input_count[input]};
                    }
                }
            }
        } else if (non_trainable_ops.find(op.type()) == non_trainable_ops.end()) {
            CAFFE_THROW("unknown backprop operator type: " + op.type());
        }
    }
    std::reverse(gradient_ops.begin(), gradient_ops.end());
    return std::make_tuple(gradient_ops, split_inputs);
}

void MlNet::add_weighted_sum_op(caffe2::NetDef& target,
                                const std::vector<std::string>& inputs,
                                const std::string& sum)
{
    add_op(target, "WeightedSum", inputs, {sum});
}

void MlNet::add_TensorProtos_db_input_op(const std::string& reader, 
                                  const std::string& data, 
                                  const std::string& label, 
                                  int batch_size)
{
    auto *op = add_op(net, "TensorProtosDBInput", {reader}, {data, label});
    add_arg(op, "batch_size", batch_size);
}

namespace {
    caffe2::OperatorDef* add_op(caffe2::NetDef& net,
                                const std::string& name,
                                const std::vector<std::string>& blobs_in,
                                const std::vector<std::string>& blobs_out)
    {
        auto *op = net.add_op();
        op->set_type(name);
    
        for (auto& blob_in : blobs_in)
            op->add_input(blob_in);
        for (auto& blob_out : blobs_out)
            op->add_output(blob_out);
    
        return op;
    }

    caffe2::OperatorDef* add_input(caffe2::NetDef& net, const std::string& input)
    {
        net.add_external_input(input);
    }

    caffe2::Argument* add_arg(caffe2::OperatorDef* op,
                                 const std::string& name)
    {
        auto *arg = op->add_arg();
        arg->set_name(name);

        return arg;
    }

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 const std::string& value)
    {
        auto *arg = add_arg(op, name);
        arg->set_s(value);

        return arg;
    }

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 int value)
    {
        auto *arg = add_arg(op, name);
        arg->set_i(value);

        return arg;
    }

    caffe2::Argument* add_arg(caffe2::OperatorDef* op, 
                                 const std::string& name,
                                 float value)
    {
        auto *arg = add_arg(op, name);
        arg->set_f(value);

        return arg;
    }

    caffe2::Argument* add_arg(caffe2::OperatorDef* op,
                                 const std::string& name,
                                 const std::vector<MlNet::TIndex>& values)
    {
        auto *arg = add_arg(op, name);
        for (auto value : values)
            arg->add_ints(value);

        return arg;
    }

    template <typename VType = int>
    caffe2::OperatorDef* add_Fill_op(caffe2::NetDef& net,
                                     MlNet::fill_type type, 
                                     const std::vector<MlNet::TIndex>& dims,
                                     const std::string& blob,
                                     VType value)
    {
        std::string fill_name;
        switch (type) {
        case MlNet::fill_type::Xavier:
            fill_name = "XavierFill";
            break;
        case MlNet::fill_type::MSRA:
            fill_name = "MSRAFill";
            break;
        case MlNet::fill_type::Constant:
            fill_name = "ConstantFill";
        }
        auto *op_w = add_op(net, fill_name, {}, {blob});
        add_arg(op_w, "shape", dims);
        if (type == MlNet::fill_type::Constant) {
            add_arg(op_w, "value", value);
        }

        return op_w;
    }


    caffe2::OperatorDef* add_gradient_op(caffe2::NetDef& net, caffe2::OperatorDef& op)
    {
        caffe2::OperatorDef* grad = nullptr;
        std::vector<caffe2::GradientWrapper> output(op.output_size());
        for (auto i = 0; i != output.size(); ++i) {
            output[i].dense_ = op.output(i) + "_grad";
        }
        caffe2::GradientOpsMeta meta = caffe2::GetGradientForOp(op, output);
        if (meta.ops_.size()) {
            for (auto& m : meta.ops_) {
                auto op = net.add_op();
                op->CopyFrom(m);
                if (!grad)
                    grad = op;
            }
        }
        return grad;
    }


}
