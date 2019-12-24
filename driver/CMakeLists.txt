if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601)
  add_definitions(-DWINVER=0x0601)
  add_definitions(-DWIN32_LEAN_AND_MEAN)

  message("** driver:win32/64: using win10/UMDF2 access method")

  add_subdirectory(win10-umdf2)
else(WIN32)
  message("** driver:GNU/Linux: using sysfs access method")
endif(WIN32)
