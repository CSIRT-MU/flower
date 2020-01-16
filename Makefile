include /usr/local/etc/PcapPlusPlus.mk

CXX_FLAGS=-Wextra -Wall -pedantic -std=c++17 -g -O2 $(PCAPPP_BUILD_FLAGS)
CXX_LIBS=$(PCAPPP_LIBS)
CXX_LIBS_DIR=$(PCAPPP_LIBS_DIR)
CXX_INCLUDES=$(PCAPPP_INCLUDES) -I include
CXX_HEADERS=$(wildcard include/*.hpp)

all: flower

flower: src/main.cpp $(CXX_HEADERS)
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) $(CXX_LIBS_DIR) $< -o $@ $(CXX_LIBS)

clean:
	rm flower
