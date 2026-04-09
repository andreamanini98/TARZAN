#ifndef PRINTING_UTILITIES_H
#define PRINTING_UTILITIES_H

#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "TARZAN/regions/Region.h"


/**
 * @brief Creates a std::string with elements of a container joined by a delimiter.
 *
 * @tparam Container Type of the container (must be iterable)
 * @tparam Converter Type of the conversion function
 * @param container The container of elements to join
 * @param delimiter The delimiter to insert between elements
 * @param converter Function to convert each element to string (defaults to to_string())
 * @return std::string with the joined elements
 */
template<typename Container, typename Converter = std::nullptr_t>
std::string join_elements(const Container &container, const std::string &delimiter, Converter converter = nullptr)
{
    std::ostringstream oss;

    bool first = true;
    for (const auto &element: container)
    {
        if (!first)
            oss << delimiter;
        first = false;

        if constexpr (std::is_same_v<Converter, std::nullptr_t>)
        {
            // If no converter provided, try to use to_string() method or direct insertion.
            if constexpr (requires { element.to_string(); })
                oss << element.to_string();
            else
                oss << element;
        } else
        // Use the provided converter function.
            oss << converter(element);
    }
    return oss.str();
}


/**
 * @brief Prints a clock valuation with clock names during strategy synthesis.
 *
 * @param cv the clock valuation to print.
 * @param indicesToClocks a map from clock indices to clock names.
 * @param indent the indentation string to prepend to each line.
 */
// TODO: if desired, this function can be modified to show the intervals in which clock values fall.
inline void printClockValuationInStrategy(const std::vector<std::pair<int, bool>> &cv,
                                          const std::unordered_map<int, std::string> &indicesToClocks,
                                          const std::string &indent)
{
    for (int i = 0; i < static_cast<int>(cv.size()); i++)
        std::cout << indent << indicesToClocks.at(i) << " := (" << cv[i].first << ", " << (cv[i].second ? "frac > 0" : "frac = 0") << ")\n";
}


/**
 * @brief Prints a region (location name and clock valuation).
 *
 * @param reg the region to print.
 * @param intToLocations a map from location indices to location names.
 * @param indicesToClocks a map from clock indices to clock names.
 * @param locationsToPlayers a map from location indices to players.
 * @param indent the indentation string to prepend to each line.
 */
inline void printRegionInStrategy(const region::Region &reg,
                                  const std::unordered_map<int, std::string> &intToLocations,
                                  const std::unordered_map<int, std::string> &indicesToClocks,
                                  const absl::flat_hash_map<int, players_sym> &locationsToPlayers,
                                  const std::string &indent)
{
    const int regLocation = reg.getLocation();
    std::cout << indent << intToLocations.at(regLocation) << " [" << locationsToPlayers.at(regLocation) << "]\n";
    printClockValuationInStrategy(reg.getClockValuation(), indicesToClocks, indent);
}


/**
 * @brief Repeats a string n times.
 *
 * @param str the string to repeat.
 * @param n how many times to repeat the string.
 * @return a string repeated n times.
 */
inline std::string repeatString(const std::string &str, const size_t n)
{
    std::string result;
    result.reserve(str.size() * n);

    for (size_t i = 0; i < n; i++)
        result += str;

    return result;
}

#endif //PRINTING_UTILITIES_H
