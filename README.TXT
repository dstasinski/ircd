                                _             _
                               (_)_ __ ___ __| |
                               | | '__/ __/ _` |
                               | | | | (_| (_| |
                               |_|_|  \___\__,_|

                       
                         ircd - minimalistic irc server

================================================================================

~~ About ~~

	Very simple irc server made as a project for a System programming class.
	Support small subset of RFC 1459:
		
		NICK, JOIN, WHO, PART, PRIVMSG, QUIT

	Should work fine with most clients but they won't be happy, since they
	won't receive responses to most of their queries.
	
~~ CLI ~~

	If CLI (getopt) is enabled during configuration and compilation you can use
	the following options:

    ircd [-v] [-a addr] [-p port]

        -v      Enable verbose (debug) output. Default is off.

        -a      Set the listening address. Default is INADDR_ANY.

        -p      Set the port port. Default is 6667.

~~ Build process ~~

    You can just compile and link all files in the project:
    
        $ gcc -std=c99 -c src/*.c src/commands/*.c
        $ gcc -o ircd *.o

    There is also a CMake configaration file that will produce the required
	makefile:

        $ cmake .
        $ make
   
    Extra options can be enabled/disabled with the CUI configurator:

        $ ccmake .

================================================================================
