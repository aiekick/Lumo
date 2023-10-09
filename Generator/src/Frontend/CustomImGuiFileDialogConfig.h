/*
Copyright 2022-2022 Stephane Cuillerdier (aka aiekick)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once
#pragma warning(disable : 4005)

#include <ImWidgets.h>

// uncomment and modify defines under for customize ImGuiFileDialog

//this options need c++17
#define USE_STD_FILESYSTEM

//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

// the slash's buttons in path cna be used for quick select parallles directories
#define USE_QUICK_PATH_SELECT
// 
// the spacing between button path's can be customized. 
// if disabled the spacing is defined by the imgui theme
// define the space between path buttons 
//#define CUSTOM_PATH_SPACING 2

//#define USE_THUMBNAILS
//the thumbnail generation use the stb_image and stb_resize lib who need to define the implementation
//btw if you already use them in your app, you can have compiler error due to "implemntation found in double"
//so uncomment these line for prevent the creation of implementation of these libs again
#define DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
#define DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
//#define IMGUI_RADIO_BUTTON RadioButton
//#define DisplayMode_ThumbailsList_ImageHeight 32.0f
//#define tableHeaderFileThumbnailsString "Thumbnails"
//#define DisplayMode_FilesList_ButtonString "FL"
//#define DisplayMode_FilesList_ButtonHelp "File List"
//#define DisplayMode_ThumbailsList_ButtonString "TL"
//#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
//todo : grid 
//#define DisplayMode_ThumbailsGrid_ButtonString "TG"
//#define DisplayMode_ThumbailsGrid_ButtonHelp "Thumbnails Grid"

//#define USE_EXPLORATION_BY_KEYS
// this mapping by default is for GLFW but you can use another
//#include <GLFW/glfw3.h>
// Up key for explore to the top
#define IGFD_KEY_UP 265 //GLFW_KEY_UP
// Down key for explore to the bottom
#define IGFD_KEY_DOWN 264 // GLFW_KEY_DOWN
// Enter key for open directory
#define IGFD_KEY_ENTER 257 // GLFW_KEY_ENTER
// BackSpace for comming back to the last directory
#define IGFD_KEY_BACKSPACE 259 // GLFW_KEY_BACKSPACE

// by ex you can quit the dialog by pressing the key excape
//#define USE_DIALOG_EXIT_WITH_KEY
//#define IGFD_EXIT_KEY ImGuiKey_Escape

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
#define IMGUI_PATH_BUTTON ImGui::ContrastedButton_For_Dialogs
// standar button
#define IMGUI_BUTTON ImGui::ContrastedButton_For_Dialogs

#define ICON_SDFM_PLUS u8"\uf415"
#define ICON_SDFM_MINUS u8"\uf374"
#define ICON_SDFM_CHECK u8"\uf12c"
#define ICON_SDFM_CANCEL u8"\uf739"
#define ICON_SDFM_TRASH_CAN_OUTLINE u8"\ufa79"
#define ICON_SDFM_SERVER u8"\uf48b"
#define ICON_SDFM_FOLDER_SEARCH_OUTLINE u8"\uf968"
#define ICON_SDFM_FOLDER u8"\uf24b"
#define ICON_SDFM_LINK u8"\uf337"
#define ICON_SDFM_FILE u8"\uf214"
#define ICON_SDFM_CHEVRON_UP u8"\uf143"
#define ICON_SDFM_CHEVRON_DOWN u8"\uf140"
#define ICON_SDFM_BOOKMARK u8"\uf0c0"

// locales string
#define createDirButtonString ICON_SDFM_PLUS
#define okButtonString ICON_SDFM_CHECK " OK"
#define cancelButtonString ICON_SDFM_CANCEL " Cancel"
#define resetButtonString ICON_SDFM_TRASH_CAN_OUTLINE
#define drivesButtonString ICON_SDFM_SERVER
#define searchString ICON_SDFM_FOLDER_SEARCH_OUTLINE
#define dirEntryString ICON_SDFM_FOLDER " "
#define linkEntryString ICON_SDFM_LINK " "
#define fileEntryString ICON_SDFM_FILE " "
//#define fileNameString "File Name : "
//#define dirNameString "Directory Path :"
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#define OverWriteDialogConfirmButtonString ICON_SDFM_CHECK " Confirm"
#define OverWriteDialogCancelButtonString ICON_SDFM_CANCEL " Cancel"

//Validation buttons
//#define okButtonString " OK"
//#define okButtonWidth 0.0f
//#define cancelButtonString " Cancel"
//#define cancelButtonWidth 0.0f
//alignement [0:1], 0.0 is left, 0.5 middle, 1.0 right, and other ratios
//#define okCancelButtonAlignement 0.0f
//#define invertOkAndCancelButtons 0

// DateTimeFormat
// see strftime functionin <ctime> for customize
// "%Y/%m/%d %H:%M" give 2021:01:22 11:47
// "%Y/%m/%d %i:%M%p" give 2021:01:22 11:45PM
//#define DateTimeFormat "%Y/%m/%d %i:%M%p"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#define tableHeaderAscendingIcon ICON_SDFM_CHEVRON_UP
#define tableHeaderDescendingIcon ICON_SDFM_CHEVRON_DOWN
#define tableHeaderFileNameString " File name"
#define tableHeaderFileTypeString " Type"
#define tableHeaderFileSizeString " Size"
#define tableHeaderFileDateTimeString " Date"
#define fileSizeBytes "o"
#define fileSizeKiloBytes "Ko"
#define fileSizeMegaBytes "Mo"
#define fileSizeGigaBytes "Go"

// default table sort field (must be FIELD_FILENAME, FIELD_TYPE, FIELD_SIZE, FIELD_DATE or FIELD_THUMBNAILS)
//#define defaultSortField FIELD_FILENAME

// default table sort order for each field (true => Descending, false => Ascending)
//#define defaultSortOrderFilename true
//#define defaultSortOrderType true
//#define defaultSortOrderSize true
//#define defaultSortOrderDate true
//#define defaultSortOrderThumbnails true

#define USE_BOOKMARK
#define defaultBookmarkPaneWith 200.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
#define bookmarksButtonString ICON_SDFM_BOOKMARK
//#define bookmarksButtonHelpString "Bookmark"
#define addBookmarkButtonString ICON_SDFM_PLUS
#define removeBookmarkButtonString ICON_SDFM_MINUS