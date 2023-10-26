/*
 * Copyright (c) 2019-2021, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <rmm/mr/device/device_memory_resource.hpp>

#include <rmm/cuda_stream_view.hpp>
#include <rmm/detail/error.hpp>

#include <cstddef>

namespace rmm::mr {
/**
 * @addtogroup device_memory_resources
 * @{
 * @file
 */
/**
 * @brief `device_memory_resource` derived class that uses cudaMalloc/Free for
 * allocation/deallocation.
 */
class cuda_memory_resource final : public device_memory_resource {
 public:
  cuda_memory_resource()                            = default;
  ~cuda_memory_resource() override                  = default;
  cuda_memory_resource(cuda_memory_resource const&) = default;  ///< @default_copy_constructor
  cuda_memory_resource(cuda_memory_resource&&)      = default;  ///< @default_move_constructor
  cuda_memory_resource& operator=(cuda_memory_resource const&) =
    default;  ///< @default_copy_assignment{cuda_memory_resource}
  cuda_memory_resource& operator=(cuda_memory_resource&&) =
    default;  ///< @default_move_assignment{cuda_memory_resource}

  /**
   * @brief Query whether the resource supports use of non-null CUDA streams for
   * allocation/deallocation. `cuda_memory_resource` does not support streams.
   *
   * @returns bool false
   */
  [[nodiscard]] bool supports_streams() const noexcept override { return false; }

  /**
   * @brief Query whether the resource supports the get_mem_info API.
   *
   * @return true
   */
  [[nodiscard]] bool supports_get_mem_info() const noexcept override { return true; }

 private:
  /**
   * @brief Allocates memory of size at least \p bytes.
   *
   * The returned pointer will have at minimum 256 byte alignment.
   *
   * The stream argument is ignored.
   *
   * @param bytes The size of the allocation
   * @param stream This argument is ignored
   * @return void* Pointer to the newly allocated memory
   */
  void* do_allocate(std::size_t bytes, [[maybe_unused]] cuda_stream_view stream) override
  {
    void* ptr{nullptr};
    RMM_CUDA_TRY_ALLOC(cudaMalloc(&ptr, bytes));
    return ptr;
  }

  /**
   * @brief Deallocate memory pointed to by \p p.
   *
   * The stream argument is ignored.
   *
   * @param ptr Pointer to be deallocated
   * @param bytes The size in bytes of the allocation. This must be equal to the
   * value of `bytes` that was passed to the `allocate` call that returned `p`.
   * @param stream This argument is ignored.
   */
  void do_deallocate(void* ptr,
                     [[maybe_unused]] std::size_t bytes,
                     [[maybe_unused]] cuda_stream_view stream) override
  {
    RMM_ASSERT_CUDA_SUCCESS(cudaFree(ptr));
  }

  /**
   * @brief Compare this resource to another.
   *
   * Two cuda_memory_resources always compare equal, because they can each
   * deallocate memory allocated by the other.
   *
   * @param other The other resource to compare to
   * @return true If the two resources are equivalent
   * @return false If the two resources are not equal
   */
  [[nodiscard]] bool do_is_equal(device_memory_resource const& other) const noexcept override
  {
    return dynamic_cast<cuda_memory_resource const*>(&other) != nullptr;
  }

  /**
   * @brief Get free and available memory for memory resource
   *
   * @throws rmm::cuda_error if unable to retrieve memory info.
   *
   * @return std::pair contaiing free_size and total_size of memory
   */
  [[nodiscard]] std::pair<std::size_t, std::size_t> do_get_mem_info(cuda_stream_view) const override
  {
    std::size_t free_size{};
    std::size_t total_size{};
    RMM_CUDA_TRY(cudaMemGetInfo(&free_size, &total_size));
    return std::make_pair(free_size, total_size);
  }
};
/** @} */  // end of group
}  // namespace rmm::mr
