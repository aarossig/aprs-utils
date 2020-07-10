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
