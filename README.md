# Midicontrol
A simple program to interpret midi notes and run corresponding shell commands

##Building

1. Clone or download contents of git repository  
`git clone https://github.com/Ghosy/midicontrol.git`

2. Navigate to the cloned repository  
`cd midicontrol`

3. Build the executable  
`make`

4. Run midicontrol  
`bin/midicontrol`

##Configuration

Location: ~/.midicontrolrc or ~/.config/midicontrol/midicontrolrc

Example configuration:
`
device = Launchpad  
144,0,127 = mpc -q prev, light_push, 60  
144,1,127 = mpc -q toggle, light_check, 60, mpc | grep -q "\\[playing\\]"  
144,2,127 = mpc -q next, light_push, 60  
144,3,127 = mpc -q volume -5, light_push, 60  
144,4,127 = mpc -q volume +5, light_push, 60  
144,5,127 = mpc -q repeat, light_check, 60, mpc | grep -q "repeat: on"  
144,6,127 = mpc -q single, light_check, 60, mpc | grep -q "single: on"  
144,7,127 = mpc -q random, light_check, 60, mpc | grep -q "random: on"  
144,16,127 = mpc -q clear && mpc -q load Reflections\ in\ Sand >> /dev/null  
144,17,127 = mpc -q clear && mpc -q load Noble\ Beast >> /dev/null  
144,18,127 = mpc -q clear && mpc -q load Break\ From\ This\ World >> /dev/null  
144,19,127 = mpc -q clear && mpc -q load Transistor\ OST >> /dev/null  
144,20,127 = mpc -q clear && mpc -q load Stan\ SB >> /dev/null  
144,21,127 = mpc -q clear && mpc -q load PPPPPP >> /dev/null  
144,22,127 = mpc -q clear && mpc -q load A\ Prague\ Spring >> /dev/null  
144,23,127 = mpc -q clear && mpc -q load VPR >> /dev/null  
144,24,127 = mpc -q clear  
144,39,127 = ~/projects/soundboard/soundboard.sh -c -f ~/projects/soundboard/clips/JeopardyTheme.wav, light_wait, 60  
`
