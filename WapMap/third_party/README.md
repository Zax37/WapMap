stb_image_write.h is stb_image_write v1.16, vendored from the SFML 3.0.2
extlibs/headers/stb_image copy. Its dual MIT/public-domain license is included
at the end of the header.

The mapshot writer builds a private static implementation and supplies the
project's existing zlib dependency for PNG compression levels 0–9. Output is
encoded through callbacks so filesystem paths and committing complete files
remain under application control.
