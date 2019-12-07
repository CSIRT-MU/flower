include /usr/local/etc/PcapPlusPlus.mk

CXX_FLAGS=-Wextra -Wall -pedantic -std=c++14 -O2 $(PCAPPP_BUILD_FLAGS)
CXX_LIBS=$(PCAPPP_LIBS)
CXX_LIBS_DIR=$(PCAPPP_LIBS_DIR)
CXX_INCLUDES=$(PCAPPP_INCLUDES)

all: flower

flower: src/main.cpp
	$(CXX) $(CXX_FLAGS) $(CXX_INCLUDES) $(CXX_LIBS_DIR) $< -o $@ $(CXX_LIBS)
