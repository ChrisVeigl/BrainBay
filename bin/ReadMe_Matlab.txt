This document briefly describes the BrainBay Matlab object by Lester John.
-------------------------------------------------------------------------

The matlab object works only with the commercial .dlls from mathworks,
and it needs a special brainbay - build. Please contact chris@shifz.org if you
want a a build that supports the matlab object.

Purpose:

To provide a real-time input/processing/output link between BrainBay and Matlab

Operation:

1. The operation of the Matlab object is similar to that of the Evaluator object.
2. A matlab script or function call is enterred into the Matlab Function Call edit box.
3. Up to 10 inputs (consecutively labelled A-J) may be connected
4. A single output is provided.

Matlab Function Call:

1. The matlab script may be a Matlab command line script; a Matlab function call; or a custom written Matlab functi.on 
2. Following the execution of the script, the "ans" variable is copied from the Matlab workspace to the BrainBay-Matlab object.
3. Note that if the inputs are vectors (ie. if an input buffer size > 1 has been chosen), the the Matlab "ans" variable will also be a vector. The entire "ans" vector is copied to BrainBay, however only the last element of the output vector is sent to the output of the BrainBay-Matlab object.

Input Variables:

1. Up to 10 input variables may be connected
2. The variables are labelled A-J consecutively
3. These variables may be buffered up to 200 samples

Input buffers:

1. The Matlab object is able to buffer the input, according to user selection, by up to 200 samples
2. The length of the input buffer is set by the dialog box
3. The entire buffer of all the variables (A-J) is passed to Matlab.
4. The input buffer is initially set to all zeroes. 
5. The first sample is copied to the first element. 
6. Successive samples are copied to the 2nd, 3rd ... element etc.
7. Once the buffer is full, the buffer is shifted down and each new sample is copied to the top (ie. last element) of the buffer.

Additional variables:

1. Aside from the (up to 10) input variables, there are two automatically generated variables that are transferred to the Matlab workspace: "input_buffer_length" and "input_buffer_index"
2. These two variables may also be included in the function call
3. "input_buffer_length" contains the user selected length of the input buffers
4. "input_buffer_index" contains the index of the current sample. For buffers > 1, this will increase and then once the buffer is full it will clamp at the  maximum buffer length i.e. at "input_buffer_length". 

Output: 

1. If the inputs are vectors (ie. if an input buffer size > 1 has been chosen), the the Matlab "ans" variable will also be a vector. Whilst the entire "ans" vector is copied to BrainBay, only the last element of the output vector is sent to the output of the BrainBay-Matlab object.

To test a sample Matlab Script:

1. insert the Matlab Object in BrainBay
2. insert two signal generators objects (e.g 2 Hz and 50 Hz)
3. connect the two signal generator objects to inputs A, and B respectively of the Matlab object
4. enter the following script in the edit box of the Matlab object "brainbay_test(A,B)"
5. click the Apply button of the Matlab object dialog box
6. Connect the output of the Matlab object to the Oscilloscope object
7. Copy the matlab function brainbay_test.m into the Matlab Work directory (or any other suitable directory)
8. Run the BrainBay configuration.

Real-time operation:

1. The code is suitable for real-time operation with the only limitation being the effective speed of the PC, Matlab, and  matlab function combinatin being called. If the function takes too long to execute, then it cannot be used for realtime operation without the execution being decreased. Execution times may be decreased by rewriting the function and/or using a faster PC (processor, clockspeed, memory, etc.)


Known Bugs:

1. Load and Save configuration does not work yet.


--
LR John, University of Cape Town, 
3 July 2006