#include "BlackGPIO.h"

#include <iostream>
#include <string>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <iomanip>
#include <time.h>
#include <sys/types.h>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#define PATH_ADC "/sys/bus/iio/devices/iio:device0/in_voltage"

pid_t processGame;

// #include "functions.h"

int seed(int i) {
	srand (time(NULL));
	return std::rand()%i + 1;
}

int readAnalog(int number) {
	std::stringstream ss;
	ss << PATH_ADC << number << "_raw";
	std::fstream fs;
	fs.open(ss.str().c_str(), std::fstream::in);
	fs >> number;
	fs.close();
	return number;
}

void endGame(int sig) {
	system("clear");
	std::cout << "\t >>> TIME IS OVER!! =(" << std::endl;
	std::cout << "\t >>> GAME OVER!" << std::endl;
	kill(processGame, SIGKILL);
}

int main(int argc, char * argv[]){
	
	// Declarations of things for the BBB:
	// Led that means that the game are run.
	BlackLib::BlackGPIO   ledRun(BlackLib::GPIO_50,BlackLib::output, BlackLib::SecureMode);
	// Button to confirm the player's try.
	BlackLib::BlackGPIO   button1(BlackLib::GPIO_115,BlackLib::input);
	// Led RGB.
	BlackLib::BlackGPIO   ledR(BlackLib::GPIO_60,BlackLib::output, BlackLib::SecureMode);
	BlackLib::BlackGPIO   ledG(BlackLib::GPIO_30,BlackLib::output, BlackLib::SecureMode);
	BlackLib::BlackGPIO   ledB(BlackLib::GPIO_51,BlackLib::output, BlackLib::SecureMode);
	// Turn off the RGB light (security case).
	ledR.setValue(BlackLib::low);
	ledG.setValue(BlackLib::low);
	ledB.setValue(BlackLib::low);

	// Aother important declarations:
	int timeLeft;
	
	// Process of the game.
	processGame = fork();
	// signal alarm
	// Number of reset times.
	// int resetTimes = 0;

	switch(processGame){
		// Error:
		case -1: 
		exit(1);
		// Sun process:
		case 0:{
// Turn on the led that means that the game are running.
		ledRun.setValue(BlackLib::high);	

			// Defining the size of the table.
		int tableSize = readAnalog(0);
		tableSize = (tableSize*150)/4096;

			// Generating the random number;
		int nRandom = seed(tableSize);
		int lastTry = 0;
		int score = 100;

		std::cout << "WELCOME TO LIGHT TABLE GAME 2!\nCreating a number between [1-" << tableSize << "]...\n\n";
		std::cout << "Tray your luck: Use the KNOB to guess a number...\n";
		std::cout << "\t >>> You have to try to guess the number\n\t >>> generated randomly in the given range.\n";
		std::cout << "\t PRESS THE BUTTON TO CONFIRM YOUR ATTEMPT\n";
		std::cout << "Initial score: " << score << std::endl;

		while(1){
			if(button1.getNumericValue()){
				if( score <= 0 ){
					std::cout << "GAME OVER!\n";
					std::cout << "The answer is: " << nRandom << std::endl;
					exit(1);
				}

				int number = readAnalog(1);
				number = ((number*tableSize)/4096) + 1;
				lastTry = number;
				std::cout << "\t >>> Your guess: " << number << std::endl;
				std::cout << "\t >>> Actual table size: " << tableSize << std::endl;
				std::cout << "\t NUMERO GERADO: " << nRandom << std::endl;
				if( number == nRandom ){
					ledG.setValue(BlackLib::high);
					std::cout << "CONGRATULATIONS!!! YOU WIN!\n";
					std::cout << ">>> Final score: " << score << std::endl;
					sleep(3);
					ledG.setValue(BlackLib::low);
					kill(getppid(), SIGKILL);
					kill(getpid(), SIGKILL);
					exit(1);
				}else if( number >= (nRandom - 5) && number <= (nRandom + 5) ){
					ledB.setValue(BlackLib::high);
					sleep(1);
					std::cout << "Try again...\n";
					ledB.setValue(BlackLib::low);
					sleep(1);
					system("clear");
					score -= 7;
				}else{
					ledR.setValue(BlackLib::high);
					sleep(1);
					std::cout << "Try again...\n";
					ledR.setValue(BlackLib::low);
					sleep(1);
					system("clear");
					score -= 2;
				}
				std::cout << "Last Attempt: " << lastTry << std::endl;
				std::cout << "SCORE: " << score << std::endl;
			}
		}
		ledRun.setValue(BlackLib::low);
		}break;
		// Father process:
		default:{
		signal(SIGALRM, endGame);
		timeLeft = alarm(60);
		sleep(60);
		}
		break;
	}
	return EXIT_SUCCESS;
}
