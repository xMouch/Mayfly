#!/bin/bash
wmctrl -xc m_run
alacritty --class m_run -e ./term-wrap.sh ./run.sh &
sleep 0.1
wmctrl -a 4coder
sleep 0.1
wmctrl -xr m_run -b toggle,above
