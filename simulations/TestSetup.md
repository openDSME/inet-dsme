- Without Interference
	- run the file 'Without_Interference_Q_Learning_Normal_MCTS_10_150m.ini'
	
- With Interference
	- run 'With_Interference_Q_Learning_Normal_32Nodes.ini'
	- for the test of the standard implementation, where only 0.01 mW of transmission power is used, the file of the radio needs to be changed. The file is in inet in 'inet/src/inet/physicallayer/ieee802154/packetlevel/Ieee802154NarrowbandScalarRadio.ned'.
	Here the line '''
	transmitter.power = default(2.24mW);
	''' has to be changed to '''
	transmitter.power = default(0.01mW);
	'''
	- The same needs to be done, if the energy detection should be tested with -10dBm. Here the part '''
	receiver.energyDetection = default(-90dBm);
	''' has to be changed to '''
	receiver.energyDetection = default(-10dBm);
	''', this is the same file as before.
	
- Testing the Options of the Q-Learning algorithm
	- use 'Q_Learning_test_epsilon_greedy.ini' to test the difference between Q-Learning with and without epsilon-greedy
	- use 'Q_Learning_test_hotbooting.ini' to test the difference between Q-Learning with and without hotbooting
	- use 'Q_Learning_test_state_transitions.ini' to test the difference between the state transitions when using Q-Learning
