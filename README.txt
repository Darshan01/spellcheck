AUTHORS: Darshan Lakshminarayanan
NetIds: DL1058

---------------------------------------------USAGE---------------------------------------------

Usage:
    make
    ./spchk path/to/dictionary.txt path/to/fileOrDirectory [additional/files/or/directories]

Example:
    make
    ./spchk dict.txt testFile.txt dir1

Checks the spelling of the given files or all the text files in the given directories
by comparing against the words in the dictionary file (the first argument)

***The dictionary must be sorted lexicographically for optimal run time***

----------------------------------------FILE DESCRIPTIONS--------------------------------------

driver.c/h:
    takes the command line arguments, traverses them if they are directories, reads every word
    in all given files and text files in the directories

    stores the dictionary in an arraylist (alSTR.c)
    passes the words read into spell checking function

spell.c/h:
    takes a word and searches the dictionary file for a match

    if a match is not found, the incorrect word is printed
    along with its file, line number, and column number

arraylist.c/h:
    an arraylist of characters

alStr.c/h:
    an arraylist of strings

--------------------------------------------TEST FILES-----------------------------------------

myDict.txt:
    small dictionary with 25 words in lexicographic order
    and ASCII order with respect to capitalization

dict.txt:
    dictionary with 100k+ words in sorted the same as myDict.txt

test.txt:
    Tests the spelling of words with different capitalizations
    
    Tests the spelling of words with hyphens and other non alpha characters in between

    Tests regular words that are not in the dictionary

    Tests words with leading brackets and quotes and trailing non-letter characters

    Tests words with mixed capitalization

    Tests words with apostrophes

    Tests strings of non-alpha characters

    ./spchk dict.txt test.txt

gettysburg.txt:
    Tests standard english using the gettysburg address

    ./spchk dict.txt gettysburg.txt

dir1 and dir2:
    Tests directory traversal and if the program only reads .txt files in directories

    ./spchk dict.txt dir1 dir2

To run the program on all files and directories:
    ./spchk dict.txt test.txt dir1 dir2
