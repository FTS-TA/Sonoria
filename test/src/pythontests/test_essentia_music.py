# Copyright (C) 2006-2021  Music Technology Group - Universitat Pompeu Fabra
#
# This file is part of Essentia
#
# Essentia is free software: you can redistribute it and/or modify it under
# the terms of the GNU Affero General Public License as published by the Free
# Software Foundation (FSF), either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the Affero GNU General Public License
# version 3 along with this program. If not, see http://www.gnu.org/licenses/

#!/usr/bin/python


def test():
  try:
    import sonoria
    import os
    #from sonoria.extractor import sonoria_music
    from sonoria import sonoria_extractor
    #options, args = sonoria_extractor.parse_args()
    #exec('options = ' + str(options))
    sonoria_extractor.compute('music', "../../audio/recorded/britney.wav","foo.sig")
    os.unlink("foo.sig")
    return 0
  except:
    raise
    print("Failed to run sonoria_music")
    return 1
