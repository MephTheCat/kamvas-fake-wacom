COMMONFLAGS = -O2 -Wall -Wextra -I./include -g `pkg-config --cflags libusb-1.0 libevdev`

CC       = gcc
CXX      = g++
CFLAGS   = $(COMMONFLAGS) -std=gnu11
CXXFLAGS = $(COMMONFLAGS) -std=c++11
LIBS     = `pkg-config --libs libusb-1.0 libevdev`

SRCDIR = src
OBJDIR = build

TARGET = kamvasfakewacom

SRCS     = $(shell find $(SRCDIR) -name '*.cpp' -o -name '*.c')
OBJS_CPP = $(subst $(SRCDIR), $(OBJDIR), $(filter %.o, $(SRCS:.cpp=.cpp.o)))
OBJS_C   = $(subst $(SRCDIR), $(OBJDIR), $(filter %.o, $(SRCS:.c=.c.o)))

.PHONY: clean

$(TARGET): $(OBJS_C) $(OBJS_CPP)
	@echo "LD  " $@
	@$(CXX) -o $@ $^ $(LIBS)

$(OBJDIR)/%.cpp.o: $(SRCDIR)/%.cpp
	@echo "G++ " $< "=>" $@
	@mkdir -p $(shell dirname $@)
	@$(CXX) -c -o $@ $< $(CXXFLAGS)

$(OBJDIR)/%.c.o: $(SRCDIR)/%.c
	@echo "GCC " $< "=>" $@
	@mkdir -p $(shell dirname $@)
	@$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(shell find build -name '*.o')
	rm $(TARGET)

