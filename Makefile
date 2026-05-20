CXX = g++
CXXFLAGS = -I./include -g -Wall
BUILD_DIR = build
TARGET = $(BUILD_DIR)/test.exe

SRCS = test.cpp \
       src/common/utils.cpp \
       src/common/Matrix.cpp \
       src/core/Stock.cpp \
       src/data/DataLoader.cpp

OBJS = $(addprefix $(BUILD_DIR)/,$(SRCS:.cpp=.o))

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
