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

#ifndef ESSENTIA_IFFTA_H
#define ESSENTIA_IFFTA_H

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

class IFFTA : public Algorithm {

 protected:
  Input<std::vector<std::complex<Real> > > _fft;
  Output<std::vector<Real> > _signal;

 public:
  IFFTA() {
    declareInput(_fft, "fft", "the input frame");
    declareOutput(_signal, "frame", "the IFFT of the input frame");
      
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

  ~IFFTA();

  void declareParameters() {
    declareParameter("size", "the expected size of the input frame. This is purely optional and only targeted at optimizing the creation time of the FFT object", "[1,inf)", 1024);
    declareParameter("normalize", "whether to normalize the output by the FFT length.", "{true,false}", true);
  }


  void compute();
  void configure();

  static const char* name;
  static const char* category;
  static const char* description;

 protected:
//  fftwf_plan _fftPlan;
//  std::complex<Real>* _input;
//  Real* _output;

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

  bool _normalize;

  void createFFTObject(int size);
};

} // namespace standard
} // namespace sonoria

#include "streamingalgorithmwrapper.h"

namespace sonoria {
namespace streaming {

class IFFTA : public StreamingAlgorithmWrapper {

 protected:
  Sink<std::vector<std::complex<Real> > > _fft;
  Source<std::vector<Real> > _signal;

 public:
  IFFTA() {
    declareAlgorithm("IFFT");
    declareInput(_fft, TOKEN, "fft");
    declareOutput(_signal, TOKEN, "frame");
  }
};

} // namespace streaming
} // namespace sonoria

#endif // ESSENTIA_IFFTA_H
