#!/bin/bash

#do this for all the sample textures
for infile in `ls sampleTextures`; do
  ./bin/Debug/TextureSynthesis sampleTextures/$infile 23 64 &
done

