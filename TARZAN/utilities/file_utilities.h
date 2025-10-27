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


//  TODO: vedere se fare funzioni generiche di scrittura a file generici.
namespace benchmark
{
    inline std::string benchmarkName;


    /**
     * @brief Writes the benchmarks results on file.
     *
     * @param benchmarkResults the strings to write on file.
     */
    inline void writeBenchmarkResult(const std::vector<std::string> &benchmarkResults)
    {
        const std::string benchmarkFilePath = "/Users/echo/Desktop/PhD/Tools/TARZAN/output/";
        const std::string benchmarkFileName = "benchmark_results.txt";

        std::ofstream results(benchmarkFilePath + benchmarkFileName, std::ios::app);

        results << benchmarkName << std::endl;
        for (const auto &result: benchmarkResults)
            results << result << std::endl;
        results << std::endl;

        results.close();
    }
}

#endif //FILE_UTILITIES_H
