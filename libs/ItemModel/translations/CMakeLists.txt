
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

mdt_add_library_translations(
  TARGET ItemModel
  SOURCES_DIRECTORY ../src
  TS_FILES_DIRECTORY .
)
