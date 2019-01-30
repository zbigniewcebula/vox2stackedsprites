# vox2stackedsprites
Simple converter that breaks up VOX file into PNG layers for stacked layers technique

TODO:
* Add option (-iul / --ignoreunusedlayers flag?) to ignore last layers of emptiness (every pixel alpha is 0)
* Use LibPNG for Windows compilation
* Make sure that every limit is protected against overflow etc...

OPTIONAL TODO:
* Add "cpp" in/out format that is used by CImg library
* Multithreading?

Dependencies:
 - Linux
	* libx11-dev
	* libpng-dev
 - Windows
 	* libpng

[![license zlib](https://img.shields.io/badge/license-zlib-brightgreen.svg?style=flat)](http://zlib.net/zlib_license.html)

Copyright (C) [2019] [Rafał Romanowicz] All rights reserved.

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
