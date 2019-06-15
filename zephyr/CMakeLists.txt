zephyr_sources(
  ../camera/camera.c
  ../bsp/gapuino.c
  ../camera/camera.c
  ../camera/himax/himax.c
  ../display/display.c
  ../display/ili9341/ili9341.c
  ../flash/flash.c
  ../flash/hyperflash/hyperflash.c
)

zephyr_compile_options(
  -DCONFIG_GAPUINO
  )

zephyr_include_directories(../include)