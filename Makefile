CXX = g++
CXXFLAGS = -std=c++23 -Wall -g -Itasks -Ischeduler -Ischeduler/stats

SRCS = tasks/Task.cpp tasks/PeriodicTask.cpp tasks/AperiodicTask.cpp \
       tasks/SporadicTask.cpp \
       scheduler/Scheduler.cpp \
       scheduler/stats/TaskStats.cpp scheduler/stats/Stats.cpp \
       main.cpp

tasksched:
	$(CXX) $(CXXFLAGS) -o tasksched $(SRCS)

clean:
	rm -f tasksched

run:
	make tasksched
	./tasksched

.PHONY: clean run	