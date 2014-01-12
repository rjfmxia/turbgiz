CFG ?= Debug
SYS ?= Linux
GRF ?= Glx
BUILDPATH ?= ../build

build :
	cd ../src ; make CFG=$(CFG) SYS=$(SYS) GRF=$(GRF) BUILDPATH=$(BUILDPATH) -f ../build/makeapp APP=TurbGiz

clean :
	cd ../src ; make clean CFG=$(CFG) SYS=$(SYS) GRF=$(GRF) BUILDPATH=$(BUILDPATH) -f ../build/makeapp APP=TurbGiz
