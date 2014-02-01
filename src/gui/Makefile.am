include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(STD_CXXFLAGS) $(WARN_CXXFLAGS)

AM_CPPFLAGS += -I$(srcdir)/.. -isystem $(top_srcdir)/contrib

noinst_LIBRARIES = libgui.a
noinst_HEADERS = \
	Gui.h \
	GuiAdjustment.h \
	GuiBox.h \
	GuiButton.h \
	GuiContainer.h \
	GuiEvents.h \
	GuiFixed.h \
	GuiISelectable.h \
	GuiImage.h \
	GuiImageButton.h \
	GuiImageRadioButton.h \
	GuiLabel.h \
	GuiLabelSet.h \
	GuiMeterBar.h \
	GuiMultiStateImageButton.h \
	GuiRadioButton.h \
	GuiRadioGroup.h \
	GuiScreen.h \
	GuiStack.h \
	GuiTabbed.h \
	GuiTextEntry.h \
	GuiTextLayout.h \
	GuiTexturedQuad.h \
	GuiToggleButton.h \
	GuiToolTip.h \
	GuiVScrollBar.h \
	GuiVScrollPortal.h \
	GuiWidget.h

libgui_a_SOURCES = \
	Gui.cpp \
	GuiBox.cpp \
	GuiButton.cpp \
	GuiContainer.cpp \
	GuiFixed.cpp \
	GuiImage.cpp \
	GuiImageButton.cpp \
	GuiImageRadioButton.cpp \
	GuiLabel.cpp \
	GuiLabelSet.cpp \
	GuiMeterBar.cpp \
	GuiMultiStateImageButton.cpp \
	GuiRadioButton.cpp \
	GuiRadioGroup.cpp \
	GuiScreen.cpp \
	GuiStack.cpp \
	GuiTabbed.cpp \
	GuiTextEntry.cpp \
	GuiTextLayout.cpp \
	GuiTexturedQuad.cpp \
	GuiToggleButton.cpp \
	GuiToolTip.cpp \
	GuiVScrollBar.cpp \
	GuiVScrollPortal.cpp \
	GuiWidget.cpp
