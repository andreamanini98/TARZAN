#include "TARZAN/testing/backwardReachability/generic_backward_tests.h"
#include "TARZAN/testing/backwardReachability/fischer_backward.h"
#include "TARZAN/testing/backwardReachability/flower_backward.h"
#include "TARZAN/testing/backwardReachability/bridge_backward.h"
#include "TARZAN/testing/backwardReachability/trainAHV93_backward.h"


int main()
{
    //testTrainAHV93Flat2Efficient();

    //testTrainAHV93Flat2Explodes();

    //testTrainAHV93Flat3Efficient();

    //testTrainAHV93Flat3Explodes();

    testTrainAHV93Flat3Reachable();

    return 0;
}
