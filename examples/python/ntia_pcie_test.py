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

import ntia_pcie_api as ntia

NEURON_COMPONENTS = 256
TEST_VECTOR_SIZE  = 5

### values for classifiers
CLASS_RBF         =              (0x0000)
CLASS_KNN         =              (0x0001)
### values for metrics
METRICS_L1        =              (0x0000)
METRICS_LSUP      =              (0x0001)
### values for modes
MODE_SR           =              (0x0010)
MODE_NR           =              (0x0000)
### influence fields default values
DEF_MAXIF         =              (0x4000)
DEF_MINIF         =              (0x0002)




def main():
  pcie_card = ntia.ntia_pcie_card()

  print('--- init system ----------------------')
  pcie_card.sys_init()

  if pcie_card.cards_count < 1:
    print('NTIA PCIe card(s) not found')
    pcie_card.sys_deinit()
    quit(1)

  print('PCIe card(s) found = {0}'.format(pcie_card.cards_count))
  device_list = pcie_card.card_list
  print(device_list)

  print('--- open device ----------------------')
  pcie_card.device_open(device_list[0]['pci_bus'],
                        device_list[0]['pci_slot'],
                        device_list[0]['pci_func'])
  print('PCIe card handle   = {0}'.format(pcie_card.handle))
  print('neurons count/comm = {0}/{1}'.format(pcie_card.neurons_overall, pcie_card.neurons_committed))

  pcie_card.nn_reset()

  v    = bytearray([0]*256)

### ------------------
  print('--- learn V#1 ------------------------')
  v[0] = 1
  v[1] = 1
  v[2] = 1
  v[3] = 1
  v[4] = 1

  pcie_card.nn_vector_learn(METRICS_LSUP, 1, 1, 6, DEF_MINIF, TEST_VECTOR_SIZE,  v)
  print('neurons count/comm = {0}/{1}'.format(pcie_card.neurons_overall, pcie_card.neurons_committed))

### ------------------

  print('--- learn V#2 ------------------------')
  v[0] = 6
  v[1] = 6
  v[2] = 6
  v[3] = 6
  v[4] = 6

  pcie_card.nn_vector_learn(METRICS_LSUP, 1, 2, 6, DEF_MINIF, TEST_VECTOR_SIZE,  v)
  print('neurons count/comm = {0}/{1}'.format(pcie_card.neurons_overall, pcie_card.neurons_committed))

### ------------------

  print('--- classify V#1 ---------------------')
  v[0] = 5
  v[1] = 5
  v[2] = 5
  v[3] = 5
  v[4] = 5

  classify_result = pcie_card.nn_vector_classify(METRICS_LSUP, 1, CLASS_KNN, 9, TEST_VECTOR_SIZE,  v)
  print('classify result    = {0}'.format(classify_result))

  knowledge_base = []
  neuron_state = dict()

#  print('--- single neuron read ---------------')
#  neuron_state.clear()
#  neuron_state = pcie_card.nn_neuron_read(1)
#  print('neoron_state result    = {0}'.format('OK'))
#  print(' context    = {0}'.format(neuron_state['context']))
#  print(' category   = {0}'.format(neuron_state['category']))
#  print(' comps      = {0}'.format(len(neuron_state['comp'])))
#  print(' {0}'.format(neuron_state))

  print('--- KB store to memory ---------------')
  for ix in range(0, pcie_card.neurons_committed) :
    neuron_state = pcie_card.kbase_store()
    print(' ncr        = {0}'.format(neuron_state['ncr']))
    print(' category   = {0}'.format(neuron_state['category']))
    print(' comps      = {0}'.format(len(neuron_state['comp'])))
    print(' base = {0}'.format(neuron_state))
    knowledge_base.append(neuron_state)

  print('--- KB load from array ---------------')
  pcie_card.nn_reset()
  print(' NN soft reset - OK')
  print('neurons count/comm = {0}/{1}'.format(pcie_card.neurons_overall, pcie_card.neurons_committed))
  print(' load base')
  for ix in range(0, len(knowledge_base)) :
    pcie_card.kbase_load(knowledge_base[ix], TEST_VECTOR_SIZE)
  print('neurons count/comm = {0}/{1}'.format(pcie_card.neurons_overall, pcie_card.neurons_committed))


  print('--- classify V#2 - (after restore KB) -')
  v[0] = 5
  v[1] = 5
  v[2] = 5
  v[3] = 5
  v[4] = 5

  classify_result = pcie_card.nn_vector_classify(METRICS_LSUP, 1, CLASS_KNN, 9, TEST_VECTOR_SIZE,  v)
  print('classify result    = {0}'.format(classify_result))

###  quit
  pcie_card.device_close()
  pcie_card.sys_deinit()

if __name__ == "__main__":
  main()
