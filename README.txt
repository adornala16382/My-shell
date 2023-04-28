Aryan Dornala
ad1496

I am submitting by myself.

Extensions implemented: Home directory and Directory wilcards

Test Plan:

    (a) - What properties your library must have for you to consider it correct

            - ability to run in either BATCH_MODE or INTERACTIVE_MODE
            - Ability to run built-in commands cd,pwd, and exit and non built-in commands
            - (cd and cd ~) will send you to the HOME directory
            - Pipes work properly
            - Wildcards work properly
            - Redirection works properly
            - multiple wildcards in any part of a path
            - Errors get thrown appropriately

    (b) - How you intend to check that your code has these properties

            - Create additional C files which can be used by testing to read input and provide output in various ways.
            BATCH_MODE:
            - Create a test.txt file containing a list of commands for mysh to execute in BATCH_MODE that test the properties
                mentioned above.
            - Check the output and make sure each command does the expected output.
            INTERACTIVE_MODE:
            - Run the same commands from test.txt sequentially.
                - Check that the output of each command does the expected output.
                - Additionally, make sure that everytime a command fails, an error pops up and '!' shows up before the prompt 'mysh>'
    
    (c) - Description of additional C files

            [foo.c]
            - Reads standard input and outputs up to 300 characters read from the input with the text "You entered:" appearing before.
            [bar.c]
            - Outputs "BAR: " followed by the first argument it is provided.
            [baz.c]
            - Outputs all arguments provided in the format "[argv[i]]" in a single line and on a new line it outputs 
                "BAZ: " followed with up to 300 characters read from standard input. 
    
    (d) - Step by step explanation of how to test

            - Ensure that you run:
                ~ make mysh
                ~ make bar
                ~ make foo
                ~ make baz

            Testing BATCH_MODE:
                - Run [./mysh test.txt]
                - Now we must look at the output along with additional files created from running the test and ensure 
                    that all the properties listed above work as intended.
                
                TEST CASES EXPLAINED:
                 - echo hi
                    Should output "hi". Tests that a non-built-in command works as the program will look through set 
                    directories for the program.
                 - ls
                    Should output a list of all the directories and files under the current directory. 
                    Tests that a non-built-in command works as the program will look through set directories for the program.
                 - nonExistentCommand
                    Should give an error. Tests that an error is thrown when an unknown command is given.
                 - pwd
                    Should output the current directory. 
                    Tests that a built-in command works.    
                 - cd cdTest
                    Should change directories into a subdirectory called cdTest. 
                 - cd nonExistentDirectory
                    Should give an error. 
                    Tests that the cd built-in command can only go to directories that exist.
                 - pwd
                    Should output the directory with /cdTest at the end.
                    Tests that cd command worked by showing that the directory changed to the correct one.
                 - ls *.txt
                    Should output all files under the current directory that ends with .txt.
                    Tests that wildcards work.
                 - touch z*.c
                    Should create a file z*.txt.
                    Tests that if no matches are found for a wildcard, the command is executed with the *.
                 - cd ..
                    Should change directories into the parent directory.
                 - pwd
                    Should output the directory we started with.
                    Tests that cd command worked by showing that the directory changed to the correct one.                 
                 - ls */*.c
                    Should output all files that end with .c in any subdirectory of the current directory.
                    Tests that Directory wildcards work (an extension)
                 - echo test1 | foo
                    Should output "You entered: test1".
                    Tests that output from left side of the pipe is used as standard input for the right side of the pipe.
                 - cat *Write.c | foo
                    Should output the "You entered: " followed by the contents of noWrite.c.
                    Tests that wildcards work along with pipes.
                 - ls>file1.txt
                    file1.txt should contain a list of all the files/folders under the current directory.
                    Tests redirecting the output of the left command into the file on the right.
                 - foo<file1.txt
                    Should output "You entered: " followed by the contents of file1.txt.
                    Tests redirecting the contents of the file from the right side as standard input into the left command.
                 - chmod 404 noWrite.c
                    Disables write access to noWrite.c
                 - bar WRITING > noWrite.c
                    Should give an error.
                    Tests that redirecting output into a file that has write access should fail.
                 - chmod 664 noWrite.c
                    Enables write access to noWrite.c
                 - echo testing | foo>file3.txt
                    file3.txt should contain "You entered: testing".
                    Tests that piping works with redirection.
                 - foo < noWrite.c | foo>newFile.txt
                    newFile.txt should contain "You entered: You entered: " followed by the contents of noWrite.c.
                    Tests that piping work with redirection in both sides of the pipe.
                 - foo<noWrite.c|baz *.txt>file4.txt
                    file4.txt should contain a list in the format ["argv[i]"] of all files ending with .txt under the current directory
                    followed by "BAZ: You entered: " followed by the contents of noWrite.txt.
                    Tests that wildcards also works with piping and redirection.
                 - cd
                    Should change the current directory to the home directory.
                 - pwd
                    Should output the Home directory
                    Tests that changing to the Home directory works. (an extension)
                 - cd nonExistentDirectory/~
                    Should change the current directory to the home directory.
                 - pwd
                    Should output the Home directory
                    Tests that a directory with ~ represents the Home directory.(an extension)               
                 - exit
                    Should output "exiting... " and exits the program mysh.
                    Tests that the built-in exit command works.
            
            Testing INTERACTIVE_MODE:
                - Run [./mysh]
                - Execute every command listed in test.txt one at a time.
                - Compare the behavior of each command to the descriptions provided in the [TEST CASES EXPLAINED] section above and
                    make sure they match. 
                - Only difference now is when an error is thrown, the following prompt should be '!mysh> '.
                - When a command executes without any errors, the following prompt should be reset to 'mysh> '.
                










