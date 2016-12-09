/*
TERMINAL COMPILE CODE: g++ -o main main.cpp -pthread -lrt -Wall


Program: main.cpp
Authors:  -Benjamin Tobalt
          -Nathaniel Charlebois

ATM Functionality:
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

    The ATM component then ends, waiting for the next customer (we will use it to terminate the program: if
    you use the string “X”, the program ends.

The DB server does the following:
  1. Wait messages forever
  2. If the message is a PIN message, it gets the account number and the PIN. It searches the account
      number in the DB. It then subtracts 1 to the PIN number, and compares with the one stored in the
      DB. If the account number is found and the PIN is correct, return an “OK’ message, and saves the
      information on the account in a local variable. If there is a failure, returns a “NOT OK” message.
      After the third attempt, the account is blocked. To do so, the first digit in the account number is
      converted to an “X” character.
  3. If the message is a “Request Funds” message, get the Funds field, and return it.
  4. If the message is a “Withdraw” message, get the amount requested, and check the funds available.
      If there is enough money into the account, return “Enough funds”, decrement the funds available,
      and update the file. If there is not enough money, return “Not enough funds”
  5. If an “Update DB” message is received, the updated information is obtained and saved to the file
      (adding 1 to each position of the PIN).

The DB Editor will:
  1. Request an account number
  2. Request a PIN.
  3. Request a value representing the funds available
  4. Send this information with an “Update DB” message to the DB
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
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <exception>
#include <memory>
#include <cstring>

/*
Couldn't pass complex types like string through the Message queues
*/
#define ACC_NUM_SIZE 5
#define PIN_NUM_SIZE 3
#define RESP_NUM_SIZE 8
//Maximum number of messages in the message queue
#define MAX_MSGS 10

typedef struct rcv_message_struct{
  char response[RESP_NUM_SIZE];
}single_message_struct;
#define RCV_MSG_SIZE sizeof(rcv_message_struct);

typedef struct gen_message_struct{
  char accountNum[ACC_NUM_SIZE];
  char pin[PIN_NUM_SIZE];
}gen_message_struct;
#define SEND_MSG_SIZE sizeof(gen_message_struct);

typedef struct update_message_struct{
  char accountNum[ACC_NUM_SIZE];
  char pin[PIN_NUM_SIZE];
  float amount;
}update_message_struct;
#define UPDATE_MSG_SIZE sizeof(update_message_struct);

struct account_template{
  char accNum[ACC_NUM_SIZE];
  char pinNum[PIN_NUM_SIZE];
  float amount;
};

struct account_template accounts[MAX_MSGS];
int accounts_size = 0;

//Pthread variables
pthread_t atm_thread;
pthread_t server_thread;
pthread_t db_editor_thread;
pthread_mutex_t atm_server_mutex;
pthread_mutex_t server_db_mutex;


#define ATM_SERVER_NAME "/pin_msg"
#define SERVER_DB_NAME "/db_msg"
#define UPDATE_NAME "/up_msg"

//Initialize static message variables
static struct mq_attr msgq_attr_send;
static struct mq_attr msgq_attr_rcv;
static struct mq_attr msgq_attr_update;
static mqd_t atm_server_message = -1;
static mqd_t server_db_message = -1;
static mqd_t update_message = -1;

void serverPrint(string printInput, int clearFlag);

void parseDatabase(struct account_template accountArray[]){
  const char* databaseFile = "database.txt";

  int numberOfLines = 0;

  char tempACC[5];
  //char tempACC[ACC_NUM_SIZE];
  char tempPIN[3];
  char tempAMOUNT[13];
  string lineCounter;
  int contextSwitch=0;

  std::ifstream numFile("database.txt");

  while (std::getline(numFile, lineCounter))
      ++numberOfLines;
  numFile.close();

  accounts_size = numberOfLines;


  std::ifstream myfile("database.txt");
  for(int i=0; i<numberOfLines*3; i++){
    if(contextSwitch == 0){
      myfile.getline(tempACC,ACC_NUM_SIZE+1,',');
      for(int k=0; k<ACC_NUM_SIZE;k++){
        accounts[i/3].accNum[k] = tempACC[k];
      }
    }
    else if(contextSwitch == 1){
      myfile.getline(tempPIN,PIN_NUM_SIZE+1,',');
      for(int k=0; k<PIN_NUM_SIZE;k++){
        accounts[i/3].pinNum[k] = tempPIN[k];
      }
    }
    else if(contextSwitch == 2){
      myfile.getline(tempAMOUNT,13,'\n');
      accounts[i/3].amount = atof(tempAMOUNT);

    }
    if(contextSwitch == 2){
      contextSwitch = 0;
    }
    else{
      contextSwitch++;
    }
  }
  myfile.close();

}

void writeDatabase(){
  ofstream database;
  const char* databaseFile = "database.txt";
  database.open(databaseFile, std::ofstream::out | std::ofstream::trunc);
  database.precision(13);
  for(int i=0; i<accounts_size; i++){
    string tempACC = string(accounts[i].accNum,ACC_NUM_SIZE);
    string tempPIN = string(accounts[i].pinNum,PIN_NUM_SIZE);
    database << tempACC << "," << tempPIN << "," << accounts[i].amount << endl;

  }

  database.close();
}

void serverPrint(string printInput, int clearFlag){
  fstream serverLog;
  const char* serverLogFile = "serverLog.txt";
  if(clearFlag == 1){
    serverLog.open(serverLogFile, std::ofstream::out | std::ofstream::trunc);
  }
  else{
    serverLog.open(serverLogFile, fstream::app);
  }
  serverLog << printInput + "\n";
  serverLog.close();
}

void dbPrint(string printInput, int clearFlag){
  fstream dbLog;
  const char* dbLogFile = "dbLog.txt";
  if(clearFlag == 1){
    dbLog.open(dbLogFile, std::ofstream::out | std::ofstream::trunc);
  }
  else{
    dbLog.open(dbLogFile, fstream::app);
  }
  dbLog << printInput + "\n";
  dbLog.close();
}

void* server(void* args){
  //Clear the log and output the starting string
  serverPrint("Server is running.",1);
  parseDatabase(accounts);
  writeDatabase();
  //INIT VARS

  gen_message_struct receive_message;
  update_message_struct updaterMsg;
  rcv_message_struct send_message;
  char rvcd_account_num[ACC_NUM_SIZE];
  char currentPin[PIN_NUM_SIZE];

  int databaseInput = -1;


  while(1){

      databaseInput = msgrcv(update_message,(char*) &updaterMsg,
        sizeof(updaterMsg),0, IPC_NOWAIT);
      if(databaseInput > -1){
        dbPrint("Added a new account to the DB \n",0);
        //add account to the dataBase
        accounts_size ++;
        for(int i=0; i<ACC_NUM_SIZE; i++){
          accounts[accounts_size].accNum[i] = updaterMsg.accountNum[i];
        }
        for(int i=0; i<PIN_NUM_SIZE; i++){
          accounts[accounts_size].pinNum[i] = updaterMsg.pin[i];
        }
        accounts[accounts_size].amount = updaterMsg.amount;
      }



    databaseInput = mq_receive(atm_server_message, (char*) &receive_message,
      sizeof(receive_message), 0);
    //Check the receive status
    if(databaseInput == -1){
      serverPrint("Message queue failed to deliver the message",0);
    }

    serverPrint("Checking if the message is valid",0);
    //get the received data
    char enteredPin[PIN_NUM_SIZE];
    for(int i=0; i<PIN_NUM_SIZE; i++){
      enteredPin[i] = receive_message.pin[i];
    }

    char enteredAccount[ACC_NUM_SIZE];
    for(int i=0; i<PIN_NUM_SIZE; i++){
      enteredAccount[i] = receive_message.accountNum[i];
    }


    //decodes the pin
    enteredPin[2] =  enteredPin[2]-1;
    //sendMsg.accountNum[i] = accountNumber[i];
    //check array of accounts for the account.

    //parse the accounts
    for(int i = 0; i < MAX_MSGS; i++){
      //check if account exists
      if(accounts[i].accNum == enteredAccount){
        //check if accounts pin is correct
        for(int i=0; i<PIN_NUM_SIZE;i++){
          currentPin[i] = accounts[i].pinNum[i];
        }
        currentPin[2] = currentPin[2]-1;

        if(currentPin == enteredPin){

          memcpy(send_message.response, "OK", strlen("OK"));
          //send msg to atm so atm can update the account amount
          mq_send(atm_server_message, (const char*) &send_message, sizeof(send_message), 0);
          break;
        }
        else{
          memcpy(send_message.response, "NOT OK", strlen("NOT OK"));
          //send msg to atm so atm can update the account amount
          mq_send(atm_server_message, (const char*) &send_message, sizeof(send_message), 0);
          break;
        }
      }
    }









  }
}

void* db_editor(void* args){
  //Clear the log and output the starting string
  dbPrint("Database Editor is running.",1);

    //create an update msg for the atm
    update_message_struct updateMsg;

    unsigned int msg_prio = 0;
    string accountNumber;
    string pin;
    float balance;

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

        cout << "Enter Amount to Withdraw: \n";
        cin >> balance;
        cout << "You entered: " << balance << "\n";
        //Encode the input to the message packet
        for(int i=0; i<ACC_NUM_SIZE; i++){
          updateMsg.accountNum[i] = accountNumber[i];
        }
        for(int i=0; i<PIN_NUM_SIZE; i++){
          updateMsg.pin[i] = pin[i];
        }
        updateMsg.amount = balance;

    //send msg
    mq_send(update_message, (const char*) &updateMsg, sizeof(updateMsg), msg_prio);
}


void* atm(void* args){

  gen_message_struct sendMsg;
  rcv_message_struct rcvMsg;
  unsigned int msg_prio = 0;

  int timesAccessed = 0;
  string accountNumber;
  string pin;

  int atmInput;

  //Get account Number
  cout << "ATM is executing: \n";
  while(timesAccessed < 3){
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
    atmInput = mq_send(atm_server_message, (const char*) &sendMsg, sizeof(sendMsg), msg_prio);
    //Check the send status
    if(atmInput == -1){
      cout << "Sending a message via the message queue failed.";
      exit(0);
    }
    //Receive genMsg
    cout << "Waiting for the server response.";
    atmInput = mq_receive(server_db_message, (char*) &rcvMsg, sizeof(rcvMsg), 0);
    //Check the receive status
    if(atmInput == -1){
      cout << "Receiving a message via the message queue failed ";
      exit(0);
    }
    cout << "Received a response.";

    if(rcvMsg.response == "OK"){
      cout << "Select an option (withdraw) or (display):";

      
    }
    else if(rcvMsg.response == "NOT OK"){

    }
    else{

    }

}
//3 Invalid Tries
cout << "Account is blocked.";

}

int main(int args, char const *argv[]){

  //Thread VARS
  long start_condition = 0;
  pthread_attr_t attr;

  //Initialize Message Queue VARS
  msgq_attr_send.mq_maxmsg = MAX_MSGS;
  msgq_attr_send.mq_msgsize = (size_t) SEND_MSG_SIZE;
  msgq_attr_rcv.mq_maxmsg = MAX_MSGS;
  msgq_attr_rcv.mq_msgsize = (size_t) RCV_MSG_SIZE;
  msgq_attr_update.mq_maxmsg = MAX_MSGS;
  msgq_attr_update.mq_msgsize = (size_t) UPDATE_MSG_SIZE;
  //Initialize Message Queues
  atm_server_message = mq_open(ATM_SERVER_NAME, O_CREAT | O_RDWR, 0666, &msgq_attr_send);
  server_db_message = mq_open(SERVER_DB_NAME, O_CREAT | O_RDWR, 0666, &msgq_attr_rcv);
  update_message = mq_open(UPDATE_NAME, O_CREAT | O_RDWR, 0666, &msgq_attr_update);

  //Check if MSG Queue were initialized
  if(atm_server_message == -1){
    cout << "ERROR: ATM<->SERVER message queue failed to Initialize.\n";
  }
  if(server_db_message == -1){
    cout << "ERROR: SERVER<->DB message queue failed to Initialize.\n";
  }
  if(update_message == -1){
    cout << "ERROR: DB<->SERVER message queue failed to Initialize.\n";
  }

  //Initialize Threads
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr,1024*1024);
  pthread_create(&atm_thread, NULL, atm, (void*) start_condition);
  pthread_create(&server_thread, NULL, server, (void*) start_condition);
  pthread_create(&db_editor_thread, NULL, db_editor, (void*) start_condition);

  //Run Threads
  pthread_join(atm_thread, NULL);
  pthread_join(server_thread, NULL);
  pthread_join(db_editor_thread, NULL);
}
