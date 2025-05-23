//
// Copyright (C) 2012 The Android Open Source Project
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

#ifndef UPDATE_ENGINE_COMMON_FAKE_PREFS_H_
#define UPDATE_ENGINE_COMMON_FAKE_PREFS_H_

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include <android-base/macros.h>

#include "update_engine/common/prefs_interface.h"

namespace chromeos_update_engine {

// Implements a fake preference store by storing the value associated with
// a key in a std::map, suitable for testing. It doesn't allow to set a value on
// a key with a different type than the previously set type. This enforces the
// type of a given key to be fixed. Also the class checks that the Get*()
// methods aren't called on a key set with a different type.

class FakePrefs : public PrefsInterface {
 public:
  FakePrefs() = default;
  ~FakePrefs();

  // PrefsInterface methods.
  bool GetString(std::string_view key, std::string* value) const override;
  bool SetString(std::string_view key, std::string_view value) override;
  bool GetInt64(std::string_view key, int64_t* value) const override;
  bool SetInt64(std::string_view key, const int64_t value) override;
  bool GetBoolean(std::string_view key, bool* value) const override;
  bool SetBoolean(std::string_view key, const bool value) override;

  bool Exists(std::string_view key) const override;
  bool Delete(std::string_view key) override;
  bool Delete(std::string_view key,
              const std::vector<std::string>& nss) override;

  bool GetSubKeys(std::string_view ns,
                  std::vector<std::string>* keys) const override;

  void AddObserver(std::string_view key, ObserverInterface* observer) override;
  void RemoveObserver(std::string_view key,
                      ObserverInterface* observer) override;
  bool StartTransaction() override { return false; }
  bool CancelTransaction() override { return false; }
  bool SubmitTransaction() override { return false; }

 private:
  enum class PrefType {
    kString,
    kInt64,
    kBool,
  };
  struct PrefValue {
    std::string as_str;
    int64_t as_int64;
    bool as_bool;
  };

  struct PrefTypeValue {
    PrefType type;
    PrefValue value;
  };

  // Class to store compile-time type-dependent constants.
  template <typename T>
  class PrefConsts {
   public:
    // The PrefType associated with T.
    static FakePrefs::PrefType const type;

    // The data member pointer to PrefValue associated with T.
    static T FakePrefs::PrefValue::*const member;
  };

  // Returns a string representation of the PrefType useful for logging.
  static std::string GetTypeName(PrefType type);

  // Checks that the |key| is either not present or has the given |type|.
  void CheckKeyType(std::string_view key, PrefType type) const;

  // Helper function to set a value of the passed |key|. It sets the type based
  // on the template parameter T.
  template <typename T>
  void SetValue(std::string_view key, T value);

  // Helper function to get a value from the map checking for invalid calls.
  // The function fails the test if you attempt to read a value  defined as a
  // different type. Returns whether the get succeeded.
  template <typename T>
  bool GetValue(std::string_view key, T* value) const;

  // Container for all the key/value pairs.
  std::map<std::string, PrefTypeValue, std::less<>> values_;

  // The registered observers watching for changes.
  std::map<std::string, std::vector<ObserverInterface*>, std::less<>>
      observers_;

  DISALLOW_COPY_AND_ASSIGN(FakePrefs);
};

}  // namespace chromeos_update_engine

#endif  // UPDATE_ENGINE_COMMON_FAKE_PREFS_H_
