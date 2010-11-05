#!/bin/bash
git pull
make powder-64-sse3
make powder-64-sse2
make powder-sse3
make powder-debug
make powder-sse2
make powder
make powder-sse
tar czf PowderToyGit.tar.gz ./

