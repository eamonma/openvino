// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include "base_reference_test.hpp"
#include "openvino/op/bitwise_not.hpp"

using namespace ov;

namespace reference_tests {
namespace BitwiseOpsRefTestDefinitions {

enum BitwiseTypes { BITWISE_NOT };

struct RefBitwiseParams {
    BitwiseTypes opType;
    std::vector<reference_tests::Tensor> inputs;
    reference_tests::Tensor expected;
};

struct Builder : ParamsBuilder<RefBitwiseParams> {
    REFERENCE_TESTS_ADD_SET_PARAM(Builder, opType);
    REFERENCE_TESTS_ADD_SET_PARAM(Builder, inputs);
    REFERENCE_TESTS_ADD_SET_PARAM(Builder, expected);
};

class ReferenceBitwiseLayerTest : public testing::TestWithParam<RefBitwiseParams>, public CommonReferenceTest {
public:
    void SetUp() override {
        const auto& params = GetParam();
        function = CreateFunction(params.opType, params.inputs);
        for (auto& input : params.inputs) {
            inputData.push_back(input.data);
        }
        refOutData = {params.expected.data};
    }
    static std::string getTestCaseName(const testing::TestParamInfo<RefBitwiseParams>& obj) {
        const auto& param = obj.param;
        std::ostringstream result;
        result << "BitwiseType=" << param.opType << "_";
        for (size_t i = 0; i < param.inputs.size(); i++) {
            const auto input = param.inputs[i];
            result << "inpt_shape" << i << "=" << input.shape << "_";
            result << "inpt_type" << i << "=" << input.type << "_";
        }
        result << "oType=" << param.expected.type;
        return result.str();
    }

private:
    static std::shared_ptr<ov::Model> CreateFunction(BitwiseTypes op_type,
                                                     const std::vector<reference_tests::Tensor>& inputs) {
        ov::ParameterVector params_vec;
        for (auto& input : inputs) {
            params_vec.push_back(std::make_shared<op::v0::Parameter>(input.type, input.shape));
        }

        std::shared_ptr<ov::Node> bitwise_op;
        switch (op_type) {
        case BitwiseTypes::BITWISE_NOT: {
            bitwise_op = std::make_shared<ov::op::v13::BitwiseNot>(params_vec[0]);
            break;
        }
        default: {
            throw std::runtime_error("Incorrect type of Bitwise operation");
        }
        }
        return std::make_shared<ov::Model>(ov::NodeVector{bitwise_op}, ov::ParameterVector{params_vec});
    }
};
}  // namespace BitwiseOpsRefTestDefinitions
}  // namespace reference_tests
