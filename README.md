
jackmaster - A "Master Console" for the jack-audio-connection-kit

Features:
---------
* 100% pure C
* Low CPU usage
* Session management via LASH
* Fader automation via ALSA sequencer
* Number of inputs: unlimited (actually, it depends on things like available
  memory, the architecture and compiler. Lets just say: enaugh)
* Crosstalk: 0.0 dB
* Dynamics range: somewhere in the vicinity of 1529.23 dB


Bugs:
-----
huh?


Minor shortcomings:
-------------------
* When an active channel is rerouted, you might hear a click (although i
  never noticed such a thing, i know it *must* be there occasionally)
* Ugly icon (at least, it is recognisable)
* Number of inputs/subs and screen layout can only be changed by recompiling
* Only stereo ports
* No aux-sends/returns yet
* No (LADSPA-)insert effects (yet)


Installation:
-------------
Just the usual
./configure
make
make install (as root, if you can't write in /usr/local)


Thanks to:
----------
The JackEQ folks, for gtkmeter and some code fragments
The XMMS crowd, for the config-file stuff
The GTK bunch, for their wonderful shiny toolkit
RMS and Linus, for the obvious reasons
Albert Gore, for funding the internet development
