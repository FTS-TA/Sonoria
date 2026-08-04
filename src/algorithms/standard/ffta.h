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

#ifndef SONORIA_FFTA_H
#define SONORIA_FFTA_H

#include "algorithm.h"
#include "threading.h"
#include <complex>

// Cross-platform FFT support
#ifdef __APPLE__
  #include <Accelerate/Accelerate.h>
#else
  #include <fftw3.h>
#endif

namespace sonoria {
namespace standard {

class FFTA : public Algorithm {

 protected:
  Input<std::vector<Real> > _signal;
  Output<std::vector<std::complex<Real> > > _fft;

 public:
  FFTA() {
      declareInput(_signal, "frame", "the input audio frame");
      declareOutput(_fft, "fft", "the FFT of the input frame");
        
#ifdef __APPLE__
      fftSetup = NULL;
      accelBuffer.realp = NULL;
      accelBuffer.imagp = NULL;
#else
      fftPlan = NULL;
      fftwSignal = NULL;
      fftwComplex = NULL;
#endif
      _fftPlanSize = 0;
  }

  ~FFTA();

  void declareParameters() {
    declareParameter("size", "the expected size of the input frame. This is purely optional and only targeted at optimizing the creation time of the FFT object", "[1,inf)", 1024);
  }

  void compute();
  void configure();

  static const char* name;
  static const char* category;
  static const char* description;

 protected:
  friend class IFFTA;
  friend class FFTAComplex;
  friend class IFFTAComplex;
  static ForcedMutex globalFFTAMutex;

#ifdef __APPLE__
  FFTSetup fftSetup;
    
  int logSize;
  int _fftPlanSize;
  DSPSplitComplex accelBuffer;
#else
  fftwf_plan fftPlan;
  float* fftwSignal;
  fftwf_complex* fftwComplex;
  int _fftPlanSize;
#endif

  void createFFTObject(int size);
};

} // namespace standard
} // namespace sonoria

#include "streamingalgorithmwrapper.h"

namespace sonoria {
namespace streaming {

class FFTA : public StreamingAlgorithmWrapper {

 protected:
  Sink<std::vector<Real> > _signal;
  Source<std::vector<std::complex<Real> > > _fft;

 public:
  FFTA() {
    declareAlgorithm("FFT");
    declareInput(_signal, TOKEN, "frame");
    declareOutput(_fft, TOKEN, "fft");
  }
};

} // namespace streaming
} // namespace sonoria

#endif // ESSENTIA_FFTW_H
