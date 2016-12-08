/*
Program: main.cpp
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
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fstream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <exception>
#include <memory>

/*
Couldn't pass complex types like string through the Message queues
*/
#define ACC_NUM_SIZE 5
#define PIN_NUM_SIZE 3
//Maximum number of messages in the message queue
#define MAX_MSGS 10;


typedef struct general_message_struct{
  char accountNum[ACC_NUM_SIZE];
  char pin[PIN_NUM_SIZE];
}general_message_struct;
#define GEN_MSG_STRUCT_SIZE sizeof(general_message_struct);

//Pthread variables
pthread_t atm_thread;
pthread_t server_thread;
pthread_t db_editor_thread;
pthread_mutex_t atm_server_mutex;
pthread_mutex_t server_db_mutex;


#define ATM_SERVER_NAME "/pin_msg"
#define SERVER_DB_NAME "/db_msg"

//Initialize static message variables
static struct mq_attr msgq_attr;
static mqd_t atm_server_message = -1;
static mqd_t server_db_message = -1;

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

  general_message_struct sendMsg;
  general_message_struct rcvMsg;
  unsigned int msg_prio = 0;

  int timesAccessed = 0;
  string accountNumber;
  string pin;

  int atmStatus;

  //Get account Number
  cout << "ATM is executing: \n";
  while(accountNumber.size()!=5){
    cout << "Enter Account Number(5 Digits) or 'quit' to exit: ";
    cin >> accountNumber;
    cout << "You entered: " + accountNumber + "\n";
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
    cout << "You entered: " + pin + "\n";
    if(pin.size()!=3){
      cout << "Invalid PIN Format \n";
    }
  }
  //Encode the input to the message packet
  for(int i=0; i<ACC_NUM_SIZE; i++){
    sendMsg.accountNum[i] = accountNumber[i];
  }
  for(int i=0; i<PIN_NUM_SIZE; i++){
    sendMsg.pin[i] = pin[i];
  }


  //Send genMsg
  atmStatus = mq_send(atm_server_message, (const char*) &sendMsg, sizeof(sendMsg), msg_prio);
  //Check the send status
  if(atmStatus < 0){
    cout << "Sending a message via the message queue failed.";
    exit(0);
  }
  //Receive genMsg
  atmStatus = mq_receive(server_db_message, (char*) &rcvMsg, sizeof(rcvMsg), NULL);
  //Check the receive status
  if(atmStatus < 0){
    cout << "Receiving a message via the message queue failed ";
    exit(0);
  }

}

int main(int args, char const *argv[]){

  pthread_attr_t attr;

  //Initialize Message Queue VARS
  msgq_attr.mq_maxmsg = MAX_MSGS;
  msgq_attr.mq_msgsize = (size_t) GEN_MSG_STRUCT_SIZE;
  //Initialize Message Queues
  atm_server_message = mq_open(ATM_SERVER_NAME, O_CREAT | O_RDWR, 0666, &msgq_attr);
  server_db_message = mq_open(SERVER_DB_NAME, O_CREAT | O_RDWR, 0666, &msgq_attr);
  //Check if MSG Queue were initialized
  if(atm_server_message == -1){
    cout << "ERROR: ATM<->SERVER message queue failed to Initialize.\n";
  }
  if(server_db_message == -1){
    cout << "ERROR: SERVER<->DB message queue failed to Initialize.\n";
  }

  //Initialize Threads


}
