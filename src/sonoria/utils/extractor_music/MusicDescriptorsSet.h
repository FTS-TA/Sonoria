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

#ifndef MUSIC_DESCRIPTOR_SET_H
#define MUSIC_DESCRIPTOR_SET_H


#include "sonoria/streaming/sourcebase.h"
#include "sonoria/pool.h"
#include "sonoria/types.h"
#include "sonoria/sonoriamath.h"
#include "sonoria/algorithm.h"
#include "sonoria/scheduler/network.h"
#include "sonoria/streaming/streamingalgorithm.h"
#include "sonoria/algorithmfactory.h"
#include "sonoria/streaming/algorithms/poolstorage.h"
#include "sonoria/streaming/algorithms/vectorinput.h"

using namespace std;
using namespace sonoria;
using namespace sonoria::streaming;

class MusicDescriptorSet { 

 public:
 	static const string nameSpace;  

 protected:
  Pool options;

};

#endif