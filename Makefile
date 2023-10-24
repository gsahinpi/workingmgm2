#------------------------------------------------------------------------
#
#   Company: Magmio a.s.
#
#
#   Project: Magmio
#
#
# -----------------------------------------------------------------------
#
#
#   (c) Copyright 2013-2023 Magmio a.s.
#   All rights reserved.
#
#   Please review the terms of the license agreement before using this
#   file. If you are not an authorized user, please destroy this
#   source code file and notify Magmio a.s. immediately
#   that you inadvertently received an unauthorized copy.
#
#------------------------------------------------------------------------

include $(MAGMIO_PATH)/Makefile.conf

#Disable threading
MAKEFLAGS :=

LIBS+=-lmagmio -lpthread -lpugixml

.PHONY: clean model all

all: model_app
model_app: model 

model: strategies.h $(MAGMIO_PATH)/model/model.a main.cpp hft_types.o user_strategy.o order_example.cpp strategy_prefilter_example.cpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp hft_types.o user_strategy.o $(MAGMIO_PATH)/model/model.a $(LIBS)

#strategy engine
hft_types.o: hft_types.h strategies.h
	$(CXX) $(CXXFLAGS) -I. -Wno-unused -Wno-unknown-pragmas -DMODEL -c -o $@ hft_types.cpp

user_strategy.o: hft_types.o strategies.h strategies.cpp
	$(CXX) $(CXXFLAGS) -I. -Wno-unused -Wno-unknown-pragmas -DMODEL -c -o $@ strategies.cpp

clean:
	rm -f model  user_strategy.o hft_types.o

