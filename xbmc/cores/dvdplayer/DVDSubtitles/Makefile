INCLUDES=-I. -I../ -I../../../ -I../../../.. -I../../../linux -I../../../../guilib -I../../../FileSystem/ -I../DVDCodecs/Overlay/ -I../Codecs -I../Codecs/ffmpeg

CXXFLAGS+= -D__STDC_CONSTANT_MACROS

SRCS=	DVDFactorySubtitle.cpp \
	DVDSubtitleLineCollection.cpp \
	DVDSubtitleParserSubrip.cpp \
	DVDSubtitleParserVplayer.cpp \
	DVDSubtitleStream.cpp \
	DVDSubtitleParserMicroDVD.cpp \
	DVDSubtitleParserMPL2.cpp \
	DVDSubtitleParserSami.cpp \
	DVDSubtitlesLibass.cpp \
	DVDSubtitleParserSSA.cpp \
	SamiTagConvertor.cpp \
					
LIB=	DVDSubtitles.a

include ../../../../Makefile.include
-include $(patsubst %.cpp,%.P,$(patsubst %.c,%.P,$(SRCS)))

