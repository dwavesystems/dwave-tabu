#ifndef __TABU_SEARCH_H__
#define __TABU_SEARCH_H__

#include <vector>
#include <map>
#include "bqpUtil.h"

class TabuSearch
{
    public:
        TabuSearch(std::vector<std::vector<double> > Q, std::vector<int> initSol, int tenure, int scaleFactor, long int timeout);
        double bestEnergy();
        std::vector<int> bestSolution();

    private:
        BQP bqp;
        int sf;
};


#endif //__TABU_SEARCH_H__