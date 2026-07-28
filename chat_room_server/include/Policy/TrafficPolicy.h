#pragma once
#include <cstdint>
#include "../Deserialization/DTO/RequestStruct.h" 

namespace Policy {

    enum class TrafficLane : uint8_t {
        FAST,
        SLOW
    };

    struct TrafficDecision {
        TrafficLane lane;
        bool requires_ordering;    
    };

    struct TrafficPolicy {

        static constexpr TrafficDecision decide(RequestType type) noexcept {

            switch (type) {

                case RequestType::LOGIN_REQUEST:
                    return {
                        TrafficLane::FAST,
                        false
                    };

                case RequestType::MESSAGE_REQUEST:
                case RequestType::FIRST_MESSAGE_REQUEST:
                    return {
                        TrafficLane::SLOW,
                        true
                    };

                default:
                    return {
                        TrafficLane::SLOW,
                        false
                    };
            }
        }

      
        static constexpr std::uint32_t maxInflightPerClient() noexcept {
            return 4;
        }
    };

} 
