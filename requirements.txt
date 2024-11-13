
Parameters:
    1. Text file name "TextFile"
    2. Semaphore table size "M"

Loop start:
    In that order:
        Increase current time
        From Command File:
            Start new processes
            Terminate processes? // maybe termination should happen before spawn so the M running check is easier
        Send randomly selected line from Text file 
    Repeat
    We expect process to print message sent 



Parent P:
    creates child proccesses
        processes are created BEFORE anything else happens in current loop
    reads Command File CF
    reads Text file 
    P creates Semaphore Table up to size M
        M is also the number of max num of children processes that can be running at any time
    P receives exit codes from terminated processes THE STEP AFTER T IS CALLED


Child C_i:
    owns a semaphore
        it should be freed afterwards so it can be reused
    uses shared memory 
        to read the messages received
    receives messages from P and prints them to stdout
    if no message received stay blocked // semaphore signal or something like that
    must receive termination message somehow // maybe special exit message code through shared memory    
    must send exit() code upon termination
        must print to stdout num of messages received during lifetime 
        must print to stdout total time spent alive (curr_time - creation_time )
        

Command File:
    Txt file with lines with structure:
        "Timestamp - Process - Command"
    Timestamp is int which shows when a command should be ran (time==timestamp)
    Process is which process to signal to
    Command is S (SPAWN) or T(TERMINATE)
        SPAWN: Parent P creates a new process
        TERMINATE: Parent P terminates Process



