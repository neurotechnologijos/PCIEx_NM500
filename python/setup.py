# language: python
# -*- coding: utf-8 -*-

'''
/*
 * Copyright (c) 2017-2020 NeuroTechnologijos UAB
 * (https://www.neurotechnologijos.com)
 *
 * SPDX-License-Identifier: MIT
 *
 */
'''

from distutils.core import setup
from distutils.extension import Extension
from Cython.Build import cythonize

import os
import platform

name_e  =  "ntia_pcie_api"
sources = ["ntia_pcie_api.pyx"]
libs_e  = ["ntiaPCIe_static"]

if platform.system() == 'Windows':
  libs_e = libs_e + ["cfgmgr32"]

extensions = [
  Extension(name_e,
            sources,
            language='c',
            libraries=libs_e,
            library_dirs=["./", "../lib/", "../bin/"])
             ]
setup(
  ext_modules = cythonize(extensions)
)
