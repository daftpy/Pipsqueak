//
// Created by Daftpy on 7/24/2025.
//

#ifndef CORE_TYPES_HPP
#define CORE_TYPES_HPP
#include <vector>

namespace pipsqueak::core {
    using Sample = float;

    // The PCM data buffer data is represented as a vector of interleaved 32-bit floats.
    using PCMData = std::vector<Sample>;
}

#endif //CORE_TYPES_HPP