XBE_TITLE = nxdk_tests
GEN_XISO = $(XBE_TITLE).iso
NXDK_DIR ?= $(CURDIR)/../nxdk
NXDK_CXX = y

DEBUG = y

SRCS = \
	$(wildcard $(CURDIR)/*.cpp) \
	$(wildcard $(CURDIR)/winapi/*.cpp)

CXXFLAGS += -Wall -Wextra -std=gnu++11
CFLAGS   += -std=gnu11

ifeq ($(DEBUG),y)
CFLAGS += -I$(CURDIR) -DDEBUG -D_DEBUG
CXXFLAGS += -I$(CURDIR) -DDEBUG -D_DEBUG
endif


include $(NXDK_DIR)/Makefile
