#!/bin/bash

# meson setup build  --native-file=native.build --reconfigure
# meson setup build  --native-file=native.build
meson compile -C build
