#include "common.h"
#include <limits>
#include "tabu_search.h"
#include "bqpSolver.h"

using std::vector;
using std::size_t;

TabuSearch::TabuSearch(vector<vector< double > > Q, vector<int> initSol, int tenure, int scaleFactor, long int timeout):
    sf(scaleFactor)
{
    size_t nvars = Q.size();
    for (int i = 0; i < nvars; i++){
        if (Q[i].size() != nvars)
            throw Exception("Q must be a symmetric square matrix");
    }
    if (scaleFactor < 0)
        throw Exception("scaleFactor must be a positive integer");

    if (initSol.size() != nvars)
        throw Exception("length of init_solution doesn't match the size of Q");

    if (tenure < 1 || tenure > (nvars - 1))
        throw Exception("tenure must be in the range [1, num_vars - 1]");

    bqp.evalNum = 0;
    bqp.iterNum = 0;
    bqp.nIterations = 0;
    bqp.nVars = nvars;
    bqp.restartNum = 0;
    bqp.Q.resize(nvars);
    bqp.upperBound = std::numeric_limits<long>::min();
    for (int i = 0; i < nvars; i++)
        bqp.Q[i].resize(nvars);

    for (int i = 0; i < nvars; i++){
        for (int j = i; j < nvars; j++){
            if (Q[i][j] != Q[j][i])
                throw Exception("Q must be symmetric");
            bqp.Q[i][j] = bqp.Q[j][i] = (long) (Q[i][j] * scaleFactor);
        }
    }
    bqp.solution.resize(nvars);
    bqp.solutionQuality = 0;
    bqpSolver_multiStartTabooSearch(&bqp, timeout, 1000000, tenure, initSol.data(), NULL);
}


double TabuSearch::bestEnergy()
{
    return (double)bqpUtil_getObjective(&bqp, bqp.solution.data()) / sf;
}

vector<int> TabuSearch::bestSolution()
{
    return bqp.solution;
}
