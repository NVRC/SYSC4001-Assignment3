/*
Program: atm.cpp
Authors:  -Benjamin Tobalt
          -Nathaniel Charlebois

Functionality:
    1. Requests an account number
    2. Requests a PIN
    3. Transmits these two values to the DB Server using a PIN message. If the DB server returns “NOT
        OK”, restart from 1.
    4. After the 3rd trial, “account is blocked” is displayed on screen.
    5. If the DB server returns “OK”, request the operation:
        a. display current funds: send a “Request Funds” message, waits for the funds available, and
            displays the number on screen
        b. funds to be withdrawn: sends a Withdraw message, together with the funds requested. If
            the funds available are not enough, the DB server returns a “Not Enough” message. In
            that case, “not enough funds” is displayed on screen. If there are enough funds, the DB
            Server resp
*/
using namespace std;
#include<stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


int timesAccessed = 0;
string accountNumber;
string pin;

//Get user input in the shell
void getUserInput(){
  //Get account Number
  while(accountNumber.size()!=5){
    cout << "Enter Account Number(5 Digits): ";
    cin >> accountNumber;
    if(accountNumber.size()!=5){
      cout << "Invalid Account format \n";
    }
  }
  //Get PIN number
  while(pin.size()!=3){
    cout << "Enter PIN: ";
    cin >> pin;
    if(pin.size()!=3){
      cout << "Invalid PIN Format \n";
    }
  }


}

int main(){
  getUserInput();
}

/*
class ATM {
  private:
    string accountNumber;

};
*/
