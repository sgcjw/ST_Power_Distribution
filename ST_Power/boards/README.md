# Adding custom boards
Most of the time, an existing board configuration can be modified for our use case. It is *very rare* that the following will not work, unless a third-party unsupported system-on-chip is being used.

## STM32
For most STM32 chips, the board configuration can be stolen from its corresponding NUCLEO dev board.

For example, SBC-CAN uses the **STM32F042K6** chip, which has a corresponding board the [ST Nucleo F042K6](https://docs.platformio.org/en/latest/boards/ststm32/nucleo_f042k6.html). The configuration for this board can be found in the corresponding platform repo (e.g., [nucleo_f042k6](https://github.com/platformio/platform-ststm32/blob/master/boards/nucleo_f042k6.json)) as a link from the wiki page.

Being a development board, the NUCLEO has slightly different behaviour for flashing. A generic variant of the F042K6 can be created under `boards/` with this additional configuration disabled: 

```diff
-    "openocd_board": "st_nucleo_f0",
    "openocd_target": "stm32f0x",

-    "name": "ST Nucleo F042K6",
+    "name": "STM32F042K6 Generic",

-    "url": "https://developer.mbed.org/platforms/ST-Nucleo-F042K6/",
-    "vendor": "ST"
+    "url": "",
+    "vendor": ""
```

Another example is PMB STM, which uses **STM32F072RB** chip. Following similar steps, the edited board file can be found on `board/generic_f072rb.json` which removes some dependencies for the NUCLEO-F072RB board.
