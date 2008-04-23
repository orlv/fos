#----------------------------------------------------------------------------#
#	Copyright (C) 2007 Oleg Fedorov
#----------------------------------------------------------------------------#

include $(ROOT)/config.mk

INSTALLDIR	=	$(LIB)
OUTPUT		=	libfreetype.a

OBJECTS		=	autofit/autofit.o

OBJECTS		+=	base/ftapi.o	base/ftcalc.o	base/ftdbgmem.o	\
			base/ftgloadr.o	base/ftnames.o	base/ftobjs.o	\
			base/ftoutln.o	base/ftrfork.o	base/ftstream.o	\
			base/fttrigon.o	base/ftutil.o	base/ftbbox.o	\
			base/ftbdf.o	base/ftbitmap.o	base/ftdebug.o	\
			base/ftgasp.o	base/ftglyph.o	base/ftgxval.o	\
			base/ftinit.o	base/ftlcdfil.o	base/ftmm.o	\
			base/ftotval.o	base/ftpatent.o	base/ftpfr.o	\
			base/ftstroke.o	base/ftsynth.o	base/ftsystem.o	\
			base/fttype1.o	base/ftwinfnt.o	base/ftxf86.o
OBJECTS		+=	bdf/bdf.o
OBJECTS		+=	cache/ftcmru.o		cache/ftcbasic.o	\
			cache/ftccache.o	cache/ftccmap.o		\
			cache/ftcglyph.o	cache/ftcimage.o	\
			cache/ftcmanag.o	cache/ftcsbits.o
OBJECTS		+=	cff/cffcmap.o	cff/cffdrivr.o	cff/cffgload.o	\
			cff/cffload.o	cff/cffobjs.o	cff/cffparse.o
OBJECTS		+=	cid/cidgload.o	cid/cidload.o	cid/cidobjs.o	\
			cid/cidparse.o	cid/cidriver.o	cid/type1cid.o
OBJECTS		+=	gxvalid/gxvalid.o
OBJECTS		+=	gzip/ftgzip.o
OBJECTS		+=	lzw/ftlzw.o
OBJECTS		+=	otvalid/otvalid.o
OBJECTS		+=	pcf/pcf.o
OBJECTS		+=	pfr/pfr.o
OBJECTS		+=	psaux/afmparse.o	psaux/psauxmod.o \
			psaux/psconv.o		psaux/psobjs.o	\
			psaux/t1cmap.o		psaux/t1decode.o
OBJECTS		+=	pshinter/pshalgo.o	pshinter/pshglob.o	\
			pshinter/pshmod.o	pshinter/pshrec.o
OBJECTS		+=	psnames/psmodule.o
OBJECTS		+=	raster/ftraster.o	raster/ftrend1.o
OBJECTS		+=	sfnt/sfdriver.o	sfnt/sfobjs.o	sfnt/ttbdf.o	\
			sfnt/ttcmap.o	sfnt/ttkern.o	sfnt/ttload.o	\
			sfnt/ttmtx.o	sfnt/ttpost.o	sfnt/ttsbit.o
OBJECTS		+=	smooth/ftgrays.o	smooth/ftsmooth.o
OBJECTS		+=	truetype/ttdriver.o	truetype/ttgload.o	\
			truetype/ttgxvar.o	truetype/ttinterp.o	\
			truetype/ttobjs.o	truetype/ttpload.o
OBJECTS		+=	type1/t1afm.o	type1/t1driver.o	type1/t1gload.o	\
			type1/t1load.o	type1/t1objs.o		type1/t1parse.o
OBJECTS		+=	type42/t42drivr.o	type42/t42objs.o	\
			type42/t42parse.o
OBJECTS		+=	winfonts/winfnt.o

CFLAGS		+=	-DFT2_BUILD_LIBRARY

include $(SYSTEM_SRC)/mk/trg_lib.mk
