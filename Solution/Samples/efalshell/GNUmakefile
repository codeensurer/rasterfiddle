COMPONENT_ROOT = Ellis/EFAL/Samples/efalshell

APPS = efalshell

OBJ	=	\
	efalshell.o

include ../../../../GNUmake.opt

CXXFLAGS    += 

ELLIS_INCLUDE += -I$(ELLIS_SRC_ROOT)/Ellis/EFAL/Samples/EFAL/include

# Order matters and this seems to work
ARCHIVES := \
	-l EFAL
	
default:	lib

all:	lib

lib:	$(OBJS)  $(APPS)  $(EXPORTS)

clean: 
	$(RM) -rf $(OBJDIR)
	
.PHONY: all clean lib default 
