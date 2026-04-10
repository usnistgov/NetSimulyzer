#pragma once
#include <type_traits>

namespace netsimulyzer {

template <typename T, typename... Ts>
inline constexpr bool is_one_of_v = (std::is_same_v<T, Ts> || ...);

}

