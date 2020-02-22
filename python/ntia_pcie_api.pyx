### language: cython
# cython: language_level=3
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

cimport ntia_pcie_def

from libc.stdint cimport uint8_t, uint16_t, uint32_t, uint64_t, uintptr_t
from libc.stddef cimport size_t
from libc.string cimport memset

### python high level interface

cdef class ntia_pcie_card:

  cdef ntia_pcie_def.nta_dev_handle_t  _c_dev_handle
  cdef ntia_pcie_def.nta_pcidev_list_t _c_dev_list

  def __init__(self):
    self._c_dev_handle._iox_handle = NULL
    self._c_dev_handle._signature  = 0

  def __del__(self):
    ntia_pcie_def.ntpcie_device_close(&self._c_dev_handle)
    ntia_pcie_def.ntpcie_sys_deinit(&self._c_dev_handle)
    self._c_dev_handle._iox_handle = NULL
    self._c_dev_handle._signature  = 0

### access to internal structs

  @property
  def handle(self):
    return <uintptr_t>self._c_dev_handle._iox_handle
  @handle.setter
  def handle(self, value):
    raise AttributeError("handle: can't set attribute")

  @property
  def cards_count(self):
    return <int>self._c_dev_list.devs_count
  @cards_count.setter
  def cards_count(self, value):
    raise AttributeError("cards_count: can't set attribute")

  @property
  def card_list(self) -> list:
    _p_card_list = []
    for ix in range(0, self._c_dev_list.devs_count):
      _p_card_list.append(dict(pci_bus   = self._c_dev_list.devices[ix].bus,
                               pci_slot  = self._c_dev_list.devices[ix].slot,
                               pci_func  = self._c_dev_list.devices[ix].func))
    return _p_card_list
  @card_list.setter
  def card_list(self, value):
    raise AttributeError("card_list: can't set attribute")

  @property
  def neurons_overall(self):
    return <int>self._c_dev_handle.nn_state.neurons_overall
  @neurons_overall.setter
  def neurons_overall(self, value):
    raise AttributeError("neurons_overall: can't set attribute")

  @property
  def neurons_committed(self):
    return <int>self._c_dev_handle.nn_state.neurons_committed
  @neurons_committed.setter
  def neurons_committed(self, value):
    raise AttributeError("neurons_committed: can't set attribute")

  @property
  def base_id(self):
    return <int>self._c_dev_handle.nn_state.kbase_id
  @base_id.setter
  def base_id(self, value):
    self._c_dev_handle.nn_state.kbase_id = <uint64_t>value

# TODO: change const value to ...
  @property
  def components_max(self):
    return <int>256
  @components_max.setter
  def components_max(self, value):
    raise AttributeError("components_max: can't set attribute")

### translated methods call

## system functions
  def sys_init(self) -> None:
    result = ntia_pcie_def.ntpcie_sys_init(&self._c_dev_handle, &self._c_dev_list)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_sys_init: return not SUCCESS")

  def sys_deinit(self) -> None:
    result = ntia_pcie_def.ntpcie_sys_deinit(&self._c_dev_handle)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_sys_deinit: return not SUCCESS")

## device functions
  def device_reset(self) -> None:
    result = ntia_pcie_def.ntpcie_device_reset(&self._c_dev_handle)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_device_reset: return not SUCCESS")

  def device_open(self, pci_bus: int, pci_slot: int, pci_func: int) -> None:
    result = ntia_pcie_def.ntpcie_device_open(&self._c_dev_handle, pci_bus, pci_slot, pci_func)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_device_open: return not SUCCESS")

  def device_close(self) -> None:
    result = ntia_pcie_def.ntpcie_device_close(&self._c_dev_handle)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_device_close: return not SUCCESS")

### NN functions
  def nn_reset(self) -> None:
    result = ntia_pcie_def.ntpcie_nn_reset(&self._c_dev_handle)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_nn_reset: return not SUCCESS")

### vector learn/classify functions
## learn function
  def nn_vector_learn(self, dist_eval: int, context: int, category: int, maxif: int, minif: int, comps: int, vector: bytearray) -> None:
    if dist_eval != 0 and dist_eval !=1:
      raise ValueError("dist_eval: valid values is 0=L1 or 1=Lsup")
    if context < 1 or context > 127:
      raise ValueError("context: valid range is [1..127]")
    if category < 0 or category > 32766:
      raise ValueError("category: valid range is [0..32766]")
    if maxif < 0 or maxif > 65535:
      raise ValueError("maxif: valid range is [0..65535]")
    if minif < 0 or minif > 65535:
      raise ValueError("minif: valid range is [0..65535]")
    if comps < 1 or comps > 256:
      raise ValueError("comps: valid range is [1..256]")

    result = ntia_pcie_def.ntpcie_nn_vector_learn(&self._c_dev_handle, dist_eval, context, category, maxif, minif, comps, vector)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_nn_vector_learn: return not SUCCESS")

## classify function
  def nn_vector_classify(self, dist_eval: int, context: int, classifier: int, answers: int, comps: int, vector: bytearray) -> list :
    if dist_eval != 0 and dist_eval !=1:
      raise ValueError("dist_eval: valid values is 0=L1 or 1=Lsup")
    if context < 1 or context > 127:
      raise ValueError("context: valid range is [1..127]")
    if classifier != 0 and classifier != 1:
      raise ValueError("classificator: valid values is 0=RBF or 1=KNN")
    if answers < 1 or answers > 12:
      raise ValueError("answers: valid range is [1..12]")
    if comps < 1 or comps > 256:
      raise ValueError("comps: valid range is [1..256]")

    cdef size_t _c_number_of_responses

    cdef ntia_pcie_def.response_neuron_state_t _c_response[12]

    _c_number_of_responses = answers
    result = ntia_pcie_def.ntpcie_nn_vector_classify(&self._c_dev_handle, dist_eval, context, classifier,
                              comps, vector,
                              &_c_number_of_responses, _c_response)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_nn_vector_classify: return not SUCCESS")

    result_out = []
    for ix in range(0, _c_number_of_responses) :
      result_out.append(dict(distance = _c_response[ix].distance,
                             category = _c_response[ix].category,
                             id       = _c_response[ix].id))
    return result_out

## neuron state/KB manage functions
  def nn_neuron_read(self, n_number: int) -> dict:
    cdef ntia_pcie_def.nn_neuron_t _c_neuron
    cdef size_t ix
    if n_number < 0 or n_number+1 > self._c_dev_handle.nn_state.neurons_committed :
      raise AttributeError("n_number: valid range of values = [0..neurons_committed-1]")
    result = ntia_pcie_def.ntpcie_nn_neuron_read(&self._c_dev_handle, n_number, &_c_neuron)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_neuron_read: return not SUCCESS")
    comps = bytearray([0]*256)
    # ATTT: manual copy for prevent stop at '\0' byte
    for ix in range(0, 256) :
      comps[ix] = _c_neuron.comp[ix]
    return dict(opcode      = 0,
                ncr         = _c_neuron.ncr,
                category    = _c_neuron.category,
                aif         = _c_neuron.aif,
                minif       = _c_neuron.minif,
                comp        = comps)


  def kbase_store(self) -> dict:
    cdef ntia_pcie_def.nn_neuron_t _c_neuron
    cdef size_t ix
    result = ntia_pcie_def.ntpcie_kbase_store(&self._c_dev_handle, &_c_neuron)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_kbase_store: return not SUCCESS")
    comps = bytearray([255]*256)
    # ATTT: manual copy for prevent stop at '\0' byte
    for ix in range(0, 256) :
      comps[ix] = _c_neuron.comp[ix]
    return dict(opcode      = 0,
                ncr         = _c_neuron.ncr,
                category    = _c_neuron.category,
                aif         = _c_neuron.aif,
                minif       = _c_neuron.minif,
                comp        = comps)

  def kbase_load(self, _neuron_state: dict, comps_count: int) -> None:
    cdef ntia_pcie_def.nn_neuron_t _c_neuron
    cdef int ix
    if comps_count < 1 or comps_count > 256 :
      raise AttributeError("comps_count: valid range of values = [1..256]")

    _c_neuron.opcode    = 0
    _c_neuron.ncr       = _neuron_state['ncr']
    _c_neuron.category  = _neuron_state['category']
    _c_neuron.aif       = _neuron_state['aif']
    _c_neuron.minif     = _neuron_state['minif']

    # ATTT: manual copy for prevent stop at '\0' byte
    for ix in range(0, comps_count) :
      _c_neuron.comp[ix] = _neuron_state['comp'][ix]

    result = ntia_pcie_def.ntpcie_kbase_load(&self._c_dev_handle, comps_count, &_c_neuron)
    if result != ntia_pcie_def.ntpcie_nn_error_t.NTPCIE_ERROR_SUCCESS :
      raise RuntimeError("ntpcie_kbase_load: return not SUCCESS")
    return
