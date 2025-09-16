#ifndef TARZAN_FUNCTION_UTILITIES_H
#define TARZAN_FUNCTION_UTILITIES_H


/// Returns the correct index for accessing a clock in a bitvector.
inline int cIdx(const int numOfClocks, const int i)
{
    return numOfClocks - 1 - i;
}


#endif //TARZAN_FUNCTION_UTILITIES_H
