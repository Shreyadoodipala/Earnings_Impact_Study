CXX = g++
CXXFLAGS = -I./include -g -Wall -fopenmp
LDFLAGS = -fopenmp 
LIBS = -lcurl
BUILD_DIR = build
TARGET = $(BUILD_DIR)/earnings_impact_study

SRCS = src/ui_visualization/Menu.cpp \
       src/common/utils.cpp \
       src/common/Matrix.cpp \
       src/core/Stock.cpp \
       src/core/Sector.cpp \
       src/data/DataLoader.cpp \
       src/data/StockHistory.cpp \
       src/analysis/Calculation.cpp \
       src/ui_visualization/Gnuplot.cpp

OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
