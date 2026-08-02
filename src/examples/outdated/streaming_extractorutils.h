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

#ifndef STREAMING_EXTRACTOR_METADATA_H
#define STREAMING_EXTRACTOR_METADATA_H

#include <essentia/algorithmfactory.h>
#include <essentia/pool.h>
#include <essentia/types.h>
#include <string>

void setDefaultOptions(sonoria::Pool& pool);
void setOptions(sonoria::Pool& options, const std::string& filename);
void mergeOptionsAndResults(sonoria::Pool& results, const sonoria::Pool& options);
void pcmMetadata(sonoria::streaming::AlgorithmFactory& factory,
                 const std::string& audioFilename, sonoria::Pool& pool);
void readMetadata(const std::string& audioFilename, sonoria::Pool& pool);

#endif // STREAMING_EXTRACTOR_METADATA_H
