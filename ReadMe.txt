Rosco
AHRS and ADS-B Weather and Traffic display app for Android
----------------------------------------------------------

by Allen K. Lair
   Unexploded Minds

Updated 2018-01-29

DO NOT RELY ON THIS APP FOR YOUR PRIMARY FLIGHT INSTRUMENTATION.

This app is meant solely as a backup and a convenient alternative
display of available traffic and weather information available
from ADS-B through the Stratux ADS-B receiver.

This application connects to the Stratux WiFi signal and makes
several websocket connections that the Stratux makes available.
It does not use the GLD90 stream on port 4000 but instead uses the
websocket interface that provides much (all?) of the same info.  The
StreamReader class could probably be modified to use the GLD90 format
stream if desired.

Most of the hard work is done by the Stratux itself and this app
is mostly a "dumb" display for that data.  It is not meant to replace
far more robust EFB apps.  It is meant to be used as a simple display
for the AHRS and available ADS-B data provided by the Stratux.

This app depends heavily on the Qt libraries, version 5.10 as of this
writing.  You will also need the Android NDK and Java JDK in order to
compile, package and run the app on your android device.  Several IDEs
make this process fairly painless.  Qt Creator was used as the IDE for
initial development.  As of this writing JDK versions above 1.8 are
not guaranteed to integrate completely with the NDK and the Qt
Android integration.  That may very well change in the future.

This software is provided for free under the GPL v3 license.  Enjoy!
