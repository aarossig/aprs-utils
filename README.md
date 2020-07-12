# aprs-utils

This project contains utilities for use with APRS Amateur Packet Radio.

## aprs-file-copy

The aprs-file-copy tool implements a chunked file transfer protocol that
uses APRS packet radio for backhaul.

### overview

This is inspired by the desire to send signed emails and small, highly
compressed images over the exsiting APRS infrastructure. Emails that are signed
with PGP contain a signature that is itself larger than a single APRS packet.
Images can be heavily compressed with modern compression algorithms such as WebP
to produce images that are visually interesting with file sizes that are as
small as 1-5kB.

### operating considerations

The purpose of this tool is to transmit small (1-10k) files over the APRS
network without distrupting other users. This means very slow transfers of very
small files. The effective data rate will end up being in the neighborhood of
1-10 bytes/sec (less than 10% of APRS total available bandwidth). This gives an
expectation that file transfers could take 10-15 minutes to complete, even for
small files.

### example usage

The primary modes that this tool operates in is sender and receiver. These
are controlled by the flags `--send` and `--receive`.

#### broadcast sender

`aprs-file-copy` supports transmitting files via RF through the use of a TNC
(Terminal Node Controller). Example usage:

```
aprs-file-copy --callsign <your call> \ # You station callsign.
    --send <file to send>               # The path to the file to transmit
```

This will connect to the TNC running on localhost (`direwolf` typically). This
can be overridden with the `--tnc_hostname` flag to connect to a TNC that is
running on another machine.

#### broadcast receiver

##### RF

`aprs-file-copy` supports receiving from RF via TNC. Example usage:

```
aprs-file-copy --callsign <your call> \
    --receive
```

This will connect to the TNC running on localhost (`direwolf` typically). This
can be overridden with the `--tnc_hostname` flag to connect to a TNC that is
running on another machine. The same as the file sender.

##### APRS-IS

`aprs-file-copy` also supports receiving files from the internet using the
APRS-IS (APRS Internet Services) servers. Example usage:

```
aprs-file-copy --callsign KN6FVU \
    --use_aprs_is \
    --receive
```

## dependencies

This project has some dependencies which must be met in order to build.

```
boost-filesystem
cmake
libb64
libsdl-net
pkg-config
protobuf
tclap
```

CMake should do a reasonable job of keeping track of which dependencies are
missing from your system when you attempt to build.

## building

This project builds with `cmake`, so following the standard workflow should
allow you to build.

```
mkdir build
cd build
cmake ..
make -j`nproc`
```
