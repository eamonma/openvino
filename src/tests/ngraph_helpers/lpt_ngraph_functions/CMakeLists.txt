# Copyright (C) 2018-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

set(TARGET_NAME lptNgraphFunctions)

set(PUBLIC_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

addIeTarget(
        NAME ${TARGET_NAME}
        TYPE STATIC
        ROOT ${PUBLIC_HEADERS_DIR}
        INCLUDES
            PUBLIC
                ${PUBLIC_HEADERS_DIR}
        ADDITIONAL_SOURCE_DIRS
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        LINK_LIBRARIES
            PRIVATE
                ngraphFunctions
                openvino::runtime::dev
        ADD_CPPLINT
        DEPENDENCIES
            ngraphFunctions
        DEVELOPER_PACKAGE
            tests
)

ov_build_target_faster(${TARGET_NAME}
    UNITY
    PCH PRIVATE "src/precomp.hpp"
)
