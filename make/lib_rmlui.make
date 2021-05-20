MKFILE      := $(lastword $(MAKEFILE_LIST)) 

include make/platform.make
include make/configure.make
include make/platform_os.make

TARGET		 := libRmlCore

CPPSOURCES := $(MOUNT_DIR)/RmlUI/Source \
              $(MOUNT_DIR)/RmlUI/Source/Core \
						  $(MOUNT_DIR)/RmlUI/Source/Core/Elements \
							$(MOUNT_DIR)/RmlUI/Source/Core/FontEngineDefault
INCLUDES   := /Library/Frameworks/Mono.framework/Headers \
						  /usr/local/include/freetype2

#SYSROOT    := $(shell xcrun --show-sdk-path)
SYSROOT    := /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.15.sdk

LIBS = /usr/local/lib/libfreetype.dylib \
         $(FREETYPE_LIBS)

CPPFILES   := $(foreach dir,$(CPPSOURCES), $(wildcard $(dir)/*.cpp))
OBJS       := $(CPPFILES:.cpp=.o)
Q3OBJ      := $(addprefix $(B)/rmlui/,$(notdir $(OBJS)))

export INCLUDE	:= $(foreach dir,$(INCLUDES),-I$(dir))

CXX      = /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++
# -fsigned-char \
             -O2 -ftree-vectorize -g -ffast-math -fno-short-enums
CFLAGS   = $(INCLUDE) \
						 -DRMLUI_NO_THIRDPARTY_CONTAINERS \
						 -DBUILD_FREETYPE $(FREETYPE_CFLAGS) \
						 -DRmlCore_EXPORTS \
						 -isysroot $(SYSROOT) \
						 -MMD \
						 -DRMLUI_NO_THIRDPARTY_CONTAINERS
CXXFLAGS  = $(CFLAGS)  -std=c++14

define DO_RMLUI_CXX
	$(echo_cmd) "RMLUI_CC $<"
	$(Q)$(CXX) $(SHLIBCFLAGS) $(CXXFLAGS) -o $@ -c $<
endef

$(B)/rmlui/%.o: code/RmlUi/Source/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/Elements/%.cpp
	$(DO_RMLUI_CXX)

$(B)/rmlui/%.o: code/RmlUi/Source/Core/FontEngineDefault/%.cpp
	$(DO_RMLUI_CXX)

$(B)/$(TARGET)$(SHLIBNAME): $(Q3OBJ) 
	$(echo_cmd) "LD $@"
	$(Q)$(CXX) $(CFLAGS) $^ $(LIBS) $(SHLIBLDFLAGS) -o $@

mkdirs:
	@if [ ! -d $(BUILD_DIR) ];then $(MKDIR) $(BUILD_DIR);fi
	@if [ ! -d $(B) ];then $(MKDIR) $(B);fi
	@if [ ! -d $(B)/rmlui ];then $(MKDIR) $(B)/rmlui;fi

default:
	$(MAKE) -f $(MKFILE) B=$(BD) mkdirs
	$(MAKE) -f $(MKFILE) B=$(BD) $(BD)/$(TARGET)$(SHLIBNAME)

clean:
	@rm -rf $(BD)/rmlui
	@rm -rf $(BD)/$(TARGET)$(SHLIBNAME)
	@rm -rf $(BR)/rmlui
	@rm -rf $(BR)/$(TARGET)$(SHLIBNAME)

#############################################################################
# DEPENDENCIES
#############################################################################

ifdef B
D_FILES=$(shell find $(B)/rmlui -name '*.d')
endif

ifneq ($(strip $(D_FILES)),)
include $(D_FILES)
endif

.PHONY: all clean clean2 clean-debug clean-release copyfiles \
	debug default dist distclean makedirs release \
  targets tools toolsclean mkdirs \
	$(D_FILES)

.DEFAULT_GOAL := default

# OBJS = Source/Core/BaseXMLParser.o \
Source/Core/Box.o \
Source/Core/Clock.o \
Source/Core/ComputeProperty.o \
Source/Core/Context.o \
Source/Core/ContextInstancer.o \
Source/Core/ContextInstancerDefault.o \
Source/Core/ConvolutionFilter.o \
Source/Core/Core.o \
Source/Core/DataController.o \
Source/Core/DataControllerDefault.o \
Source/Core/DataExpression.o \
Source/Core/DataModel.o \
Source/Core/DataModelHandle.o \
Source/Core/DataTypeRegister.o \
Source/Core/DataVariable.o \
Source/Core/DataView.o \
Source/Core/DataViewDefault.o \
Source/Core/Decorator.o \
Source/Core/DecoratorGradient.o \
Source/Core/DecoratorInstancer.o \
Source/Core/DecoratorNinePatch.o \
Source/Core/DecoratorTiled.o \
Source/Core/DecoratorTiledBox.o \
Source/Core/DecoratorTiledBoxInstancer.o \
Source/Core/DecoratorTiledHorizontal.o \
Source/Core/DecoratorTiledHorizontalInstancer.o \
Source/Core/DecoratorTiledImage.o \
Source/Core/DecoratorTiledImageInstancer.o \
Source/Core/DecoratorTiledInstancer.o \
Source/Core/DecoratorTiledVertical.o \
Source/Core/DecoratorTiledVerticalInstancer.o \
Source/Core/DocumentHeader.o \
Source/Core/Element.o \
Source/Core/ElementAnimation.o \
Source/Core/ElementBackgroundBorder.o \
Source/Core/ElementDecoration.o \
Source/Core/ElementDefinition.o \
Source/Core/ElementDocument.o \
Source/Core/ElementHandle.o \
Source/Core/ElementInstancer.o \
Source/Core/Elements/DataFormatter.o \
Source/Core/Elements/DataQuery.o \
Source/Core/Elements/DataSource.o \
Source/Core/Elements/DataSourceListener.o \
Source/Core/Elements/ElementDataGrid.o \
Source/Core/Elements/ElementDataGridCell.o \
Source/Core/Elements/ElementDataGridExpandButton.o \
Source/Core/Elements/ElementDataGridRow.o \
Source/Core/Elements/ElementForm.o \
Source/Core/Elements/ElementFormControl.o \
Source/Core/Elements/ElementFormControlDataSelect.o \
Source/Core/Elements/ElementFormControlInput.o \
Source/Core/Elements/ElementFormControlSelect.o \
Source/Core/Elements/ElementFormControlTextArea.o \
Source/Core/Elements/ElementImage.o \
Source/Core/Elements/ElementLabel.o \
Source/Core/Elements/ElementProgressBar.o \
Source/Core/Elements/ElementTabSet.o \
Source/Core/Elements/ElementTextSelection.o \
Source/Core/Elements/InputType.o \
Source/Core/Elements/InputTypeButton.o \
Source/Core/Elements/InputTypeCheckbox.o \
Source/Core/Elements/InputTypeRadio.o \
Source/Core/Elements/InputTypeRange.o \
Source/Core/Elements/InputTypeSubmit.o \
Source/Core/Elements/InputTypeText.o \
Source/Core/Elements/SelectOption.o \
Source/Core/Elements/WidgetDropDown.o \
Source/Core/Elements/WidgetSlider.o \
Source/Core/Elements/WidgetTextInput.o \
Source/Core/Elements/WidgetTextInputMultiLine.o \
Source/Core/Elements/WidgetTextInputSingleLine.o \
Source/Core/Elements/WidgetTextInputSingleLinePassword.o \
Source/Core/Elements/XMLNodeHandlerDataGrid.o \
Source/Core/Elements/XMLNodeHandlerTabSet.o \
Source/Core/Elements/XMLNodeHandlerTextArea.o \
Source/Core/ElementScroll.o \
Source/Core/ElementStyle.o \
Source/Core/ElementText.o \
Source/Core/ElementUtilities.o \
Source/Core/Event.o \
Source/Core/EventDispatcher.o \
Source/Core/EventInstancer.o \
Source/Core/EventInstancerDefault.o \
Source/Core/EventListenerInstancer.o \
Source/Core/EventSpecification.o \
Source/Core/Factory.o \
Source/Core/FileInterface.o \
Source/Core/FileInterfaceDefault.o \
Source/Core/FontEffect.o \
Source/Core/FontEffectBlur.o \
Source/Core/FontEffectGlow.o \
Source/Core/FontEffectInstancer.o \
Source/Core/FontEffectOutline.o \
Source/Core/FontEffectShadow.o \
Source/Core/FontEngineInterface.o \
Source/Core/Geometry.o \
Source/Core/GeometryBackgroundBorder.o \
Source/Core/GeometryDatabase.o \
Source/Core/GeometryUtilities.o \
Source/Core/LayoutBlockBox.o \
Source/Core/LayoutBlockBoxSpace.o \
Source/Core/LayoutDetails.o \
Source/Core/LayoutEngine.o \
Source/Core/LayoutInlineBox.o \
Source/Core/LayoutInlineBoxText.o \
Source/Core/LayoutLineBox.o \
Source/Core/LayoutTable.o \
Source/Core/LayoutTableDetails.o \
Source/Core/Log.o \
Source/Core/Math.o \
Source/Core/Memory.o \
Source/Core/ObserverPtr.o \
Source/Core/Plugin.o \
Source/Core/PluginRegistry.o \
Source/Core/Profiling.o \
Source/Core/PropertiesIteratorView.o \
Source/Core/Property.o \
Source/Core/PropertyDefinition.o \
Source/Core/PropertyDictionary.o \
Source/Core/PropertyParserAnimation.o \
Source/Core/PropertyParserColour.o \
Source/Core/PropertyParserDecorator.o \
Source/Core/PropertyParserFontEffect.o \
Source/Core/PropertyParserKeyword.o \
Source/Core/PropertyParserNumber.o \
Source/Core/PropertyParserRatio.o \
Source/Core/PropertyParserString.o \
Source/Core/PropertyParserTransform.o \
Source/Core/PropertySpecification.o \
Source/Core/RenderInterface.o \
Source/Core/Spritesheet.o \
Source/Core/Stream.o \
Source/Core/StreamFile.o \
Source/Core/StreamMemory.o \
Source/Core/StringUtilities.o \
Source/Core/StyleSheet.o \
Source/Core/StyleSheetContainer.o \
Source/Core/StyleSheetFactory.o \
Source/Core/StyleSheetNode.o \
Source/Core/StyleSheetNodeSelector.o \
Source/Core/StyleSheetNodeSelectorEmpty.o \
Source/Core/StyleSheetNodeSelectorFirstChild.o \
Source/Core/StyleSheetNodeSelectorFirstOfType.o \
Source/Core/StyleSheetNodeSelectorLastChild.o \
Source/Core/StyleSheetNodeSelectorLastOfType.o \
Source/Core/StyleSheetNodeSelectorNthChild.o \
Source/Core/StyleSheetNodeSelectorNthLastChild.o \
Source/Core/StyleSheetNodeSelectorNthLastOfType.o \
Source/Core/StyleSheetNodeSelectorNthOfType.o \
Source/Core/StyleSheetNodeSelectorOnlyChild.o \
Source/Core/StyleSheetNodeSelectorOnlyOfType.o \
Source/Core/StyleSheetParser.o \
Source/Core/StyleSheetSpecification.o \
Source/Core/SystemInterface.o \
Source/Core/Template.o \
Source/Core/TemplateCache.o \
Source/Core/Texture.o \
Source/Core/TextureDatabase.o \
Source/Core/TextureLayout.o \
Source/Core/TextureLayoutRectangle.o \
Source/Core/TextureLayoutRow.o \
Source/Core/TextureLayoutTexture.o \
Source/Core/TextureResource.o \
Source/Core/Transform.o \
Source/Core/TransformPrimitive.o \
Source/Core/TransformState.o \
Source/Core/TransformUtilities.o \
Source/Core/Tween.o \
Source/Core/TypeConverter.o \
Source/Core/URL.o \
Source/Core/Variant.o \
Source/Core/WidgetScroll.o \
Source/Core/XMLNodeHandler.o \
Source/Core/XMLNodeHandlerBody.o \
Source/Core/XMLNodeHandlerDefault.o \
Source/Core/XMLNodeHandlerHead.o \
Source/Core/XMLNodeHandlerTemplate.o \
Source/Core/XMLParser.o \
Source/Core/XMLParseTools.o \
Source/Core/FontEngineDefault/FontEngineInterfaceDefault.o \
Source/Core/FontEngineDefault/FontFace.o \
Source/Core/FontEngineDefault/FontFaceHandleDefault.o \
Source/Core/FontEngineDefault/FontFaceLayer.o \
Source/Core/FontEngineDefault/FontFamily.o \
Source/Core/FontEngineDefault/FontProvider.o \
Source/Core/FontEngineDefault/FreeTypeInterface.o \
 \
Source/Debugger/Debugger.o \
Source/Debugger/DebuggerPlugin.o \
Source/Debugger/DebuggerSystemInterface.o \
Source/Debugger/ElementContextHook.o \
Source/Debugger/ElementInfo.o \
Source/Debugger/ElementLog.o \
Source/Debugger/Geometry.o
