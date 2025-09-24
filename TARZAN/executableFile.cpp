#include "TARZAN/testing/successorsAndPredecessorsTesting.h"

// #define REGION_TIMING


int main()
{
#ifdef REGION_TIMING
    std::cout << "Tick period: " << static_cast<double>(std::chrono::high_resolution_clock::period::num) / std::chrono::high_resolution_clock::period::den <<
            " seconds\n";
    const auto start = std::chrono::high_resolution_clock::now();
#endif


    // testDiscretePredecessorsLightSwitch();

    // Con il flower da 16 clock:
    // foreword: Function took: 35,845,987 microseconds
    // backward: Function took: 23,750,887 microseconds
    testFlowerBackwards();


#ifdef REGION_TIMING
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Function took: " << duration.count() << " microseconds" << std::endl;
#endif

    return 0;
}
