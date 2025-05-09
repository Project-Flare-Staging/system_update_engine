//
// Copyright (C) 2018 The Android Open Source Project
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#ifndef UPDATE_ENGINE_PAYLOAD_CONSUMER_VERITY_WRITER_INTERFACE_H_
#define UPDATE_ENGINE_PAYLOAD_CONSUMER_VERITY_WRITER_INTERFACE_H_

#include <cstdint>
#include <memory>

#include <android-base/macros.h>

#include "common/utils.h"
#include "payload_consumer/file_descriptor.h"
#include "update_engine/payload_consumer/install_plan.h"

namespace chromeos_update_engine {

class VerityWriterInterface {
 public:
  virtual ~VerityWriterInterface() = default;

  virtual bool Init(const InstallPlan::Partition& partition) = 0;
  // Update partition data at [offset : offset + size) stored in |buffer|.
  // Data not in |hash_tree_data_extent| or |fec_data_extent| is ignored.
  // Will write verity data to the target partition once all the necessary
  // blocks has passed.
  virtual bool Update(uint64_t offset, const uint8_t* buffer, size_t size) = 0;

  // Deprecated function -> use IncrementalFinalize to allow verity writes to be
  // interrupted. left for backwards compatibility
  virtual bool Finalize(FileDescriptor* read_fd, FileDescriptor* write_fd) {
    while (!FECFinished()) {
      TEST_AND_RETURN_FALSE(IncrementalFinalize(read_fd, write_fd));
    }
    return true;
  }

  // Write hash tree && FEC data to underlying fd, if they are present
  virtual bool IncrementalFinalize(FileDescriptor* read_fd,
                                   FileDescriptor* write_fd) = 0;

  // Returns true once FEC data is finished writing
  virtual bool FECFinished() const = 0;

  // Gets progress report on FEC write
  virtual double GetProgress() = 0;

 protected:
  VerityWriterInterface() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(VerityWriterInterface);
};

namespace verity_writer {
std::unique_ptr<VerityWriterInterface> CreateVerityWriter();
}

}  // namespace chromeos_update_engine

#endif  // UPDATE_ENGINE_PAYLOAD_CONSUMER_VERITY_WRITER_INTERFACE_H_
