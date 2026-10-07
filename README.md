| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C5 | ESP32-C6 | ESP32-C61 | ESP32-H2 | ESP32-H21 | ESP32-H4 | ESP32-P4 | ESP32-S2 | ESP32-S3 | ESP32-S31 | Linux |
| ----------------- | ----- | -------- | -------- | -------- | -------- | --------- | -------- | --------- | -------- | -------- | -------- | -------- | --------- | ----- |

# Example For SDP810 Differential Pressure Sensor

Captures time series data and provides some lightweight processing

## How to use example

Setup python venv (assuming uv is already installed)
```bash
uv venv
source ./.venv/bin/activate
pip install -r requirements.txt
```

Setup expressif idf
```bash
source ${ESP_IDF_DIR}/export.sh
# ex:
source ~/hardware_projects/esp-idf/export.sh
```

Setup target, ex:
```bash
idf.py set-target esp32s3
```

build and flash
```bash
idf.py build
idf.py flash
idf.py montior
```

## Example folder contents

The project **pressure_sensor** contains one source file in C language [pressure_sensor_main.c](main/pressure_sensor_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── pytest_pressure_sensor.py  Python script used for automated testing
├── chart.py                   Chart data captured from stdout
├── main
│   ├── CMakeLists.txt
│   └── pressure_sensor_main.c Main logic
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.
