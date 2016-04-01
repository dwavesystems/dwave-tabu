all:
	swig -python -c++ tabu_solver.i
	g++ --shared -O3 tabu_solver_wrap.cxx tabu.cc bqpSolver.cpp bqpUtil.cpp -o _tabu_solver.so -I/usr/include/python2.7 -fPIC -lpython
