REM append once.txt to header files in the current directory
REM
FOR %%f IN ( *.H ) DO copy %%f+once.txt
