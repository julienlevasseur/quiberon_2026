# SD card for the MP3 player (MP3-TF-16P / DFPlayer Mini)

*Version française : [sd-card.fr.md](sd-card.fr.md).*

The player finds tracks by **number**, not by name. The firmware asks for
"track 1 of the `mp3` folder", and the module looks for a file whose name
**starts with `0001`**. Anything after the four digits is free text.

## Layout

Card formatted **FAT32** (FAT16 also works), at most 32 GB.

```
/mp3/0001_AttentionAuDepart543210.mp3   start countdown, the 3-minute ride starts when it ends
/mp3/0002_WakaWakaShakira.mp3           during the ride, looped
/mp3/0003_BravoApplaudissements.mp3     applause at the start of the finale
/mp3/0004_Vainqueur.mp3                 winner music after the applause (file still to come)
```

- The folder is named `mp3` in lower case, at the root of the card.
- Four digits, then anything: `0002_WakaWakaShakira.mp3` and `0002.mp3` are equivalent.
- The numbers match `TRACK_*` in [config.h](../firmware/quiberon_bike/src/core/config.h).
  To swap a sound, replace the file and keep its number.
- MP3 at 44.1 kHz or 48 kHz, 128–320 kbit/s. Avoid huge ID3 tags with cover art:
  some modules wait several seconds before playing them.
- Volume is set by the firmware (`VOLUME = 20` out of 30), not by the files.
  Normalise the four files to a similar loudness, so the applause is not
  twice as loud as the music.

## Good practice

- Copy the files to a **freshly formatted** card. Deleting and re-copying files
  can leave stale directory entries that confuse some clones.
- Do not leave other files at the root (no `System Volume Information` from
  Windows or `.Trashes` from macOS if you can avoid it; on macOS run
  `dot_clean /Volumes/<card>` after copying).
- After copying, check on the bench with the serial console: `play 1` … `play 4`
  (see [bench-test.md](bench-test.md)).
