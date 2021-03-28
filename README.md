# customLinuxShell_v1

Run shellPaulCvasa.cpp with g++ shellPaulCvasa.cpp -lreadline , then run socketstart.cpp with g++ socketstart.cpp to make the connection and start the shell.

REQUIREMENTS (that I had to do)
------------
The following commands must be implemented, along with their parameters (see man help pages for information on each):

1. The "ls" command. Parameters that need to be implemented are: -l, -s, -a, -F

2. The "tac" command. Parameters that need to be implemented are: -b, -s

3. The "dirname" command. No parameters need to be implemented.

4. Your program must also support pipes in the commands, e.g. > ls -l | grep -e 'tmp' | wc -l (it must work with any type of command!)

5. Your program must also support redirection in the commands, e.g. > ls -l > out.txt (it must work with any type of command!)

6. Your program must support both pipes and redirection in the same command line, e.g. > ls -l | grep -e 'tmp' > out.txt (it must work with any type of command!)
