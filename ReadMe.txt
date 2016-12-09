Sysc 4001 Assignment 3
Due: 09/12/2016 @9:00pm

Benjamin Tobalt		
Nathaniel Charlebois	

For this assignment a simple prototype of a program that mimics the work of an ATM system using concurrent
processes and IPC services was to be built. The program contains four parts these part and a brief description 
of the programs components are given below. A diagram is also included with in the main submission folder
with a new system diagram showing how the program functions (this file is system_organization.png).

ATM:
The atm retrieves input from a user. The input is checked and only allows for valid input to be entered.
e.g The pin and account number must be the proper length. 
Once valid input is entered into the program the atm packages the user input into a message struct and sends them
to the server using a pthread. 

Server:
The server recieves message structs from the server and dbEditor. The server then performs the approriate action required.
For instance an account withdrawl will update the specified accounts balance in the database. As stated below it is not the 
dbEditors job to update the database this is performed by the server. 

database.txt:
This is the DATABASE for the program as stated in the assignment ouline it is a .txt file which contains a list of
accounts. Each new account starts on a new line in the file and follows the format below.

  ACCOUNT     PIN       ACCOUNT BALANCE 

	_ _ _ _ _ , _ _ _  , _ _ _ _ _ _ _ _ _ _ _ _ _

Please note that our program only supports up to 13 digits for the float account balance. This size was chosen since
the richest man in world has 42 billion dollars so it seemed reasonable. At this time we cannot currently support corporations.

dbEditor:
PLEASE NOTE: The code for dbEditor is in the program however since we couldn't get two consoles to run together the main 
function of the program does not activate the dbEditor. The goal was to create a new process however this was not able to 
implemneted.

The is part the atm program is a separte process from the rest of the program it runs concurrently with the ATM
to update the database's text file. This update could either be adding a new account to the database or modifying an 
already present account with a withdrawl or deposit. An account is added to the database when an account number
which is not already present in the database is entered into the dbEditors' console. It is important to note that 
the dbEditor does not actually change the database. This is performed by the Server the dbEditor sends the account information 
an pin to the server through the use of a pthread to add the account to the database text file.

TO RUN THE CODE:

	1) Find the directory the project files are located
	2) Type the following to run the program 

		g++ -o main main.cpp -pthread -lrt -Wall	to compile  
		./main 						to run
		
	3) This will launch the program and two consoles will appear. You can use these consoles to perform changes to 
	to the database. The atm console can perform withdrawls and request funds. The dbeditor console is able to update 
	and add new accounts to the database.
	4) To exit the program TYPE "quit"
	  



Included files:
Within this .zip file all the files for the assignment have been included. In the partI folder a word document containing the 
concept portion of the assignment is provided. In the partII folder the coding portion of the assignment is provided, only partII
of the coding has been submitted since it was stated in the assignment outline that this would be accepted without penalty. 
System_organization.png contains an image of how the atm program functions since the provided diagram in the assignment was not 
correct for our implementation.

