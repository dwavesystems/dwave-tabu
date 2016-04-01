# Running tabu search example
import tabu_solver

# creating the problem
q = [[-1, 2, 1], [2, -3, -4.5], [1, -4.5, 3.25]]
init_solution = [0, 0, 1]
tenure = 1
scale_factor = 4
timeout = 100  # millisecond

# Running the solver
r = tabu_solver.TabuSearch(q, init_solution, tenure, scale_factor, timeout)

# Printing the results
print "q = "
for i in q:
    print i
print "best energy = ", r.bestEnergy()
print "best state  = " , r.bestSolution()

