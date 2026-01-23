#include "TARZAN/testing/gamesTesting.h"


int main()
{
    testProductionCellWin();
    testProductionCellLose();
    testProductionCellSafetyWin();
    testProductionCellSafetyLose();

    // testSolveTimedCLTLocGames();

    formula2();

    formula2_controllerCannotCycle();

    formula3_lose();

    formula3_win();

    formula2_controllerCanCycleOnlyInFall_d();

    formula2_controllerCanCycleOnlyInFall_d_lose();

    return 0;
}
