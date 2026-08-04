/*
 * Copyright (C) 2006-2021  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Sonoria
 *
 * Sonoria is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the Free
 * Software Foundation (FSF), either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * version 3 along with this program.  If not, see http://www.gnu.org/licenses/
 * 
 * NOTE: Accelerate library include issue resolved.
 * Fixed cross-platform FFT support by conditionally including:
 * - <Accelerate/Accelerate.h> for macOS/iOS
 * - <fftw3.h> for Linux/Windows (requires libfftw3-dev)
 */

#include "ffta.h"
#include "sonoria.h"

using namespace std;
using namespace sonoria;
using namespace standard;

const char* FFTA::name = "FFT";
const char* FFTA::category = "Standard";
const char* FFTA::description = DOC("This algorithm computes the positive complex short-term Fourier transform (STFT) of an array using the FFT algorithm. The resulting fft has a size of (s/2)+1, where s is the size of the input frame.\n"
"At the moment FFT can only be computed on frames which size is even and non zero, otherwise an exception is thrown.\n"
"\n"
"FFT computation will be carried out using:\n"
"- Accelerate Framework on macOS/iOS [3]\n"
"- FFTW3 library on Linux/Windows [4]\n"
"\n"
"References:\n"
"  [1] Fast Fourier transform - Wikipedia, the free encyclopedia,\n"
"  http://en.wikipedia.org/wiki/Fft\n\n"
"  [2] Fast Fourier Transform -- from Wolfram MathWorld,\n"
"  http://mathworld.wolfram.com/FastFourierTransform.html\n\n"
"  [3] vDSP Programming Guide -- from Apple\n"
"  https://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html\n\n"
"  [4] FFTW Home Page\n"
"  http://www.fftw.org/"
);

ForcedMutex FFTA::globalFFTAMutex;

FFTA::~FFTA() {
  ForcedMutexLocker lock(globalFFTAMutex);

  // we might have called sonoria::shutdown() before this algorithm goes out
  // of scope, so make sure we're not doing stupid things here
  // This will cause a memory leak then, but it is definitely a better choice
  // than a crash (right, right??? :-) )
  if (sonoria::isInitialized()) {
#ifdef __APPLE__
    vDSP_destroy_fftsetup(fftSetup);
    delete[] accelBuffer.realp;
    delete[] accelBuffer.imagp;
#else
    fftwf_destroy_plan(fftPlan);
    fftwf_free(fftwSignal);
    fftwf_free(fftwComplex);
#endif
  }
}

void FFTA::compute() {
    
  const std::vector<Real>& signal = _signal.get();
  std::vector<std::complex<Real> >& fft = _fft.get();

  // check if input is OK
  int size = int(signal.size());
  if (size == 0) {
    throw SonoriaException("FFT: Input size cannot be 0");
  }
 
  if ((
#ifdef __APPLE__
      fftSetup == 0
#else
      fftPlan == 0
#endif
      ) ||
      (
#ifdef __APPLE__
      (fftSetup != 0)
#else
      (fftPlan != 0)
#endif
       && _fftPlanSize != size)) {
    createFFTObject(size);
  }
    
#ifdef __APPLE__
  // Scramble-pack the real data into complex buffer in just the way that's
  // required by the real-to-complex FFT function that follows.
  vDSP_ctoz((DSPComplex*)&signal[0], 2, &accelBuffer, 1, size/2);
  
  // Do real->complex forward FFT
  vDSP_fft_zrip(fftSetup, &accelBuffer, 1, logSize, FFT_FORWARD);
  

  fft.resize(size/2+1);
  
  //Prob a much better way of doing this but for now this works
  // In the case of an FFT on a real input, the resulting value of each of
  // the Fourier coefficients is 2x the actual, mathematical value (see
  // VDSP fft documentation for scaling factors). We need to scale by /2.0f.
  //In Accelerate fttOutput[0] contains the real for point 0 and point N/2+1

  //Construct first point
  fft[0] = std::complex<Real>(accelBuffer.realp[0]/2.0f, 0.0f);
  
  for(int i=1; i<size/2; i++) {
      std::complex<Real> point(accelBuffer.realp[i]/2.0f, accelBuffer.imagp[i]/2.0f);
      fft[i] = point;
  }
  
  //Construct the last point
  fft[size/2] = std::complex<Real>(accelBuffer.imagp[0]/2.0f, 0.0f);
#else
  // FFTW3 implementation for Linux/Windows
  fft.resize(size/2+1);
  
  // Copy input signal to FFTW array
  for (int i = 0; i < size; i++) {
    fftwSignal[i] = static_cast<float>(signal[i]);
  }
  
  // Execute FFT plan
  fftwf_execute(fftPlan);
  
  // Copy results and scale
  fft[0] = std::complex<Real>(fftwComplex[0].r / 2.0f, 0.0f);
  
  for (int i = 1; i < size/2; i++) {
    fft[i] = std::complex<Real>(fftwComplex[i].r / 2.0f, fftwComplex[i].i / 2.0f);
  }
  
  fft[size/2] = std::complex<Real>(0.0f, fftwComplex[0].i / 2.0f);
#endif
}

void FFTA::configure() {
  createFFTObject(parameter("size").toInt());
}

void FFTA::createFFTObject(int size) {
  ForcedMutexLocker lock(globalFFTAMutex);

  // This is only needed because at the moment we return half of the spectrum,
  // which means that there are 2 different input signals that could yield the
  // same FFT...
  if (size % 2 == 1) {
    throw SonoriaException("FFT: can only compute FFT of arrays which have an even size");
  }

#ifdef __APPLE__
  delete[] accelBuffer.realp;
  delete[] accelBuffer.imagp;
  accelBuffer.realp = new float[size/2];
  accelBuffer.imagp = new float[size/2];
    
  logSize = log2(size);
    
  // With vDSP you only need to create a new fft if you've increased the size
  if(size > _fftPlanSize) {
    vDSP_destroy_fftsetup(fftSetup);
    fftSetup = vDSP_create_fftsetup(logSize, 0);
  }
#else
  // FFTW3 initialization for Linux/Windows
  if (size > _fftPlanSize || fftPlan == NULL) {
    if (fftPlan != NULL) {
      fftwf_destroy_plan(fftPlan);
      fftwf_free(fftwSignal);
      fftwf_free(fftwComplex);
    }
    
    fftwSignal = (float*)fftwf_malloc(sizeof(float) * size);
    fftwComplex = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (size/2 + 1));
    
    fftPlan = fftwf_plan_dft_r2c_1d(size, fftwSignal, fftwComplex, FFTW_ESTIMATE);
  }
#endif
    
  _fftPlanSize = size;
}
