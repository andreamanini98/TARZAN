#ifndef PRINTING_UTILITIES_H
#define PRINTING_UTILITIES_H

#include <sstream>
#include <string>
#include <concepts>
#include <type_traits>

/**
 * @brief Creates an std::string with elements of a container joined by a delimiter
 *
 * @tparam Container Type of the container (must be iterable)
 * @tparam Converter Type of the conversion function
 * @param container The container of elements to join
 * @param delimiter The delimiter to insert between elements
 * @param converter Function to convert each element to string (defaults to to_string())
 * @return std::string with the joined elements
 */
template<typename Container, typename Converter = std::nullptr_t>
std::string join_elements(const Container &container,
                          const std::string &delimiter,
                          Converter converter = nullptr)
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
            // If no converter provided, try to use to_string() method or direct insertion
            if constexpr (requires { element.to_string(); })
                oss << element.to_string();
            else
                oss << element;
        } else
        // Use the provided converter function
            oss << converter(element);
    }
    return oss.str();
}


#endif //PRINTING_UTILITIES_H
