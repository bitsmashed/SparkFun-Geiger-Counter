The _arduino_ethernet_board_ directory contains firmware for an arduino ethernet board connected to the Cosm.com API. The Geiger counter sends data to to the ethernet board, CPM is calcluated, then every minute CPM is loaded to the cosm servers. The live SparkFun Geiger Counter feed can be found [here](https://cosm.com/feeds/22279).

The _geiger_counter_board_ directory contains the firmware for the SparkFun Geiger Counter board. This firmware takes the pulses from the Geiger tube and outputs a random bit (0 or 1), every time an event occurs.

The _default_firmware_ directory contains the production hex file that comes loaded onto every Gieger Counter.