#pragma once

#include "../Deserialization/ExtractRequest.h"
#include <string_view>

namespace ingress_pipeline {

/** Wraps ExtractRequest — single place to swap parsing later. */
struct RequestParser {
    static RequestStruct parse(std::string_view json) {
        ExtractRequest extractor;
        return extractor.extract(std::string(json));
    }
};

} // namespace ingress_pipeline
