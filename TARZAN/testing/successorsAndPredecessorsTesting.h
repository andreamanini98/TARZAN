// #ifndef TARZAN_SUCCESSORSANDPREDECESSORSTESTING_H
// #define TARZAN_SUCCESSORSANDPREDECESSORSTESTING_H
//
// #include <string>
//
// #include "TARZAN/headers/library.h"
// #include "TARZAN/parser/ast.h"
// #include "TARZAN/regions/Region.h"
// #include "TARZAN/regions/RTS.h"
// #include "absl/container/flat_hash_set.h"
// #include "absl/container/btree_map.h"
// #include "TARZAN/utilities/partition_utilities.h"
// #include <omp.h>
// #include <algorithm>
// #include "TARZAN/regions/networkOfTA/RTSNetwork.h"
// #include "TARZAN/testing/successorsAndPredecessorsTesting.h"
// #include "TARZAN/parser/enums/input_output_action_enum.h"
// #include "TARZAN/utilities/file_utilities.h"
// #include "TARZAN/parser/timed_automaton_def.h"
// #include "TARZAN/parser/config.h"
//
//
// inline region::Region getRegion(int numSteps)
// {
//     region::Region reg(3, {});
//
//     const auto newH = static_cast<int *>(malloc(sizeof(int) * 3));
//     newH[0] = 0;
//     newH[1] = 0;
//     newH[2] = 1;
//
//     reg.set_h(newH);
//
//     boost::dynamic_bitset<> xm1(3);
//     boost::dynamic_bitset<> x0(3);
//     boost::dynamic_bitset<> x1(3);
//
//     xm1.set(0, true);
//     x0.set(2, true);
//     x1.set(1, true);
//
//     const std::deque Xm1{ xm1 };
//     const std::deque X1{ x1 };
//
//     reg.set_unbounded(Xm1);
//     reg.set_x0(x0);
//     reg.set_bounded(X1);
//
//     region::Region oldSuccessor = reg;
//     for (int i = 0; i < numSteps; i++)
//     {
//         region::Region successor = oldSuccessor.getImmediateDelaySuccessor({ 1, 1, 1 });
//         oldSuccessor = successor;
//     }
//
//     return oldSuccessor;
// }
//
//
// inline void testParsing()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//
//     const std::string automatonFileName = "light_switch.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\n\n\n";
//     std::cout << "Parsed automaton: " << automaton << std::endl;
// }
//
//
// inline void testImmediateDelaySuccessor()
// {
//     region::Region oldSuccessor = getRegion(0);
//     for (int i = 0; i < 7; i++)
//     {
//         region::Region successor = oldSuccessor.getImmediateDelaySuccessor({ 1, 1, 1 });
//         std::cout << "Successor " << (i + 1) << ":\n" << successor.toString() << std::endl;
//         oldSuccessor = successor;
//     }
// }
//
//
// inline void testImmediateDelayPredecessors(const int totSteps, const int maxConst)
// {
//     region::Region oldSuccessor = getRegion(0);
//
//     for (int i = 0; i < totSteps; i++)
//     {
// #ifdef REGION_PRINT_DEBUG
//         std::cout << oldSuccessor.toString() << std::endl;
// #endif
//         const region::Region successor = oldSuccessor.getImmediateDelaySuccessor({ maxConst, maxConst, maxConst });
//         oldSuccessor = successor;
//     }
//
//     std::cout << oldSuccessor.toString() << std::endl;
//
//     std::vector<region::Region> oldPred;
//     std::vector<region::Region> newPred;
//     oldPred.push_back(oldSuccessor);
//
//     for (int i = 0; i < totSteps; i++)
//     {
//         for (const region::Region &p: oldPred)
//         {
//             std::vector<region::Region> predecessors = p.getImmediateDelayPredecessors();
// #ifdef REGION_PRINT_DEBUG
//             if (!predecessors.empty())
//             {
//                 std::cout << "Predecessors!\n";
//                 for (const region::Region &r: predecessors)
//                 {
//                     std::cout << "Predecessor " << (i + 1) << ":\n";
//                     std::cout << r.toString() << std::endl;
//                 }
//             }
// #endif
//             newPred.insert(newPred.end(), predecessors.begin(), predecessors.end());
//         }
//
//         oldPred = newPred;
//         newPred.clear();
//     }
// }
//
//
// inline void testLocationMapping()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "ta0.txt";
//
//     const timed_automaton::ast::timedAutomaton TA = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << TA << std::endl;
//
//     // ReSharper disable once CppTooWideScopeInitStatement
//     std::unordered_map<std::string, int> map = TA.mapLocationsToInt();
//
//     for (const auto &[key, value]: map)
//         std::cout << key << ": " << value << std::endl;
//
//     std::cout << "\nNow printing the transitions:" << std::endl;
//     const std::vector<std::vector<timed_automaton::ast::transition>> outTransitions = TA.getOutTransitions(map);
//
//     std::cout << outTransitions.size() << std::endl;
//
//     for (size_t i = 0; i < outTransitions.size(); ++i)
//     {
//         std::cout << "Index " << i << ":" << std::endl;
//         for (const auto &transition: outTransitions[i])
//             std::cout << "  " << transition << std::endl;
//         std::cout << std::endl;
//     }
// }
//
//
// inline void testClockValuation()
// {
//     const region::Region reg = getRegion(4);
//
//     std::cout << "We now test the clock valuation of: " << reg.toString() << std::endl;
//
//     const std::vector<std::pair<int, bool>> clockValuation = reg.getClockValuation();
//     std::ostringstream oss;
//     oss << "Clock Valuation: [";
//     for (size_t i = 0; i < clockValuation.size(); i++)
//     {
//         if (i > 0) oss << ", ";
//         oss << "clock" << i << ": (" << clockValuation[i].first
//                 << ", " << (clockValuation[i].second ? "has_fraction" : "no_fraction") << ")";
//     }
//     oss << "]";
//     std::cout << oss.str() << std::endl;
// }
//
//
// inline void testClockIndices()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//
//     const std::string automatonFileName = "ta0.txt";
//
//     // ReSharper disable once CppTooWideScopeInitStatement
//     timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     for (const auto &[fst, snd]: automaton.getClocksIndices())
//     {
//         std::cout << fst << ", " << snd << std::endl;
//     }
// }
//
//
// inline void testTransitionIsSatisfiable()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "ta0.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\nThe transitions of the automaton are:" << std::endl;
//     for (const auto &tr: automaton.transitions)
//         std::cout << tr << std::endl;
//
//     std::vector<std::pair<int, bool>> clockValuation{};
//     clockValuation.emplace_back(2, true);
//     clockValuation.emplace_back(5, false);
//     clockValuation.emplace_back(5, true);
//
//     std::unordered_map<std::string, int> clocksIndices{};
//     clocksIndices.emplace("x", 0);
//     clocksIndices.emplace("y", 1);
//     clocksIndices.emplace("z", 2);
//
//     std::cout << "Now trying to see if the transitions are satisfied:" << std::endl;
//     for (const auto &tr: automaton.transitions)
//         std::cout << tr << ": \n" << tr.isTransitionSatisfied(clockValuation, clocksIndices, {}) << std::endl;
// }
//
//
// inline void testImmediateDiscreteSuccessors()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "ta0.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\n";
//
//     region::Region reg(3, {});
//
//     const auto newH = static_cast<int *>(malloc(sizeof(int) * 3));
//     newH[0] = 0;
//     newH[1] = 0;
//     newH[2] = 0;
//     reg.set_h(newH);
//
//     boost::dynamic_bitset<> x0(3);
//     x0.set(0, true);
//     x0.set(1, true);
//     x0.set(2, true);
//     reg.set_x0(x0);
//
//     std::unordered_map<std::string, int> locationsIntMap = automaton.mapLocationsToInt();
//
//     reg.set_q(locationsIntMap.at("q0"));
//
//     std::vector<std::vector<transition>> outTransitions = automaton.getOutTransitions(locationsIntMap);
//
//     std::unordered_map<std::string, int> clockIndices = automaton.getClocksIndices();
//
//     std::cout << "Initial region: " << reg.toString() << std::endl;
//
//     region::Region oldSuccessor = reg;
//
//     absl::flat_hash_map<std::string, int> values{};
//
//     while (true)
//     {
//         std::vector<region::Region> discreteSuccessors = oldSuccessor.getImmediateDiscreteSuccessors(
//             outTransitions[reg.getLocation()], clockIndices, locationsIntMap
//         );
//
//         if (discreteSuccessors.empty())
//             std::cout << "Discrete successors are empty!" << std::endl;
//         else
//         {
//             std::cout << "Discrete successors found!" << std::endl;
//             std::cout << "Discrete Successors:\n";
//             std::cout << std::string(50, '-') << std::endl;
//
//             int count = 1;
//             for (const auto &region: discreteSuccessors)
//             {
//                 std::cout << "region::Region #" << count++ << ":\n";
//                 std::cout << region.toString();
//                 std::cout << std::string(30, '-') << std::endl;
//             }
//
//             break;
//         }
//
//         region::Region successor = oldSuccessor.getImmediateDelaySuccessor({ 10, 10, 10 });
//         std::cout << "Delay successor :\n" << successor.toString() << std::endl;
//         oldSuccessor = successor;
//     }
//
//     reg = oldSuccessor;
//     std::cout << "New initial region: " << reg.toString() << std::endl;
//
//     oldSuccessor = reg;
//
//     while (true)
//     {
//         std::vector<region::Region> discreteSuccessors = oldSuccessor.getImmediateDiscreteSuccessors(
//             outTransitions[reg.getLocation()], clockIndices, locationsIntMap
//         );
//
//         if (discreteSuccessors.empty())
//             std::cout << "Discrete successors are empty!" << std::endl;
//         else
//         {
//             std::cout << "Discrete successors found!" << std::endl;
//             std::cout << "Discrete Successors:\n";
//             std::cout << std::string(50, '-') << std::endl;
//
//             int count = 1;
//             for (const auto &region: discreteSuccessors)
//             {
//                 std::cout << "region::Region #" << count++ << ":\n";
//                 std::cout << region.toString();
//                 std::cout << std::string(30, '-') << std::endl;
//             }
//
//             break;
//         }
//
//         region::Region successor = oldSuccessor.getImmediateDelaySuccessor({ 10, 10, 10 });
//         std::cout << "Delay successor :\n" << successor.toString() << std::endl;
//         oldSuccessor = successor;
//     }
// }
//
//
// inline void testIsInitial()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "ta0.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\n";
//
//     std::unordered_map<std::string, int> locationsIntMap = automaton.mapLocationsToInt();
//
//     std::cout << "Location to Integer Mapping:\n";
//     for (const auto &[location, index]: locationsIntMap)
//     {
//         std::cout << location << " -> " << index << std::endl;
//     }
//
//     std::cout << "initial locations are: " << std::endl;
//     for (const std::vector<int> initials = automaton.getInitialLocations(locationsIntMap); const int loc: initials)
//         std::cout << loc << std::endl;
// }
//
//
// inline void testHash()
// {
//     region::Region r = getRegion(3);
//
//     std::cout << "Region: " << r.toString() << std::endl;
//
//     absl::flat_hash_set<region::Region, region::RegionHash> regions{};
//
//     regions.insert(r);
//
//     std::cout << "is r in regions? " << regions.contains(r) << std::endl;
//
//     region::Region r1 = getRegion(3);
//
//     regions.insert(r1);
//
//     region::Region r2 = getRegion(3);
//
//     std::cout << "is r in regions? " << regions.contains(r2) << std::endl;
//
//     region::Region r3 = getRegion(6);
//
//     std::cout << "is r in regions? " << regions.contains(r3) << std::endl;
// }
//
//
// inline void testRTS()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     // const std::string automatonFileName = "light_switch.txt";
//
//     const std::string automatonFileName = "test_flower_small.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     region::RTS regionTransitionSystem(automaton);
//
//     //std::cout << "Location to Integer Mapping:\n";
//     //for (const auto& [location, index] : automaton.mapLocationsToInt()) {
//     //    std::cout << location << " -> " << index << std::endl;
//     //}
//
//     std::cout << "\n\n\n";
//
//     std::cout << "Computed the following regions:" << std::endl;
//
//     const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);
//
//     for (const auto &region: rts)
//         std::cout << region.toString() << std::endl;
// }
//
//
// inline void testPartitionBitset()
// {
//     const boost::dynamic_bitset bitset(std::string("10110"));
//     std::vector<std::vector<int>> partitions{};
//     std::vector<int> activeBitsIndices{};
//     partitionBitset(bitset, partitions, activeBitsIndices);
//
//     const double t1 = omp_get_wtime();
//     const std::vector<std::vector<boost::dynamic_bitset<>>> bitsets = getBitsetsFromRestrictedGrowthStrings(5, partitions, activeBitsIndices);
//     const double t2 = omp_get_wtime();
//
//     std::cout << "\n";
//     std::cout << "Total time: " << t2 - t1 << std::endl;
//     std::cout << "\n";
//
//     std::cout << "[\n";
//
//     for (size_t i = 0; i < bitsets.size(); ++i)
//     {
//         std::cout << "  [" << i << "]: [ ";
//
//         const auto &innerVector = bitsets[i];
//         for (size_t j = 0; j < innerVector.size(); ++j)
//         {
//             std::cout << innerVector[j];
//             if (j < innerVector.size() - 1)
//             {
//                 std::cout << ", ";
//             }
//         }
//
//         std::cout << " ]";
//         if (i < bitsets.size() - 1)
//         {
//             std::cout << ",";
//         }
//         std::cout << "\n";
//     }
//
//     std::cout << "]\n";
// }
//
//
// inline void printDeque(const std::deque<boost::dynamic_bitset<>> &dq, const std::string &prefix = "")
// {
//     std::cout << prefix;
//     for (const auto &b: dq) std::cout << b << " ";
//     std::cout << "\n";
// }
//
//
// inline void printComparison(const std::deque<boost::dynamic_bitset<>> &userDeque, const std::deque<boost::dynamic_bitset<>> &resultDeque)
// {
//     std::cout << "[";
//     const size_t maxLen = std::max(userDeque.size(), resultDeque.size());
//     for (size_t i = 0; i < maxLen; ++i)
//     {
//         std::string mark = " ";
//         if (i < userDeque.size() && i < resultDeque.size() && userDeque[i] != resultDeque[i])
//             mark = ""; // difference
//         if (i >= resultDeque.size())
//         {
//             std::cout << "(none)" << mark << " ";
//         } else
//         {
//             std::cout << resultDeque[i] << mark << " ";
//         }
//     }
//     std::cout << "]\n";
// }
//
//
// inline void testGenerateDeques()
// {
//     absl::btree_map<int, std::vector<boost::dynamic_bitset<>>, std::greater<>> myMap;
//
//     // Map with 3 keys
//     myMap[2] = {
//         boost::dynamic_bitset<>(3, 0b001),
//         boost::dynamic_bitset<>(3, 0b010),
//         boost::dynamic_bitset<>(3, 0b100)
//     }; // vector length 3
//     myMap[1] = {
//         boost::dynamic_bitset<>(3, 0b100),
//         boost::dynamic_bitset<>(3, 0b111)
//     };
//     myMap[0] = {
//         //boost::dynamic_bitset<>(3, 0b101),
//         boost::dynamic_bitset<>(3, 0b111)
//     };
//
//     // User-defined deque of length 3, initialized with zeros
//     std::deque<boost::dynamic_bitset<>> userDeque(3, boost::dynamic_bitset<>(3, 0));
//
//     std::cout << "User-defined starting deque:\n";
//     printDeque(userDeque, "  ");
//
//     // Generate all deques
//     const double t1 = omp_get_wtime();
//     const auto allDeques = generateAllDeques(myMap, userDeque);
//     const double t2 = omp_get_wtime();
//
//     std::cout << "\n";
//     std::cout << "Total time: " << t2 - t1 << std::endl;
//     std::cout << "\n";
//
//     std::cout << "\nTotal resulting deques: " << allDeques.size() << "\n\n";
//
//     int count = 0;
//     for (const auto &dq: allDeques)
//     {
//         std::cout << "Resulting deque " << ++count << ": ";
//         printComparison(userDeque, dq);
//     }
// }
//
//
// inline void testPermRegs()
// {
//     // Creating a region with numOfClocks clocks.
//     constexpr int numOfClocks = 5;
//     region::Region regToPerm(numOfClocks, {});
//
//     // Creating q for regToPerm;
//     constexpr int q = 3;
//
//     // Creating h for regToPerm.
//     constexpr int maxConstant = 5;
//     const auto h = static_cast<int *>(malloc(sizeof(int) * numOfClocks));
//     for (int i = 0; i < numOfClocks; i++)
//     {
//         constexpr int multiplier = 37;
//         h[i] = i * multiplier % maxConstant + 1;
//     }
//
//     // Creating unbounded clocks for RegToPerm.
//     std::deque<boost::dynamic_bitset<>> unbounded;
//     // Creating clock sets.
//     boost::dynamic_bitset<> xm1(numOfClocks);
//     xm1.set(0, true);
//     // Inserting clock sets in bounded deque.
//     unbounded.push_back(xm1);
//
//     // Creating x0 for regToPerm.
//     boost::dynamic_bitset<> x0(numOfClocks);
//
//     // Creating bounded clocks for regToPerm.
//     std::deque<boost::dynamic_bitset<>> bounded;
//     // Creating clock sets.
//     boost::dynamic_bitset<> x1(numOfClocks);
//     x1.set(1, true);
//     // Inserting clock sets in bounded deque.
//     bounded.push_back(x1);
//
//     // Creating clocks to partition.
//     boost::dynamic_bitset<> X(numOfClocks);
//     X.set(numOfClocks - 1, true);
//     X.set(numOfClocks - 2, true);
//     X.set(2, true);
//
//     // Setting up the region.
//     regToPerm.set_q(q);
//     regToPerm.set_h(h);
//     regToPerm.set_unbounded(unbounded);
//     regToPerm.set_x0(x0);
//     regToPerm.set_bounded(bounded);
//
//     std::cout << "The region in which we want to compute clock partitions is:" << std::endl;
//     std::cout << regToPerm.toString() << "\n";
//     std::cout << "The clocks to partition are: " << X << "\n";
//
//     // Creating the vector required for the regToPerm function.
//     auto clockValuation = regToPerm.getClockValuation();
//     std::vector<int> clockValues(numOfClocks);
//     for (int i = 0; i < numOfClocks; i++)
//         clockValues[i] = clockValuation[i].first;
//
//     // Creating a dummy notInX0 bitset.
//     boost::dynamic_bitset<> notInX0(numOfClocks);
//     // notInX0.set(numOfClocks - 1, true); // The first clock from the left now cannot be in x0.
//     // notInX0.set(numOfClocks - 2, true); // The second clock from the left now cannot be in x0.
//
//     const double t1 = omp_get_wtime();
//     std::vector<region::Region> regs = region::Region::permRegsBounded(q, clockValues, unbounded, x0, bounded, numOfClocks, X,
//                                                                        { maxConstant, maxConstant, maxConstant, maxConstant, maxConstant },
//                                                                        notInX0, boost::dynamic_bitset<>(numOfClocks));
//     const double t2 = omp_get_wtime();
//
//     std::cout << "\n";
//     std::cout << "Total time: " << t2 - t1 << std::endl;
//     std::cout << "\n";
//
//     // Printing resulting regions.
//     for (const auto &r: regs)
//         std::cout << r.toString() << std::endl;
//     std::cout << std::endl;
// }
//
//
// inline void testGetDiscretePredecessors()
// {
//     constexpr int numOfClocks = 2;
//
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "light_switch_predecessors_test.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "Parsed automaton: " << automaton << std::endl;
//
//     std::vector<transition> transitions = automaton.transitions;
//     std::unordered_map<std::string, int> clockIndices = automaton.getClocksIndices();
//     std::unordered_map<std::string, int> locationsAsIntMap = automaton.mapLocationsToInt();
//
//     region::Region initialRegion(numOfClocks, {});
//
//     std::cout << initialRegion.toString() << std::endl;
//
//     absl::flat_hash_map<std::string, int> values{};
//
//     std::vector<region::Region> regs = initialRegion.getImmediateDiscretePredecessors(transitions, clockIndices, locationsAsIntMap,
//                                                                                       automaton.getMaxConstants(clockIndices));
//
//     std::cout << "Printing regions: " << std::endl;
//     for (const auto &region: regs)
//         std::cout << region.toString() << std::endl;
// }
//
//
// inline void testInTransitions()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     // const std::string automatonFileName = "light_switch.txt";
//
//     const std::string automatonFileName = "ta0.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     const std::unordered_map<std::string, int> locationsToInt = automaton.mapLocationsToInt();
//
//     std::vector<std::vector<transition>> transitions = automaton.getInTransitions(locationsToInt);
//
//     std::cout << "locationsToInt contents:\n";
//     for (const auto &[name, index]: locationsToInt)
//     {
//         std::cout << "  " << name << " -> " << index << "\n";
//     }
//
//     for (int i = 0; i < static_cast<int>(transitions.size()); i++)
//     {
//         std::cout << "Location " << i << std::endl;
//         for (const auto &transition: transitions[i])
//             std::cout << transition << std::endl;
//     }
// }
//
//
// inline void testDiscretePredecessorsLightSwitch()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "light_switch.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     const std::unordered_map<std::string, int> locationsToInt = automaton.mapLocationsToInt();
//
//     std::cout << "locationsToInt contents:\n";
//     for (const auto &[name, index]: locationsToInt)
//     {
//         std::cout << "  " << name << " -> " << index << "\n";
//     }
//
//     constexpr int numOfClocks = 1;
//
//     region::Region startingRegion(numOfClocks, {});
//     startingRegion.set_q(1);
//
//     const auto h = static_cast<int *>(malloc(sizeof(int) * numOfClocks));
//     h[0] = 2;
//     startingRegion.set_h(h);
//
//     boost::dynamic_bitset<> x0(numOfClocks);
//     startingRegion.set_x0(x0);
//
//     boost::dynamic_bitset<> unboundedClock(numOfClocks);
//     unboundedClock.set(0, true);
//     std::deque<boost::dynamic_bitset<>> unbounded;
//     unbounded.push_back(unboundedClock);
//
//     startingRegion.set_unbounded(unbounded);
//
//     std::cout << startingRegion.toString() << std::endl;
//
//     region::RTS regionTransitionSystem(automaton);
//
//     std::vector<region::Region> regs{};
//     regs.push_back(startingRegion);
//     std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(regs, DFS);
//
//     std::cout << "Predecessors contents:\n";
//     for (const auto &region: predecessors)
//         std::cout << region.toString() << std::endl;
// }
//
//
// inline void testFlowerBackwards()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     // const std::string automatonFileName = "light_switch.txt";
//
//     const std::string automatonFileName = "test_flower_tiny.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     const region::RTS regionTransitionSystem(automaton);
//
//     std::cout << "\n\n";
//
//     const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(0, DFS);
//
//     std::cout << "Computed the following regions:" << std::endl;
//     for (const auto &region: rts)
//         std::cout << region.toString() << std::endl;
//
//     std::cout << "\n\n";
//
//     const std::vector<region::Region> predecessors = regionTransitionSystem.backwardReachability(rts, DFS);
//
//     std::cout << "Predecessors contents:\n";
//     for (const auto &region: predecessors)
//         std::cout << region.toString() << std::endl;
// }
//
//
// inline void testNetworkActions()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//
//     const std::string automatonFileName = "light_switch.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\n\n\n";
//     std::cout << "Parsed automaton: " << automaton << std::endl;
//
//     std::cout << "\nActions:" << std::endl;
//     for (const auto &action: automaton.actions)
//         std::cout << action << std::endl;
//
//     std::cout << "\nNow printing the actions in transitions:" << std::endl;
//     for (const auto &transition: automaton.transitions)
//     {
//         std::cout << transition << std::endl;
//         std::cout << "Action first element: " << transition.action.first << std::endl;
//         std::cout << "Action second element: " <<
//                 (transition.action.second.has_value() ? in_out_act_to_string(transition.action.second.value()) : "NO_VALUE") << std::endl;
//         std::cout << std::endl;
//     }
// }
//
//
// inline void testParseMultipleAutomata()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";
//
//     const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
//
//     for (const auto &automaton: automata)
//         std::cout << "\nParsed the following automaton:\n" << automaton << std::endl;
//
//     const networkOfTA::RTSNetwork net(automata);
//
//     std::cout << "\n\n\n\n\n";
//     std::cout << "Network:" << std::endl;
//     std::cout << net.toString() << std::endl;
// }
//
//
// inline void testGetImmediateNetworkDelaySuccessor()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";
//     const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
//     const networkOfTA::RTSNetwork net(automata);
//
//     std::cout << "\n\n\n\n\n";
//     std::cout << "Network:" << std::endl;
//     std::cout << net.toString() << std::endl;
//
//     const auto &initialRegions = net.getInitialRegions();
//
//     std::cout << "\n\n\n\n\n\n";
//     std::cout << "Starting from the following Network Regions:\n" << std::endl;
//     for (const auto &region: initialRegions)
//         std::cout << region.toString() << std::endl;
//
//     std::cout << "\n\n\n";
//
//     networkOfTA::NetworkRegion current = initialRegions[0];
//     for (int i = 1; i <= 40; i++)
//     {
//         current = current.getImmediateDelaySuccessor(net.getMaxConstants());
//         std::cout << "Successor " << i << ":\n" << current.toString() << std::endl;
//     }
// }
//
//
// inline void testNetworkBuildRegionGraphForeword()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/TA_semantics_algorithms_tools";
//     const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
//     const networkOfTA::RTSNetwork net(automata);
//
//     std::cout << net.toString() << std::endl;
//
//     // Locations:
//     // Automaton [0] (3 locations): {dim -> 2, bright -> 1, off -> 0}
//     // Automaton [1] (4 locations): {relax -> 3, t -> 2, study -> 1, idle -> 0}
//
//     const std::vector<std::optional<int>> goalLocations = { 1, 3 };
//
//     // Reachable.
//     // const std::vector goalLocations = { 1, 1 };
//
//     // Unreachable (the entire region graph will be computed).
//     // const std::vector goalLocations = { 1, 3 };
//
//     const auto res = net.forwardReachability(goalLocations, DFS);
// }
//
//
// inline void testNetworkTrainGateController()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/baier_train_gate_controller";
//     const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
//     const networkOfTA::RTSNetwork net(automata);
//
//     std::cout << net.toString() << std::endl;
//
//     // Locations:
//     // Automaton [0] (3 locations): {in -> 2, near -> 1, far -> 0}
//     // Automaton [1] (4 locations): {3 -> 3, 2 -> 2, 1 -> 1, 0 -> 0}
//     // Automaton [2] (4 locations): {going_up -> 3, down -> 2, coming_down -> 1, up -> 0}
//
//     const std::vector<std::optional<int>> goalLocations = { std::nullopt, std::nullopt, 1 };
//
//     const auto res = net.forwardReachability(goalLocations, DFS);
//
//     std::cout << res[0].toString() << std::endl;
// }
//
//
// inline void testParseInvariants()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//
//     const std::string automatonFileName = "light_switch.txt";
//
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     std::cout << "\n\n\n\n\n";
//     std::cout << "Parsed automaton: " << automaton << std::endl;
// }
//
//
// inline void testMultipleMaxConstants()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "test_flower_small.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     const auto &clocksIndices = automaton.getClocksIndices();
//     const auto &maxConstants = automaton.getMaxConstants(clocksIndices);
//
//     std::cout << "Clock indices:" << std::endl;
//     for (const auto &[fst, snd]: clocksIndices)
//         std::cout << fst << " " << snd + 1 << std::endl;
//
//     std::cout << "Maximum constants:" << std::endl;
//     for (int i = 0; i < static_cast<int>(maxConstants.size()); i++)
//         std::cout << i + 1 << " " << maxConstants[i] << std::endl;
// }
//
//
// inline void testExpression()
// {
//     using namespace expr::ast;
//
//     // --- Manual testing --- //
//
//     // Build a small expression by hand: L = L + 1 * (3 + N).
//     assignmentExpr a;
//     a.lhs.name = "L";
//     a.rhs = binaryExpr{
//         variable{ "L" },
//         ADD,
//         binaryExpr{
//             1,
//             MUL,
//             binaryExpr{ 3, ADD, variable{ "N" } }
//         }
//     };
//
//     std::cout << a.to_string() << "\n";
//
//     // -- Parse testing --- //
//
//     // Parse an expression from a file.
//     std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/expressions/simple_assignment.txt";
//     std::stringstream out;
//     const std::string source = readFromFile(path);
//
//     using parser::iterator_type;
//     iterator_type iter(source.begin());
//     iterator_type const end(source.end());
//
//     assignmentExpr ass;
//
//     // Our error handler.
//     using boost::spirit::x3::with;
//     using parser::error_handler_type;
//     using parser::error_handler_tag;
//     error_handler_type error_handler(iter, end, out, path);
//
//     // Our parser.
//     // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
//     auto const parser = with<error_handler_tag>(std::ref(error_handler))[expr::assignmentExpr()];
//
//     // Now we parse.
//     using boost::spirit::x3::ascii::space;
//     // ReSharper disable once CppTooWideScope
//     bool success = phrase_parse(iter, end, parser, space, ass);
//
//     if (success)
//     {
//         if (iter != end)
//             error_handler(iter, "Error! Expecting end of input here: ");
//         else
//             std::cout << "SUCCESSFUL parsing" << std::endl;
//     } else
//         std::cerr << "Wrong parsing" << std::endl;
//
//     std::cout << ass.to_string() << std::endl;
//
//     // --- Evaluation testing --- //
//
//     // Variable context with initial values
//     absl::btree_map<std::string, int> variables = {
//         { "L", 0 },
//         { "A", 10 },
//         { "B", 2 }
//     };
//
//     std::cout << "\nL value before: " << variables["L"] << std::endl;
//
//     const auto start = std::chrono::high_resolution_clock::now();
//
//     ass.evaluate(variables);
//
//     const auto end_time = std::chrono::high_resolution_clock::now();
//     const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start);
//     std::cout << "Evaluation took: " << duration.count() << " microseconds" << std::endl;
//
//     std::cout << "Evaluated with result: " << variables["L"] << std::endl;
//
//     std::cout << "L value after: " << variables["L"] << std::endl;
// }
//
//
// inline void testBooleanExpression()
// {
//     using namespace expr::ast;
//
//     // Parse an expression from a file.
//     std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/expressions/simple_boolean.txt";
//     std::stringstream out;
//     const std::string source = readFromFile(path);
//
//     using parser::iterator_type;
//     iterator_type iter(source.begin());
//     iterator_type const end(source.end());
//
//     booleanExpr boolE;
//
//     // Our error handler.
//     using boost::spirit::x3::with;
//     using parser::error_handler_type;
//     using parser::error_handler_tag;
//     error_handler_type error_handler(iter, end, out, path);
//
//     // Our parser.
//     // We pass our error handler to the parser so we can access it later on in our on_error and on_success handlers.
//     auto const parser = with<error_handler_tag>(std::ref(error_handler))[expr::booleanExpr()];
//
//     // Now we parse.
//     using boost::spirit::x3::ascii::space;
//     // ReSharper disable once CppTooWideScope
//     bool success = phrase_parse(iter, end, parser, space, boolE);
//
//     if (success)
//     {
//         if (iter != end)
//             error_handler(iter, "Error! Expecting end of input here: ");
//         else
//             std::cout << "SUCCESSFUL parsing" << std::endl;
//     } else
//         std::cerr << "Wrong parsing" << std::endl;
//
//     std::cout << boolE << std::endl;
//
//     // --- Evaluation testing --- //
//
//     // Variable context with initial values
//     absl::btree_map<std::string, int> variables = {
//         { "L", 0 },
//         { "A", 10 },
//         { "B", 9 }
//     };
//
//     const auto start = std::chrono::high_resolution_clock::now();
//
//     bool result = boolE.evaluate(variables);
//
//     const auto end_time = std::chrono::high_resolution_clock::now();
//     const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start);
//     std::cout << "Evaluation took: " << duration.count() << " microseconds" << std::endl;
//
//     std::cout << "Evaluated with result: " << result << std::endl;
// }
//
//
// inline void testIntegers()
// {
//     // const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/tmp.txt";
//     // const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path);
//
//     // std::cout << "\n\n" << automaton << std::endl;
// }
//
//
// inline void testNoClocksSingleAutomata()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/timed-automata-definitions/";
//     const std::string automatonFileName = "torch_no_clocks.txt";
//     const timed_automaton::ast::timedAutomaton automaton = parseTimedAutomaton(path + automatonFileName);
//
//     const region::RTS regionTransitionSystem(automaton);
//
//     std::cout << regionTransitionSystem.to_string() << std::endl;
//
//     std::cout << "\n\n\n";
//
//     std::cout << "Computed the following regions:" << std::endl;
//
//     const std::vector<region::Region> rts = regionTransitionSystem.forwardReachability(7, DFS);
// }
//
//
// inline void testNoClocksNetwork()
// {
//     const std::string path = "/Users/echo/Desktop/PhD/Tools/TARZAN/TARZAN/examples/networks_of_TA/test_no_clocks";
//     const std::vector<timed_automaton::ast::timedAutomaton> automata = parseTimedAutomataFromFolder(path);
//     const networkOfTA::RTSNetwork net(automata);
//
//     std::cout << net.toString() << std::endl;
//
//     const std::vector<std::optional<int>> goalLocations = { 3, std::nullopt };
//
//     const auto res = net.forwardReachability(goalLocations, DFS);
// }
//
// #endif //TARZAN_SUCCESSORSANDPREDECESSORSTESTING_H
