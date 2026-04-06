CXX = g++
CXXFLAGS = -std=c++23 -Wall -g 


clean:
	rm -f tasksched

tasksched: 
	make clean
	$(CXX) $(CXXFLAGS) -o tasksched Task.cpp PeriodicTask.cpp AperiodicTask.cpp SporadicTask.cpp main.cpp


run:
	make tasksched
	./tasksched