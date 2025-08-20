#ifndef FILE_UTILITIES_H
#define FILE_UTILITIES_H

#include <string>
#include <iostream>
#include <fstream>


/**
* @brief Reads a file.
*
* @param path: the path in which the file to read is located.
* @returns a std::string representation of the file.
**/
inline std::string readFromFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "Attempted to open: " << path << std::endl;
        throw std::runtime_error("Failed to open file: " + path);
    }

    std::string content{ std::istreambuf_iterator(file), std::istreambuf_iterator<char>() };

    file.close();
    return content;
}


#endif //FILE_UTILITIES_H
