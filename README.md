# Example of Harfbuzz for ESP32

![Test Status](https://github.com/georgik/esp32-harfbuzz-example/actions/workflows/test.yml/badge.svg)


## On-line Demo Simulation

[![ESP32-P4 SDL3 Harfbuzz Simulation](docs/img/esp-box-harfbuzz.webp)](https://wokwi.com/experimental/viewer?diagram=https%3A%2F%2Fraw.githubusercontent.com%2Fgeorgik%2Fesp32-harfbuzz-example%2Fmain%2Fboards%2Fesp-box%2Fdiagram.json&firmware=https%3A%2F%2Fgithub.com%2Fgeorgik%2Fesp32-harfbuzz-example%2Freleases%2Fdownload%2Fv1.0.0%2Fesp32-harfbuzz-example-esp-box.bin)

[Run the ESP32-P4 simulation with Wokwi.com](https://wokwi.com/experimental/viewer?diagram=https%3A%2F%2Fraw.githubusercontent.com%2Fgeorgik%2Fesp32-harfbuzz-example%2Fmain%2Fboards%2Fesp-box%2Fdiagram.json&firmware=https%3A%2F%2Fgithub.com%2Fgeorgik%2Fesp32-harfbuzz-example%2Freleases%2Fdownload%2Fv1.0.0%2Fesp32-harfbuzz-example-esp-box.bin)

## Requirements

`idf_component_manager` 2.x - install manually

## Build

```shell
git clone git@github.com:georgik/esp32-sdl3-test.git
cd esp32-sdl3-test

idf.py @boards/esp-box-3.cfg build
```

### Other boards

- ESP32-S3-BOX-3
```shell
idf.py @boards/esp-box-3.cfg build
```

- ESP32-S3-BOX (prior Dec. 2023)
```shell
idf.py @boards/esp-box.cfg build
```

- ESP32-P4
```shell
idf.py @boards/esp32_p4_function_ev_board.cfg build
```

- M5Stack-CoreS3
```shell
idf.py @boards/m5stack_core_s3.cfg build
```


## Credits

- FreeSans.ttf - https://github.com/opensourcedesign/fonts/blob/master/gnu-freefont_freesans/FreeSans.ttf
