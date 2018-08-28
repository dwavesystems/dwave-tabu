from __future__ import print_function

from tabu.tabu_search import TabuSearch

# creating the problem
q = [[-1, 2, 1], [2, -3, -4.5], [1, -4.5, 3.25]]
init_solution = [0, 0, 1]
tenure = 1
scale_factor = 4
timeout = 100  # millisecond

# Running the solver
r = TabuSearch(q, init_solution, tenure, scale_factor, timeout)

# Printing the results
print("qubo =", q)
print("best energy =", r.bestEnergy())
print("best sample =", r.bestSolution())
