set(TARGETNAME "gui")

set( ${TARGETNAME}_H
	"Gui.h"
	"GuiAdjustment.h"
	"GuiBox.h"
	"GuiButton.h"
	"GuiContainer.h"
	"GuiEvents.h"
	"GuiFixed.h"
	"GuiGradient.h"
	"GuiImage.h"
	"GuiImageButton.h"
	"GuiImageRadioButton.h"
	"GuiISelectable.h"
	"GuiLabel.h"
	"GuiLabelSet.h"
	"GuiMeterBar.h"
	"GuiMultiStateImageButton.h"
	"GuiRadioButton.h"
	"GuiRadioGroup.h"
	"GuiRepeaterButton.h"
	"GuiScreen.h"
	"GuiStack.h"
	"GuiTabbed.h"
	"GuiTextEntry.h"
	"GuiTextLayout.h"
	"GuiTexturedQuad.h"
	"GuiToggleButton.h"
	"GuiToolTip.h"
	"GuiVScrollBar.h"
	"GuiVScrollPortal.h"
	"GuiWidget.h"
)
set( ${TARGETNAME}_SRC
	"Gui.cpp"
	"GuiBox.cpp"
	"GuiButton.cpp"
	"GuiContainer.cpp"
	"GuiFixed.cpp"
	"GuiGradient.cpp"
	"GuiImage.cpp"
	"GuiImageButton.cpp"
	"GuiImageRadioButton.cpp"
	"GuiLabel.cpp"
	"GuiLabelSet.cpp"
	"GuiMeterBar.cpp"
	"GuiMultiStateImageButton.cpp"
	"GuiRadioButton.cpp"
	"GuiRadioGroup.cpp"
	"GuiRepeaterButton.cpp"
	"GuiScreen.cpp"
	"GuiStack.cpp"
	"GuiTabbed.cpp"
	"GuiTextEntry.cpp"
	"GuiTextLayout.cpp"
	"GuiTexturedQuad.cpp"
	"GuiToggleButton.cpp"
	"GuiToolTip.cpp"
	"GuiVScrollBar.cpp"
	"GuiVScrollPortal.cpp"
	"GuiWidget.cpp"
)


# Setup dependencies
SETUP_DEFAULT_DEPENDENCIES_LIST(DEPENDENCIES_NAMES)

LINK_DIRS_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})
add_library("${TARGETNAME}" STATIC ${${TARGETNAME}_H} ${${TARGETNAME}_SRC})
LINK_WITH_VARIABLES(${TARGETNAME} ${DEPENDENCIES_NAMES})

MAKE_ENUM_DEPEND(${TARGETNAME})
#target_link_libraries(${TARGETNAME} LUA OOLUA)

# ADD_DEFINITIONS( 
	# -DBOOST_ENABLE_ASSERT_HANDLER
	# -DBOOST_ALL_DYN_LINK	# Tell boost to use dynamic libraries
	# -DBOOST_ALL_NO_LIB		# No boost auto link
# )

IF (${PROJECT_NAME}_USE_PRECOMPILED_HEADERS)
	SET_USE_PRECOMPILED_HEADER(${TARGETNAME} ${PCH_DEFAULT_H} ${PCH_DEFAULT_CPP})
ENDIF()
