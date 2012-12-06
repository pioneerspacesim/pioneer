include $(top_srcdir)/Makefile.common

AM_CFLAGS += $(WARN_CFLAGS)
AM_CPPFLAGS += $(WARN_CPPFLAGS)
AM_CXXFLAGS += $(WARN_CXXFLAGS)

INCLUDES = -isystem $(top_srcdir)/contrib -I$(srcdir)/..
if !HAVE_LUA
INCLUDES += -isystem $(top_srcdir)/contrib/lua
endif

noinst_LIBRARIES = libui.a
noinst_HEADERS = \
	Align.h \
	Background.h \
	Box.h \
	Button.h \
	CellSpec.h \
	CheckBox.h \
	ColorBackground.h \
	Container.h \
	Context.h \
	DropDown.h \
	Event.h \
	EventDispatcher.h \
	Expand.h \
	FloatContainer.h \
	Gradient.h \
	Grid.h \
	Icon.h \
	Image.h \
	Label.h \
	List.h \
	Lua.h \
	LuaSignal.h \
	Margin.h \
	MultiLineText.h \
	Point.h \
	Scroller.h \
	Single.h \
	Skin.h \
	Slider.h \
	SmallButton.h \
	TextEntry.h \
	TextLayout.h \
	Widget.h \
	WidgetSet.h

libui_a_SOURCES = \
	Align.cpp \
	Background.cpp \
	Box.cpp \
	Button.cpp \
	CellSpec.cpp \
	CheckBox.cpp \
	ColorBackground.cpp \
	Container.cpp \
	Context.cpp \
	DropDown.cpp \
	Event.cpp \
	EventDispatcher.cpp \
	Expand.cpp \
	FloatContainer.cpp \
	Gradient.cpp \
	Grid.cpp \
	Icon.cpp \
	Image.cpp \
	Label.cpp \
	List.cpp \
	Margin.cpp \
	MultiLineText.cpp \
	Scroller.cpp \
	Single.cpp \
	Skin.cpp \
	Slider.cpp \
	SmallButton.cpp \
	TextEntry.cpp \
	TextLayout.cpp \
	Widget.cpp \
	Lua.cpp \
	LuaAlign.cpp \
	LuaBackground.cpp \
	LuaBox.cpp \
	LuaButton.cpp \
	LuaCheckBox.cpp \
	LuaColorBackground.cpp \
	LuaContainer.cpp \
	LuaContext.cpp \
	LuaDropDown.cpp \
	LuaGradient.cpp \
	LuaExpand.cpp \
	LuaGrid.cpp \
	LuaIcon.cpp \
	LuaImage.cpp \
	LuaLabel.cpp \
	LuaList.cpp \
	LuaMargin.cpp \
	LuaMultiLineText.cpp \
	LuaScroller.cpp \
	LuaSingle.cpp \
	LuaSlider.cpp \
	LuaSmallButton.cpp \
	LuaTextEntry.cpp \
	LuaWidget.cpp
