zephyr_sources(
  ../camera/camera.c
  ../bsp/gapuino.c
  ../camera/camera.c
  ../camera/himax/himax.c
  ../display/display.c
  ../flash/flash.c
)

zephyr_compile_options(
  -DCONFIG_GAPUINO
  )

zephyr_include_directories(../include)