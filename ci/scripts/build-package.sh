#!/bin/bash
# Copyright (c) 2022 Intel Corporation.

# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
cd "$(dirname "$0")"

BUILD_MODE=Debug
if [ "$1" = "release" ] || [ "$1" = "Release" ] 
then 
    BUILD_MODE=Release
fi

pushd ../../
rm -rf bdtk-package.tar.gz bdtk-package
mkdir bdtk-package
cp ./build-${BUILD_MODE}/cider-velox/src/libvelox_plugin.a ./bdtk-package
cp ./build-${BUILD_MODE}/cider-velox/src/ciderTransformer/libcider_plan_transformer.a ./bdtk-package
cp ./build-${BUILD_MODE}/cider-velox/src/planTransformer/libvelox_plan_transformer.a ./bdtk-package
cp ./build-${BUILD_MODE}/cider-velox/src/substrait/libvelox_substrait_convertor.a ./bdtk-package
cp -a ./build-${BUILD_MODE}/cider/exec/module/libcider.so* ./bdtk-package
cp ./build-${BUILD_MODE}/thirdparty/velox/velox/substrait/libvelox_substrait_plan_converter.a ./bdtk-package
cp ./build-${BUILD_MODE}/cider/exec/template/libQueryEngine.a ./bdtk-package
cp ./build-${BUILD_MODE}/cider/function/libcider_function.a ./bdtk-package

cp ./build-${BUILD_MODE}/cider/function/RuntimeFunctions.bc ./bdtk-package
tar -czvf bdtk-package.tar.gz bdtk-package
popd
# export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./lib
# ./bin/presto_server -etc_dir=./etc

