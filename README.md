# Midicontrol
A simple program to interpret midi notes and run corresponding shell commands

## Building

1. Clone or download contents of git repository  
`git clone https://github.com/Ghosy/midicontrol.git`

2. Navigate to the cloned repository  
`cd midicontrol`

3. Build the executable  
`make`

4. Run midicontrol  
`bin/midicontrol`

## Configuration

Location: ~/.midicontrolrc or ~/.config/midicontrol/midicontrolrc

The configuration file uses YAML. An example of a configuration file can be found below.

Example configuration:
```
---
devices:
- device: Launchpad:Launchpad MIDI 1
  notes:
  - note: '144,0,127'
    command: mpc -q prev
    light_mode: LIGHT_PUSH
    light_value: 60
  - note: '144,1,127'
    command: mpc -q toggle
    light_mode: LIGHT_CHECK
    light_value: 60
    light_command: 'mpc | grep -q "\[playing\]"'
  - note: '144,2,127'
    command: mpc -q next
    light_mode: LIGHT_PUSH
    light_value: 60
  - note: '144,3,127'
    command: mpc -q volume -5
    light_mode: LIGHT_PUSH
    light_value: 60
  - note: '144,4,127'
    command: mpc -q volume +5
    light_mode: LIGHT_PUSH
    light_value: 60
  - note: '144,5,127'
    command: mpc -q repeat
    light_mode: LIGHT_CHECK
    light_value: 60
    light_command: 'mpc | grep -q "repeat: on"'
  - note: '144,6,127'
    command: mpc -q single
    light_mode: LIGHT_CHECK
    light_value: 60
    light_command: 'mpc | grep -q "single: on"'
  - note: '144,7,127'
    command: mpc -q random
    light_mode: LIGHT_CHECK
    light_value: 60
    light_command: 'mpc | grep -q "random: on"'
  - note: '144,39,127'
    command: ~/projects/soundboard/soundboard.sh -c -f ~/projects/soundboard/clips/JeopardyTheme.wav
    light_mode: LIGHT_WAIT
    light_value: 60
```

## License
This project is licensed under the GNU General Public License version 3(GPLv3) - see the [LICENSE.txt](LICENSE.txt) file for details.
