#include "TARZAN/testing/backward_reachability/trainAHV93_backward.h"


int main()
{
    testTrainAHV93Flat2Efficient();

    std::cout << "\n---\n\n";

    testTrainAHV93Flat2Reachable();

    std::cout << "\n---\n\n";

    testTrainAHV93Flat3Efficient();

    std::cout << "\n---\n\n";

    testTrainAHV93Flat3Reachable();

    return 0;
}
