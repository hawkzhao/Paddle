/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include <deque>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

#include "paddle/framework/op_desc.h"
#include "paddle/framework/proto_desc.h"
#include "paddle/framework/var_desc.h"
#include "paddle/platform/macros.h"

namespace paddle {
namespace framework {

class ProgramDesc;

// Each Protobuf Message, we provide a XXXBind class. In that class, we optimize
// read/write speed. Only when we want the protobuf message, the local changes
// will be synchronized (by `Sync` method).

class BlockDesc {
 public:
  BlockDesc(ProgramDesc *prog, proto::BlockDesc *desc);

  BlockDesc(const BlockDesc &other, proto::BlockDesc *desc, ProgramDesc *prog);

  ~BlockDesc() {
    this->ClearPBVars();
    this->ClearPBOps();
  }

  int32_t ID() const { return desc_->idx(); }

  int32_t Parent() const { return desc_->parent_idx(); }

  VarDesc *Var(const std::string &name_bytes);

  VarDesc *FindVar(const std::string &name_bytes) const;

  bool HasVar(const std::string &var_name) const;

  VarDesc *FindVarRecursive(const std::string &name_bytes) const;

  VarDesc &FindRecursiveOrCreateVar(const std::string &name_bytes);

  bool HasVarRecursive(const std::string &var_name) const;

  std::set<std::string> LocalVarNames() const {
    std::set<std::string> var_names;
    for (auto &var : vars_) {
      var_names.insert(var.first);
    }
    return var_names;
  }

  std::vector<VarDesc *> AllVars() const;

  BlockDesc *ParentBlock() const;

  OpDesc *AppendOp();

  void AppendAllocatedOp(std::unique_ptr<OpDesc> &&op_desc);

  OpDesc *PrependOp();

  void RemoveOp(size_t s, size_t e);

  std::vector<OpDesc *> AllOps() const;

  size_t OpSize() const { return ops_.size(); }

  OpDesc *Op(int idx) { return ops_.at(idx).get(); }

  void Flush();

  proto::BlockDesc *Proto();

  ProgramDesc *Program() { return this->prog_; }

 private:
  void ClearPBOps();
  void ClearPBVars();

 private:
  ProgramDesc *prog_;       // not_own
  proto::BlockDesc *desc_;  // not_own
  bool need_update_;

  std::deque<std::unique_ptr<OpDesc>> ops_;
  std::unordered_map<std::string, std::unique_ptr<VarDesc>> vars_;

  DISABLE_COPY_AND_ASSIGN(BlockDesc);
};
}  // namespace framework
}  // namespace paddle
