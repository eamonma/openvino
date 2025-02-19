// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "openvino/runtime/isync_infer_request.hpp"
#include "intel_gpu/plugin/graph.hpp"
#include "intel_gpu/plugin/remote_tensor.hpp"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <atomic>

namespace ov {
namespace intel_gpu {

class CompiledModel;

enum class TensorOwner : uint8_t {
    USER = 0,
    PLUGIN = 1
};

struct TensorWrapper {
    std::shared_ptr<ov::ITensor> ptr;
    TensorOwner owner;
};

class SyncInferRequest : public ov::ISyncInferRequest {
public:
    using Ptr = std::shared_ptr<SyncInferRequest>;

    explicit SyncInferRequest(const std::shared_ptr<const CompiledModel>& compiled_model);
    SyncInferRequest(const SyncInferRequest &) = delete;
    ~SyncInferRequest() override = default;

    void infer() override;
    std::vector<ov::ProfilingInfo> get_profiling_info() const override;
    std::vector<ov::SoPtr<ov::IVariableState>> query_state() const override;

    void set_tensor(const ov::Output<const ov::Node>& port, const ov::SoPtr<ov::ITensor>& tensor) override;
    void set_tensors_impl(const ov::Output<const ov::Node> port, const std::vector<ov::SoPtr<ov::ITensor>>& tensors) override;

    ov::SoPtr<ov::ITensor> get_tensor(const ov::Output<const ov::Node>& port) const override;

    void set_task_executor(const std::shared_ptr<ov::threading::ITaskExecutor>& task_executor);
    void setup_stream_graph();
    void enqueue_notify();
    void wait_notify();

    void enqueue();
    void wait();

    bool use_external_queue() const { return m_use_external_queue; }

private:
    void check_tensors() const override;

    std::unordered_map<std::string, TensorWrapper> m_user_inputs;
    std::unordered_map<std::string, TensorWrapper> m_user_outputs;

    std::unordered_map<std::string, TensorWrapper> m_plugin_inputs;
    std::unordered_map<std::string, TensorWrapper> m_plugin_outputs;

    std::unordered_map<std::string, ov::Output<const ov::Node>> m_input_ports_map;
    std::unordered_map<std::string, ov::Output<const ov::Node>> m_output_ports_map;
    std::unordered_map<std::string, std::string> m_output_names_map;

    std::map<cldnn::primitive_id, cldnn::network_output> m_internal_outputs;

    std::shared_ptr<Graph> m_graph;
    RemoteContextImpl::Ptr m_context = nullptr;
    std::shared_ptr<ov::threading::IStreamsExecutor> m_stream_executor = nullptr;
    bool m_enable_profiling = false;
    bool m_use_external_queue = false;

    std::vector<cldnn::event::ptr> prepare_input(const std::string& name, const ov::Output<const ov::Node>& port, const TensorWrapper& user_tensor_wrapper);
    std::vector<cldnn::event::ptr> prepare_output(const std::string& name, const ov::Output<const ov::Node>& port, const TensorWrapper& user_tensor_wrapper);
    std::vector<cldnn::event::ptr> prepare_batched_input(const std::string& name,
                                                         const ov::Output<const ov::Node>& port,
                                                         const std::vector<ov::SoPtr<ov::ITensor>>& user_tensors);

    TensorWrapper create_or_share_device_tensor(const TensorWrapper& user_tensor_wrapper,
                                                const std::string& name,
                                                const ov::PartialShape& pshape,
                                                ov::element::Type element_type,
                                                bool need_lockable_mem) const;
    std::shared_ptr<ov::ITensor> reinterpret_device_tensor(std::shared_ptr<RemoteTensorImpl> tensor, const ov::Shape new_shape) const;
    std::shared_ptr<ov::ITensor> create_host_tensor(const ov::PartialShape& port_shape, const ov::element::Type& port_element_type) const;
    std::shared_ptr<ov::ITensor> create_device_tensor(const ov::Shape& pshape, ov::element::Type element_type,
                                                      bool need_lockable_memory = false, void* mem_ptr = nullptr) const;
    std::shared_ptr<ov::ITensor> create_shared_device_tensor(const ov::Shape& pshape, ov::element::Type element_type, void* usm_host_mem) const;

    void allocate_inputs();
    void allocate_outputs();
    void allocate_states();
    void allocate_input(const ov::Output<const ov::Node>& port, const std::string& name);
    void allocate_output(const ov::Output<const ov::Node>& port, const std::string& name);
    cldnn::event::ptr copy_output_data(cldnn::memory::ptr src, const ov::ITensor& dst) const;

    void init_mappings(bool is_legacy_api);
    bool is_batched_input(const ov::Output<const ov::Node>& port) const;
};

}  // namespace intel_gpu
}  // namespace ov
