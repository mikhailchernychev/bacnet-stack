TOP := $(shell realpath ../..)
export TOP

target = libbacnet.so
shared_library=libbacnet

bacnet := .



CFLAGS += -g -I $(bacnet)/include -I $(bacnet)/ports/linux -I $(bacnet)/demo/object \
          -DPRINT_ENABLED=1 -DBACAPP_ALL -DBACDL_BIP=1 -DBBMD_ENABLED=1 -DMAX_ADDRESS_CACHE=1024  \
          -DBACNET_VENDOR_NAME="\"Altair SmartEdge IoT Server(tm)\"" -DBACNET_VENDOR_ID=894

libfiles := $(filter-out $(bacnet)/src/mstp.c, $(wildcard $(bacnet)/src/*.c))
libfiles := $(filter-out $(bacnet)/src/datalink.c, $(libfiles))
libfiles := $(filter-out $(bacnet)/src/ucix.c, $(libfiles))
libfiles := $(filter-out $(bacnet)/src/bacsec.c, $(libfiles))

libfiles += $(bacnet)/demo/object/candi-device-client.c $(bacnet)/demo/object/netport.c

libfiles += $(bacnet)/demo/handler/h_whois.c   $(bacnet)/demo/handler/h_iam.c \
            $(bacnet)/demo/handler/noserv.c    $(bacnet)/demo/handler/txbuf.c \
            $(bacnet)/demo/handler/h_rp.c      $(bacnet)/demo/handler/s_iam.c \
            $(bacnet)/demo/handler/s_rp.c      $(bacnet)/demo/handler/s_wp.c  \
	    $(bacnet)/demo/handler/h_npdu.c    $(bacnet)/demo/handler/s_rpm.c  \
            $(bacnet)/demo/handler/dlenv.c     $(bacnet)/demo/handler/s_whois.c \
            $(bacnet)/demo/handler/s_router.c  $(bacnet)/demo/handler/s_cov.c \
            $(bacnet)/demo/handler/h_rpm.c     $(bacnet)/demo/handler/h_wp.c \
            $(bacnet)/demo/handler/h_wpm.c     $(bacnet)/demo/handler/h_rp_a.c \


libfiles += $(bacnet)/ports/linux/bip-init.c

subdirs := $(bacnet)/src/ $(bacnet)/demo/object/ $(bacnet)/demo/handler $(bacnet)/ports/linux

sources=$(libfiles)

include $(TOP)/includes/build.mk
CFLAGS := $(filter-out -Werror, $(CFLAGS))

includes:
	mkdir -p $(staging)/usr/include/bacnet/ports/linux
	cp -av $(bacnet)/include/*.h $(staging)/usr/include/bacnet
	cp -av $(bacnet)/demo/object/device.h $(staging)/usr/include/bacnet
	cp -av $(bacnet)/ports/linux/*.h $(staging)/usr/include/bacnet/ports/linux
