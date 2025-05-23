//
// Copyright (C) 2013 The Android Open Source Project
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

#ifndef UPDATE_ENGINE_COMMON_FAKE_HARDWARE_H_
#define UPDATE_ENGINE_COMMON_FAKE_HARDWARE_H_

#include <map>
#include <string>
#include <utility>

#include <base/time/time.h>

#include "update_engine/common/error_code.h"
#include "update_engine/common/hardware_interface.h"
#include "update_engine/common/utils.h"

namespace chromeos_update_engine {

// Implements a fake hardware interface used for testing.
class FakeHardware : public HardwareInterface {
 public:
  // Value used to signal that the powerwash_count file is not present. When
  // this value is used in SetPowerwashCount(), GetPowerwashCount() will return
  // false.
  static const int kPowerwashCountNotSet = -1;

  // Default value for crossystem tpm_kernver.
  static const int kMinKernelKeyVersion = 3;

  // Default value for crossystem tpm_fwver.
  static const int kMinFirmwareKeyVersion = 13;

  // Default value for crossystem kernel_max_rollforward. This value is the
  // default for consumer devices and effectively means "unlimited rollforward
  // is allowed", which is the same as the behavior prior to implementing
  // roll forward prevention.
  static const int kKernelMaxRollforward = 0xfffffffe;

  // Default value for crossystem firmware_max_rollforward. This value is the
  // default for consumer devices and effectively means "unlimited rollforward
  // is allowed", which is the same as the behavior prior to implementing
  // roll forward prevention.
  static const int kFirmwareMaxRollforward = 0xfffffffe;

  FakeHardware() = default;

  // HardwareInterface methods.
  bool IsOfficialBuild() const override { return is_official_build_; }

  bool IsNormalBootMode() const override { return is_normal_boot_mode_; }

  bool AreDevFeaturesEnabled() const override {
    return are_dev_features_enabled_;
  }

  bool IsOOBEEnabled() const override { return is_oobe_enabled_; }

  bool IsOOBEComplete(base::Time* out_time_of_oobe) const override {
    if (out_time_of_oobe != nullptr)
      *out_time_of_oobe = oobe_timestamp_;
    return is_oobe_complete_;
  }

  std::string GetHardwareClass() const override { return hardware_class_; }

  std::string GetDeviceRequisition() const override {
    return device_requisition_;
  }

  int GetMinKernelKeyVersion() const override {
    return min_kernel_key_version_;
  }

  int GetMinFirmwareKeyVersion() const override {
    return min_firmware_key_version_;
  }

  int GetMaxFirmwareKeyRollforward() const override {
    return firmware_max_rollforward_;
  }

  bool SetMaxFirmwareKeyRollforward(int firmware_max_rollforward) override {
    if (GetMaxFirmwareKeyRollforward() == -1)
      return false;

    firmware_max_rollforward_ = firmware_max_rollforward;
    return true;
  }

  bool SetMaxKernelKeyRollforward(int kernel_max_rollforward) override {
    kernel_max_rollforward_ = kernel_max_rollforward;
    return true;
  }

  int GetPowerwashCount() const override { return powerwash_count_; }

  bool SchedulePowerwash() override {
    powerwash_scheduled_ = true;
    return true;
  }

  bool CancelPowerwash() override {
    powerwash_scheduled_ = false;
    return true;
  }

  bool IsPowerwashScheduled() { return powerwash_scheduled_; }

  bool GetNonVolatileDirectory(base::FilePath* path) const override {
    return false;
  }

  bool GetPowerwashSafeDirectory(base::FilePath* path) const override {
    return false;
  }

  int64_t GetBuildTimestamp() const override { return build_timestamp_; }

  bool AllowDowngrade() const override { return false; }

  bool GetFirstActiveOmahaPingSent() const override {
    return first_active_omaha_ping_sent_;
  }

  bool SetFirstActiveOmahaPingSent() override {
    first_active_omaha_ping_sent_ = true;
    return true;
  }

  // Setters
  void SetIsOfficialBuild(bool is_official_build) {
    is_official_build_ = is_official_build;
  }

  void SetIsNormalBootMode(bool is_normal_boot_mode) {
    is_normal_boot_mode_ = is_normal_boot_mode;
  }

  void SetAreDevFeaturesEnabled(bool are_dev_features_enabled) {
    are_dev_features_enabled_ = are_dev_features_enabled;
  }

  // Sets the SetIsOOBEEnabled to |is_oobe_enabled|.
  void SetIsOOBEEnabled(bool is_oobe_enabled) {
    is_oobe_enabled_ = is_oobe_enabled;
  }

  // Sets the IsOOBEComplete to True with the given timestamp.
  void SetIsOOBEComplete(base::Time oobe_timestamp) {
    is_oobe_complete_ = true;
    oobe_timestamp_ = oobe_timestamp;
  }

  void UnsetIsOOBEComplete() { is_oobe_complete_ = false; }

  void SetHardwareClass(const std::string& hardware_class) {
    hardware_class_ = hardware_class;
  }

  void SetDeviceRequisition(const std::string& requisition) {
    device_requisition_ = requisition;
  }

  void SetMinKernelKeyVersion(int min_kernel_key_version) {
    min_kernel_key_version_ = min_kernel_key_version;
  }

  void SetMinFirmwareKeyVersion(int min_firmware_key_version) {
    min_firmware_key_version_ = min_firmware_key_version;
  }

  void SetPowerwashCount(int powerwash_count) {
    powerwash_count_ = powerwash_count;
  }

  void SetBuildTimestamp(int64_t build_timestamp) {
    build_timestamp_ = build_timestamp;
  }

  void SetWarmReset(bool warm_reset) override { warm_reset_ = warm_reset; }

  void SetVbmetaDigestForInactiveSlot(bool reset) override {}

  // Getters to verify state.
  int GetMaxKernelKeyRollforward() const { return kernel_max_rollforward_; }

  bool GetIsRollbackPowerwashScheduled() const {
    return powerwash_scheduled_ && save_rollback_data_;
  }
  std::string GetVersionForLogging(
      const std::string& partition_name) const override {
    return partition_timestamps_[partition_name];
  }
  void SetVersion(const std::string& partition_name, std::string timestamp) {
    partition_timestamps_[partition_name] = std::move(timestamp);
  }
  ErrorCode IsPartitionUpdateValid(
      const std::string& partition_name,
      const std::string& new_version) const override {
    const auto old_version = GetVersionForLogging(partition_name);
    return utils::IsTimestampNewer(old_version, new_version);
  }

  const char* GetPartitionMountOptions(
      const std::string& partition_name) const override {
#ifdef __ANDROID__
    // TODO(allight): This matches the declaration in hardware_android.cc but
    // ideally shouldn't be duplicated.
    return "defcontext=u:object_r:postinstall_file:s0";
#else
    return "";
#endif
  }

 private:
  bool is_official_build_{true};
  bool is_normal_boot_mode_{true};
  bool are_dev_features_enabled_{false};
  bool is_oobe_enabled_{true};
  bool is_oobe_complete_{true};
  // Jan 20, 2007
  base::Time oobe_timestamp_{base::Time::FromTimeT(1169280000)};
  std::string hardware_class_{"Fake HWID BLAH-1234"};
  std::string device_requisition_{"fake_requisition"};
  int min_kernel_key_version_{kMinKernelKeyVersion};
  int min_firmware_key_version_{kMinFirmwareKeyVersion};
  int kernel_max_rollforward_{kKernelMaxRollforward};
  int firmware_max_rollforward_{kFirmwareMaxRollforward};
  int powerwash_count_{kPowerwashCountNotSet};
  bool powerwash_scheduled_{false};
  bool save_rollback_data_{false};
  int64_t build_timestamp_{0};
  bool first_active_omaha_ping_sent_{false};
  bool warm_reset_{false};
  mutable std::map<std::string, std::string> partition_timestamps_;

  DISALLOW_COPY_AND_ASSIGN(FakeHardware);
};

}  // namespace chromeos_update_engine

#endif  // UPDATE_ENGINE_COMMON_FAKE_HARDWARE_H_
