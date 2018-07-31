# Integration of openDSME in OMNeT++/INET

This code integrates <a href="http://www.opendsme.org">openDSME</a> in the OMNET++ simulator by using the INET framework.

## Installation Instructions

These instructions are also <a href="https://www.youtube.com/watch?v=8BVK7qDsatY">available as video</a>.

1. <a href="https://www.youtube.com/watch?v=WwkLIbBmU6Q">Install OMNeT++</a>

2. Create a workspace

    ```
    mkdir ~/opendsme_workspace
    cd ~/opendsme_workspace
    ```

3. A fork of INET is required that can be cloned as

    ```
    git clone https://github.com/openDSME/inet.git --single-branch	
    git submodule update --init
    ```

4. Clone this repository and get the openDSME submodule

    ```
    git clone https://github.com/openDSME/inet-dsme.git --single-branch
    cd inet-dsme
    git submodule update --init
    cd ..
    ```

5. Open the OMNeT++ GUI and import the inet as well as the inet-dsme project.

6. Build all (Ctrl+B) 

