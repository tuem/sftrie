# sftrie
# https://github.com/tuem/sftrie
#
# Copyright 2017 Takashi Uemura
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

BINS = set_cli map_cli

all: $(BINS)


CXX := g++
CXXFLAGS := -Wall -Wextra -O3 -std=c++2b -I../include

debug: CXXFLAGS += -DDEBUG -g
debug: all


SRCS = $(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRCS))
DEPS = $(patsubst %.cpp,%.d,$(SRCS))


set_cli: set_cli.o
	$(CXX) -o $@ $^

map_cli: map_cli.o
	$(CXX) -o $@ $^


clean:
	$(RM) $(OBJS) $(DEPS) $(BINS)


.PHONY: clean all


.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<

%.d: %.cpp
	@set -e; $(CXX) -MM $(CXXFLAGS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o $@ : /g' > $@; \
	[ -s $@ ] || rm -f $@
-include $(DEPS)
