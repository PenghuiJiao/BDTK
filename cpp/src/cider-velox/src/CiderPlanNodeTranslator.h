/*
 * Copyright(c) 2022-2023 Intel Corporation.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#pragma once

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include "CiderCrossJoinBuild.h"
#include "CiderHashJoinBuild.h"
#include "CiderJoinBuild.h"
#include "CiderOperator.h"
#include "CiderPipelineOperator.h"
#include "CiderPlanNode.h"
#include "CiderVeloxOptions.h"
#include "exec/plan/substrait/SubstraitPlan.h"

namespace facebook::velox::plugin {

class CiderPlanNodeTranslator : public exec::Operator::PlanNodeTranslator {
 public:
  explicit CiderPlanNodeTranslator(
      uint32_t maxDrivers = std::numeric_limits<uint32_t>::max())
      : maxDrivers_{maxDrivers} {}

  std::unique_ptr<exec::Operator> toOperator(
      exec::DriverCtx* ctx,
      int32_t id,
      const std::shared_ptr<const core::PlanNode>& node) override {
    if (auto ciderPlanNode = std::dynamic_pointer_cast<const CiderPlanNode>(node)) {
      if (FLAGS_enable_batch_processor) {
        return std::make_unique<CiderPipelineOperator>(id, ctx, ciderPlanNode);
      } else {
        return CiderOperator::Make(id, ctx, ciderPlanNode);
      }
    }
    return nullptr;
  }

  std::unique_ptr<exec::JoinBridge> toJoinBridge(
      const std::shared_ptr<const core::PlanNode>& node) override {
    if (auto ciderJoinNode = std::dynamic_pointer_cast<const CiderPlanNode>(node)) {
      auto planUtil = std::make_shared<cider::exec::plan::SubstraitPlan>(
          ciderJoinNode->getSubstraitPlan());
      if (FLAGS_enable_batch_processor) {
        if (planUtil->hasCrossRel()) {
          return std::make_unique<CiderCrossJoinBridge>();
        } else {
          // planUtil->hasJoinRel()
          return std::make_unique<CiderHashJoinBridge>();
        }

      } else {
        return std::make_unique<CiderJoinBridge>();
      }
    }
    return nullptr;
  }

  exec::OperatorSupplier toOperatorSupplier(
      const std::shared_ptr<const core::PlanNode>& node) override {
    if (auto ciderJoinNode = std::dynamic_pointer_cast<const CiderPlanNode>(node)) {
      return [ciderJoinNode](int32_t operatorId,
                             exec::DriverCtx* ctx) -> std::unique_ptr<exec::Operator> {
        auto planUtil = std::make_shared<cider::exec::plan::SubstraitPlan>(
            ciderJoinNode->getSubstraitPlan());
        if (FLAGS_enable_batch_processor) {
          if (planUtil->hasCrossRel()) {
            return std::make_unique<CiderCrossJoinBuild>(operatorId, ctx, ciderJoinNode);
          } else {
            // planUtil->hasJoinRel()
            return std::make_unique<CiderHashJoinBuild>(operatorId, ctx, ciderJoinNode);
          }

        } else {
          return std::make_unique<CiderJoinBuild>(operatorId, ctx, ciderJoinNode);
        }
      };
    }
    return nullptr;
  }

  std::optional<uint32_t> maxDrivers(
      const std::shared_ptr<const core::PlanNode>& node) override {
    if (auto ciderPlanNode = std::dynamic_pointer_cast<const CiderPlanNode>(node)) {
      return maxDrivers_;
    }
    return std::nullopt;
  }

 private:
  uint32_t maxDrivers_;
};

}  // namespace facebook::velox::plugin
