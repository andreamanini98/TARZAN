#include "TARZAN/testing/gamesTesting.h"


int main()
{
    testProductionCellWin();
    testProductionCellLose();
    testProductionCellSafetyWin();
    testProductionCellSafetyLose();

    testSolveTimedCLTLocGames();

    return 0;
}
