
set(SOURCE_FILES
  Mdt/PlainText/FileInputIterator.cpp
  Mdt/PlainText/FileInputIteratorSharedData.cpp
  Mdt/PlainText/FileMultiPassIterator.cpp
  Mdt/PlainText/StringConstIterator.cpp
  Mdt/PlainText/RecordTemplate.cpp
  Mdt/PlainText/Record.cpp
  Mdt/PlainText/StringRecord.cpp
  Mdt/PlainText/RecordListTemplate.cpp
  Mdt/PlainText/RecordList.cpp
  Mdt/PlainText/RecordListTableModel.cpp
  Mdt/PlainText/StringRecordList.cpp
  Mdt/PlainText/BoostSpiritQtTraits.cpp
  Mdt/PlainText/CsvCommonSettings.cpp
  Mdt/PlainText/CsvParserSettings.cpp
  Mdt/PlainText/CsvParserTemplate.cpp
  Mdt/PlainText/CsvStringParser.cpp
  Mdt/PlainText/CsvFileParser.cpp
  Mdt/PlainText/FileReader.cpp
)

find_package(Qt5 COMPONENTS Core)

# Get Boost include directory if not allready set
# We only use header onyl libraries, so we not use COMPONENTS
# See: http://stackoverflow.com/questions/26749461/how-to-determine-the-boost-library-names-for-cmake
if(NOT Boost_FOUND)
  find_package(Boost REQUIRED)
endif()

mdt_add_library(
  NAME PlainText_Core
  SOURCE_FILES ${SOURCE_FILES}
  HEADERS_DIRECTORY .
  LINK_DEPENDENCIES Error_Core Expected Qt5::Core
)
target_include_directories(PlainText_Core PUBLIC ${Boost_INCLUDE_DIRS})

mdt_set_library_description(
  NAME PlainText_Core
  DESCRIPTION "PlainText_Core provides common stuff for plain text processing."
)
