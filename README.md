# SFRPlot

Plot SFR history from Gadget's sfr.txt file.

## Usage
        -h: help
        -m: plot with Madau & Dickinson (2014) (require b option)
        -o: plot with observational data (require bz option)
        -b: boxsize (Unit: Mpc/h)
        -l: plot SFR in log scale
        -z: set x axis redshift
        -Z: set x axis redshift and set max of xrange
        -s: save figure (png only)

        eg) sfrplot -mob 10 -lZ 15 -s sfr.png data1/sfr.txt data2/sfr.txt
