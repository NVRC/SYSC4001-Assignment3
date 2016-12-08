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
#include <exception>
#include <memory>

#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fstream>

typedef struct general_message_struct{
  string accountNum;
  string pin;
}general_message_struct;
#define gen_msg_struct_size = sizeof(general_message_struct);

//Pthread variables
pthread_t atm_thread;
pthread_t server_thread;
pthread_t db_editor_thread;
pthread_mutex_t atm_server_mutex;
pthread_mutex_t server_db_mutex;


#define ATM_SERVER_NAME "/pin_msg"
#define SERVER_DB_NAME "/db_msg"

static struct mq_attr msgq_attr;
static mqd_t general_message = -1;

void serverPrint(string printInput){
  fstream serverLog;
  serverLog.open("serverLog.txt", fstream::app);
  serverLog << printInput;
  serverLog.close();
}

void dbPrint(string printInput){
  fstream dbLog;
  dbLog.open("serverLog.txt", fstream::app);
  dbLog << printInput;
  dbLog.close();
}


void* atm(void* args){
  int timesAccessed = 0;
  string accountNumber;
  string pin;
  //Get account Number
  while(accountNumber.size()!=5){
    cout << "Enter Account Number(5 Digits) or 'quit' to exit: ";
    cin >> accountNumber;
    if(accountNumber == "quit"){
      cout << "";
      exit(EXIT_FAILURE);
    }
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

}
