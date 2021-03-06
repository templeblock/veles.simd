# veles.simd
Various mathematical routines with SIMD acceleration (SSE/AVX/NEON) in the form of a compact C library.
Designed without any OS portability in mind, tested only on Linux. Can work on BSD/Darwin after some potentially minor adaptation. Supports Android build / bionic.

### Implemented features

*  Conversion between int16_t, int32_t and float
*  Parts of BLAS levels 1, 2, 3 with a completely different API (e.g., matrices, vectors, scalars)
*  1D convolution and correlation with best approach detection (naive, overlap-save, FFT)
*  1D peak detection
*  sin, cos, log, exp (delegated to [AVX mathfun](http://software-lisc.fbk.eu/avx_mathfun/) and [NEON mathfun](http://gruntthepeon.free.fr/ssemath/neon_mathfun.html))
*  1D and 2D normalization
*  1D decimated and stationary (undecimated) wavelets

### Building
```
./autogen.sh
mkdir build && cd build
../configure
make -j$(getconf _NPROCESSORS_ONLN)
make test
make install DESTDIR=...
```

By default, this library makes use of [FFTF](https://github.com/Samsung/FFTF).
You can pass ``--disable-simd-fftf`` to ``configure`` to skip building dependent features.

### Copyright
Copyright © 2013 Samsung R&D Institute Russia

### License
[Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0).
