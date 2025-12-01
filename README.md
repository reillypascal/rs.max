# rs.max

Max/MSP externals, including tools for databending and glitch sounds.

## \[rs.psk~\]

Import any file as raw binary data and play this data back as a control signal for synthesizing [PSK](https://en.wikipedia.org/wiki/Phase-shift_keying) (phase-shift keying) telecommunications signals. See [Nathan Ho's post](https://nathan.ho.name/posts/dm-synthesis/) on using digital modulation modes for synthesis to hear the kinds of sounds this produces.

## Notes

Compiled binaries are available in the `externals/` folder. The macOS binaries are not signed, and you may need to run the following to let Gatekeeper know they are safe:

```sh
# Run this from inside the same folder as the external
spctl --add rs.psk~.mxo
```
