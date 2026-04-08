CXX = g++
CXXFLAGS = -std=c++23 -Wall -g -Itasks -Ischeduler

SRCS = tasks/Task.cpp tasks/PeriodicTask.cpp tasks/AperiodicTask.cpp \
       tasks/SporadicTask.cpp scheduler/Scheduler.cpp main.cpp

tasksched:
	$(CXX) $(CXXFLAGS) -o tasksched $(SRCS)

clean:
	rm -f tasksched

run:
	make tasksched
	./tasksched

.PHONY: clean run