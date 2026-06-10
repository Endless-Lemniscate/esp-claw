#!/usr/bin/env python3
"""Validate the Waveshare ESP32-S3-Touch-AMOLED-1.43C board profile."""

from pathlib import Path

import yaml


ROOT = Path(__file__).resolve().parents[1]
BOARD_DIR = ROOT / "application/edge_agent/boards/waveshare/waveshare_esp32_s3_touch_amoled_143c"


def load_yaml(name):
    path = BOARD_DIR / name
    assert path.exists(), f"missing {path.relative_to(ROOT)}"
    return yaml.safe_load(path.read_text())


def find_named(items, name):
    for item in items:
        if item.get("name") == name:
            return item
    raise AssertionError(f"missing entry named {name}")


def main():
    assert BOARD_DIR.is_dir(), f"missing {BOARD_DIR.relative_to(ROOT)}"

    info = load_yaml("board_info.yaml")
    assert info["board"] == "waveshare_esp32_s3_touch_amoled_143c"
    assert info["chip"] == "esp32s3"
    assert info["manufacturer"] == "Waveshare"
    assert "ESP32-S3-PICO-1-N8R8" in info["description"]

    sdkconfig = (BOARD_DIR / "sdkconfig.defaults.board").read_text()
    assert "CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y" in sdkconfig
    assert 'CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions_8MB.csv"' in sdkconfig
    assert "CONFIG_SPIRAM_MODE_OCT=y" in sdkconfig
    assert "CONFIG_ESP_BOARD_DEV_DISPLAY_LCD_SUPPORT=y" in sdkconfig
    assert "CONFIG_ESP_SYSTEM_EVENT_QUEUE_SIZE=256" in sdkconfig

    peripherals = load_yaml("board_peripherals.yaml")["peripherals"]
    i2c = find_named(peripherals, "i2c_master")
    assert i2c["config"]["pins"] == {"sda": 47, "scl": 48}

    spi = find_named(peripherals, "spi_display")
    spi_bus = spi["config"]["spi_bus_config"]
    assert spi_bus["spi_port"] == "SPI2_HOST"
    assert spi_bus["data0_io_num"] == 9
    assert spi_bus["data1_io_num"] == 10
    assert spi_bus["data2_io_num"] == 11
    assert spi_bus["data3_io_num"] == 12
    assert spi_bus["sclk_io_num"] == 14

    i2s_out = find_named(peripherals, "i2s_audio_out")
    i2s_pins = i2s_out["config"]["pins"]
    assert i2s_pins == {"mclk": 38, "bclk": 39, "ws": 40, "dout": 41, "din": 42}

    devices = load_yaml("board_devices.yaml")["devices"]
    display = find_named(devices, "display_lcd")
    assert display["chip"] == "sh8601"
    assert display["type"] == "display_lcd"
    assert display["sub_type"] == "spi"
    assert display["config"]["x_max"] == 326
    assert display["config"]["y_max"] == 326
    assert display["config"]["mirror_x"] is True
    assert display["config"]["io_spi_config"]["cs_gpio_num"] == 15
    assert display["config"]["io_spi_config"]["dc_gpio_num"] == -1
    assert display["config"]["io_spi_config"]["lcd_cmd_bits"] == 32
    assert display["config"]["io_spi_config"]["flags"]["quad_mode"] is True
    assert display["config"]["lcd_panel_config"]["reset_gpio_num"] == 13

    touch = find_named(devices, "lcd_touch")
    assert touch["chip"] == "cst816t"
    assert touch["type"] == "lcd_touch"
    assert touch["sub_type"] == "i2c"
    assert touch["config"]["touch_config"]["x_max"] == 326
    assert touch["config"]["touch_config"]["y_max"] == 326
    assert touch["config"]["touch_config"]["int_gpio_num"] == -1
    assert touch["peripherals"][0]["i2c_addr"] == 0x2A

    setup_device = (BOARD_DIR / "setup_device.c").read_text()
    assert "touch_cfg.process_coordinates = cst820_process_coordinates;" in setup_device
    assert "touch_cfg.interrupt_callback = cst820_interrupt_cb;" not in setup_device

    audio_dac = find_named(devices, "audio_dac")
    assert audio_dac["chip"] == "es8311"
    audio_adc = find_named(devices, "audio_adc")
    assert audio_adc["chip"] == "es7210"

    print("waveshare_esp32_s3_touch_amoled_143c board profile OK")


if __name__ == "__main__":
    main()
