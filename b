#!/bin/bash

# proxychains meson wrap install  freetype2

meson setup build  --native-file=native.build --reconfigure
# meson setup build  --native-file=native.build
# meson compile -C build m

meson compile -C build m3
