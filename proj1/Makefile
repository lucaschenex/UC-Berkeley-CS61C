# Build file for CS61C Lab 2
# You should not need to edit this file.

# This file requires GNU make and depends on paths on instruction machines.

####

## Variables

# Source files (java code). wildcard selects all files matching a pattern.
SOURCES = $(wildcard *.java)
# Output JAR file
TARGET = sw.jar
# Extra JARs to have on the classpath when compiling.
CLASSPATH = /home/ff/cs61c/hadoop/hadoop-core.jar:/home/ff/cs61c/hadoop/lib/commons-cli.jar
# javac command to use
JAVAC = javac -g -deprecation -cp $(CLASSPATH)
# jar command to use
JAR = jar

## Make targets

# General form is target: dependencies (targets or files), followed by
# commands to run to build the target from the dependencies.

# Default target.
all: $(TARGET)

$(TARGET): classes $(SOURCES)
	$(JAVAC) -d classes $(SOURCES)
	$(JAR) cf $(TARGET) -C classes .

classes:
	mkdir classes

clean:
	rm -rf classes $(TARGET) *-out

# test with the ring graph 4 vertices
test-ring: clean all
	hadoop jar sw.jar SmallWorld ~cs61c/p1data/ring4.seq final-out 1
	cat final-out/part*

# test with the hep graph ~35000 vertices
test-hep: clean all
	hadoop jar sw.jar SmallWorld ~cs61c/p1data/cit-HepPh.sequ final-out 9876
	cat final-out/part*

.PHONY: clean all test-ring test-hep
