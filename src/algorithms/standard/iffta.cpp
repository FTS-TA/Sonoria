/*
 * Copyright (C) 2006-2021  Music Technology Group - Universitat Pompeu Fabra
 *
 * This file is part of Essentia
 *
 * Essentia is free software: you can redistribute it and/or modify it under
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
 */

#include "iffta.h"
#include "ffta.h"

using namespace std;
using namespace sonoria;
using namespace standard;

const char* IFFTA::name = "IFFT";
const char* IFFTA::category = "Standard";
const char* IFFTA::description = DOC("This algorithm calculates the inverse short-term Fourier transform (STFT) of an array of complex values using the FFT algorithm. The resulting frame has a size of (s-1)*2, where s is the size of the input fft frame. The inverse Fourier transform is not defined for frames which size is less than 2 samples. Otherwise an exception is thrown.\n"
"\n"
"An exception is thrown if the input's size is not larger than 1.\n"
"\n"
#ifdef __APPLE__
"FFT computation will be carried out using the Accelerate Framework [3]"
"\n"
"References:\n"
"  [1] Fast Fourier transform - Wikipedia, the free encyclopedia,\n"
"  http://en.wikipedia.org/wiki/Fft\n\n"
"  [2] Fast Fourier Transform -- from Wolfram MathWorld,\n"
"  http://mathworld.wolfram.com/FastFourierTransform.html\n"
"  [3] vDSP Programming Guide -- from Apple\n"
"  https://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html"
#else
"FFT computation will be carried out using FFTW library\n"
"\n"
"References:\n"
"  [1] Fast Fourier transform - Wikipedia, the free encyclopedia,\n"
"  http://en.wikipedia.org/wiki/Fft\n\n"
"  [2] FFTW - Fastest Fourier Transform in the West,\n"
"  http://www.fftw.org/"
#endif
);


IFFTA::~IFFTA() {
  ForcedMutexLocker lock(FFTA::globalFFTAMutex);

  // we might have called sonoria::shutdown() before this algorithm goes out
  // of scope, so make sure we're not doing stupid things here
  if (sonoria::isInitialized()) {
#ifdef __APPLE__
    vDSP_destroy_fftsetup(fftSetup);
    delete[] accelBuffer.realp;
    delete[] accelBuffer.imagp;
#else
    if (fftPlan) {
      fftwf_destroy_plan(fftPlan);
    }
    fftwf_free(fftwSignal);
    fftwf_free(fftwComplex);
#endif
  }
}

void IFFTA::compute() {

  const std::vector<std::complex<Real> >& fft = _fft.get();
  std::vector<Real>& signal = _signal.get();

  // check if input is OK
  int size = ((int)fft.size()-1)*2;
  if (size <= 0) {
    throw SonoriaException("IFFT: Input size cannot be 0 or 1");
  }
#ifdef __APPLE__
  if ((fftSetup == 0) ||
      ((fftSetup != 0) && _fftPlanSize != size)) {
    createFFTObject(size);
  }

  //Pack
  accelBuffer.realp[0] = fft[0].real();
  accelBuffer.imagp[0] = fft[fft.size()-1].real();
  
  for(int i=1; i<fft.size()-1; i++) {
      accelBuffer.realp[i] = fft[i].real();
      accelBuffer.imagp[i] = fft[i].imag();
  }
  
  vDSP_fft_zrip(fftSetup, &accelBuffer, 1, logSize, FFT_INVERSE);
  
  // copy result from plan to output vector
  signal.resize(size);
  
  vDSP_ztoc(&accelBuffer, 1, (COMPLEX*)&signal[0], 2, size/2);
#else
  // FFTW implementation for non-Apple platforms
  if ((fftPlan == NULL) || (_fftPlanSize != size)) {
    createFFTObject(size);
  }

  // Copy input data to FFTW buffer
  // fftwComplex is an array of float[2], where [0] is real and [1] is imaginary
  fftwComplex[0][0] = fft[0].real();
  fftwComplex[0][1] = 0.0f;
  
  for (int i = 1; i < (int)fft.size() - 1; i++) {
    fftwComplex[i][0] = fft[i].real();
    fftwComplex[i][1] = fft[i].imag();
  }
  // Last element (Nyquist frequency)
  fftwComplex[fft.size()-1][0] = fft[fft.size()-1].real();
  fftwComplex[fft.size()-1][1] = 0.0f;

  // Execute inverse FFT
  fftwf_execute(fftPlan);

  signal.resize(size);
  for (int i = 0; i < size; i++) {
    signal[i] = static_cast<Real>(fftwSignal[i]);
  }
#endif

  if (_normalize) {
    Real norm = (Real)size;
    
    for (int i = 0; i < size; i++) {
      signal[i] /= norm;
    }
  }
}

void IFFTA::configure() {
  createFFTObject(parameter("size").toInt());
  _normalize = parameter("normalize").toBool();
}

void IFFTA::createFFTObject(int size) {
  ForcedMutexLocker lock(FFTA::globalFFTAMutex);
    
#ifdef __APPLE__
  delete[] accelBuffer.realp;
  delete[] accelBuffer.imagp;
  accelBuffer.realp = new float[size/2];
  accelBuffer.imagp = new float[size/2];
    
  logSize = log2(size);
    
  // With vDSP you only need to create a new fft if you've increased the size.
  if(size > _fftPlanSize) {
    vDSP_destroy_fftsetup(fftSetup);
    fftSetup = vDSP_create_fftsetup(logSize, 0);
  }
#else
  // FFTW implementation for non-Apple platforms
  if (size > _fftPlanSize || fftPlan == NULL) {
    if (fftPlan != NULL) {
      fftwf_destroy_plan(fftPlan);
      fftwf_free(fftwSignal);
      fftwf_free(fftwComplex);
    }
    
    fftwSignal = (float*)fftwf_malloc(sizeof(float) * size);
    fftwComplex = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * (size/2 + 1));
    
    fftPlan = fftwf_plan_dft_c2r_1d(size, fftwComplex, fftwSignal, FFTW_ESTIMATE);
  }
#endif
    
  _fftPlanSize = size;
}
