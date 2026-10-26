# Felony

The Ornament & Crime firmware ([mxmxmx/O_C](https://github.com/mxmxmx/O_C), MIT) running in VCV Rack.
`src/oc` is the original firmware; only the hardware drivers are replaced by a virtual O&C in
`shim/` (pins, SH1106 display, DAC8565, ADC, EEPROM, timers). `tools/port.py` regenerates `src/oc`
from a checkout of the firmware in `firmware/`:

    git clone --depth 1 https://github.com/mxmxmx/O_C.git firmware
    python tools/port.py

Controls: click an encoder to press it, drag or scroll to turn it, hold for a long press
(long press right = app list). Settings are saved in the patch (the EEPROM image).

Only one Felony per Rack: the firmware keeps its state in globals.
