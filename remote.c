// cc -o remote remote.c -lwiringPi -lm
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include <wiringPi.h>
#include <softPwm.h>

#include "controller.h"
#include <math.h>


struct ps3ctls {

	int fd;
	unsigned char nr_buttons;	// Max number of Buttons
	unsigned char nr_sticks;	// Max number of Sticks
	short *button;			// button[nr_buttons]
	short *stick;			// stick[nr_sticks]
};


#define NumberOfButton 17


int btn[NumberOfButton] = {};
int b_btn[NumberOfButton] = {};
int fds;
int mode = 10;

int l_mode = 3;
int l_mode_a = 60;//55
int l_mode_b = -80;
int r_mode = 3;
int r_mode_a = 72 ;
int r_mode_b = +90;
int t_mode = 0;
int h_mode = 12;
int d_mode = 1;
int lock = 0;
int petBottleLock = 0;
int megaPhonePush = 0;
int clawOn = 0;
int clawArc = +27;
int UpDown = 0;
int ready_Go = 0;

int grip = 0;

int btn_tri = 0;
int b_btn_tri = 0;
int btn_squ = 0;
int b_btn_squ = 0;
int btn_r1 = 0;
int b_btn_r1 = 0;


int resetPCA9685(int fd) {
	wiringPiI2CWriteReg8(fd,0,0);
}


int setPCA9685Freq(int fd , float freq) {
	float prescaleval;
	int prescale , oldmode , newmode;
	freq = 0.9 * freq;
	prescaleval = 25000000.0;
	prescaleval /= 4096.0;
	prescaleval /= freq;
	prescaleval -= 1.0;
	prescale = prescaleval + 0.5;
	oldmode = wiringPiI2CReadReg8(fd,0x00);
	newmode = (oldmode & 0x7F)|0x10;
	wiringPiI2CWriteReg8(fd , 0x00 , newmode);
	wiringPiI2CWriteReg8(fd , 0xFE , prescale);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode);
	sleep(0.005);
	wiringPiI2CWriteReg8(fd , 0x00 , oldmode | 0xA1);
}


int setPCA9685Duty(int fd , int channel , int off) {
	int channelpos;
	int on;

	on   = 0;
	off += 276;
	channelpos = 0x6 + 4 * channel;
	wiringPiI2CWriteReg16(fd , channelpos   , on  & 0x0FFF);
	wiringPiI2CWriteReg16(fd , channelpos+2 , off & 0x0FFF);
}


int ps3c_test(struct ps3ctls *ps3dat) {

	int i;
	unsigned char nr_btn = ps3dat->nr_buttons;
	unsigned char nr_stk = ps3dat->nr_sticks;
	int xx,yy,x,y,z,v1,v2,ww,c1,c2,c3,c4;


	printf("%d ",digitalRead(0));	// YELLOW TEST
    printf("\n");


	if((ps3dat->button[PAD_KEY_PS])&&(!ready_Go)) {
		ready_Go = 1;
		system("mpg123 /home/pi/Music/ready_Go.mp3");
	};
		
	if(ps3dat->button[PAD_KEY_UP]) {	// X
		setPCA9685Duty(fds , 0 , -50);
	} else if(ps3dat->button[PAD_KEY_DOWN]) {
		setPCA9685Duty(fds , 0 , +50);
	} else {
		setPCA9685Duty(fds , 0 ,   0);
	};

	if(ps3dat->button[PAD_KEY_LEFT]) {	// Y
		setPCA9685Duty(fds , 1 , -50);
	} else if(ps3dat->button[PAD_KEY_RIGHT]) {
		setPCA9685Duty(fds , 1 , +50);
	} else {
		setPCA9685Duty(fds , 1 ,   0);
	};

	if(ps3dat->button[PAD_KEY_L1]) {	// Z
		setPCA9685Duty(fds , 2 , +50);
	} else if(ps3dat->button[PAD_KEY_L2]) {
		setPCA9685Duty(fds , 2 , -50);
	} else {
		setPCA9685Duty(fds , 2 ,   0);
	};

	if(ps3dat->button[PAD_KEY_CIRCLE]) {		// OPEN
		grip = 1;
	} else if(ps3dat->button[PAD_KEY_CROSS]) {	// CLOSE
		grip = 0;
	};
	
	if(grip) {
		setPCA9685Duty(fds , 3 , +95);
	}else{
		setPCA9685Duty(fds , 3 , -95);
	};

	if(ps3dat->button[PAD_KEY_TRIANGLE]) btn_tri++;		// INIT
	if(!ps3dat->button[PAD_KEY_TRIANGLE]) btn_tri = 0;
	if(b_btn_tri > btn_tri) {
		setPCA9685Duty(fds , 0 , +50);
		setPCA9685Duty(fds , 1 , +50);
		setPCA9685Duty(fds , 2 , +50);
		setPCA9685Duty(fds , 3 , +95); grip = 1;
		delay(9000);
		setPCA9685Duty(fds , 0 , 0);
		setPCA9685Duty(fds , 1 , 0);
		setPCA9685Duty(fds , 2 , 0);
	};
	b_btn_tri = btn_tri;


	if(ps3dat->button[PAD_KEY_SQUARE]) btn_squ++;		// DROP
	if(!ps3dat->button[PAD_KEY_SQUARE]) btn_squ = 0;
	if(b_btn_squ > btn_squ) {
		setPCA9685Duty(fds , 0 , -50);
		setPCA9685Duty(fds , 1 , -50);
		setPCA9685Duty(fds , 2 , +50);
		setPCA9685Duty(fds , 3 , -95); grip = 0;// CLOSE
		delay(9000);
		setPCA9685Duty(fds , 0 , 0);
		setPCA9685Duty(fds , 1 , 0);
		setPCA9685Duty(fds , 2 , 0);
		setPCA9685Duty(fds , 3 , +95); grip = 1;// OPEN
	};
	b_btn_squ = btn_squ;


	if(ps3dat->button[PAD_KEY_R1]) btn_r1++;		// GAME
	if(!ps3dat->button[PAD_KEY_R1]) btn_r1 = 0;
	if(b_btn_r1 > btn_r1) {
		system("mpg123 /home/pi/Music/GAME_START.mp3");
		while(!digitalRead(0));
		delay(100);
		setPCA9685Duty(fds , 0 , -50);
		while(digitalRead(0)) delay(100);
		setPCA9685Duty(fds , 0 , 0);

		system("mpg123 /home/pi/Music/NEXT.mp3");
		while(!digitalRead(0));
		delay(100);
		setPCA9685Duty(fds , 1 , -50);
		while(digitalRead(0)) delay(100);
		setPCA9685Duty(fds , 1 , 0);

		system("mpg123 /home/pi/Music/NEXT.mp3");
		while(!digitalRead(0));
		delay(100);
		setPCA9685Duty(fds , 2 , -50);
		setPCA9685Duty(fds , 3 , +95); grip = 1;// OPEN
		while(digitalRead(0)) delay(100);
		setPCA9685Duty(fds , 2 , 0);
//		system("mpg123 /home/pi/Music/NEXT.mp3");

		setPCA9685Duty(fds , 3 , -95); grip = 0;// CLOSE
		delay(4000);

		setPCA9685Duty(fds , 2 , +50);
		delay(4000);
		
		setPCA9685Duty(fds , 0 , -50);
		setPCA9685Duty(fds , 1 , -50);
		setPCA9685Duty(fds , 2 , +50);
		setPCA9685Duty(fds , 3 , -95); grip = 0;// CLOSE
		delay(9000);
		setPCA9685Duty(fds , 0 , 0);
		setPCA9685Duty(fds , 1 , 0);
		setPCA9685Duty(fds , 2 , 0);
		setPCA9685Duty(fds , 3 , +95); grip = 1;// OPEN

		system("mpg123 /home/pi/Music/GAME_OVER.mp3");

	};
	b_btn_r1 = btn_r1;
	
//	if(digitalRead(0)) {setPCA9685Duty(fds , 0 , -50);}else{setPCA9685Duty(fds , 0 , 0);};

	if(ps3dat->button[PAD_KEY_START]) {
		system("mpg123 /home/pi/Music/Safe_mode.mp3 &");
		return -1; // end of program
	};

	return 0;

}	//	int ps3c_test(struct ps3ctls *ps3dat)


int ps3c_input(struct ps3ctls *ps3dat) {

	int rp;
	struct js_event ev;

	do {
		rp = read(ps3dat->fd, &ev, sizeof(struct js_event));
		if (rp != sizeof(struct js_event)) {
			return -1;
		}
	} while (ev.type & JS_EVENT_INIT);

	switch (ev.type) {
		case JS_EVENT_BUTTON:
			if (ev.number < ps3dat->nr_buttons) {
				ps3dat->button[ev.number] = ev.value;
			}
			break;
		case JS_EVENT_AXIS:
			if (ev.number < ps3dat->nr_sticks) {
				ps3dat->stick[ev.number] = ev.value / 200; // 327 range -32767 ~ +32768 -> -100 ~ +100
			}
			break;
		default:
			break;
	}

	return 0;
}


int ps3c_getinfo(struct ps3ctls *ps3dat) {

	if(ioctl(ps3dat->fd , JSIOCGBUTTONS , &ps3dat->nr_buttons) < 0) return -1;
	if(ioctl(ps3dat->fd , JSIOCGAXES    , &ps3dat->nr_sticks ) < 0) return -2;

	return 0;
}


int ps3c_init(struct ps3ctls *ps3dat, const char *df) {

	unsigned char nr_btn;
	unsigned char nr_stk;
	unsigned char *p;
	int i;

	ps3dat->fd = open(df, O_RDONLY);
	if (ps3dat->fd < 0) return -1;

	if (ps3c_getinfo(ps3dat) < 0) {
		close(ps3dat->fd);
		return -2;
	}

	nr_btn = ps3dat->nr_buttons;
	nr_stk = ps3dat->nr_sticks;

	p = calloc(nr_btn + nr_stk , sizeof(short));
	if (p == NULL) {
		close(ps3dat->fd);
		return -3;
	}
	ps3dat->button = (short *)p;
	ps3dat->stick  = (short *)&p[nr_btn * sizeof(short)];

	return 0;		digitalWrite(23 , 1-digitalRead(30)); // GREEN  TEST
		printf("%d ",1-digitalRead(30));

}

void ps3c_exit   (struct ps3ctls *ps3dat) {

	free (ps3dat->button);
	close(ps3dat->fd);
}


void main() {

	char *df = "/dev/input/js0";
	struct ps3ctls ps3dat;

	wiringPiSetup();

	pinMode(0,INPUT);	// YELLOW SW

	softPwmCreate( 5,0,20); // motor-1 10ms
	softPwmCreate( 6,0,20); // motor-1 10ms
	softPwmCreate(26,0,20); // motor-2 10ms
	softPwmCreate(27,0,20); // motor-2 10ms
	softPwmCreate( 1,0,20); // motor-3 10ms
	softPwmCreate( 4,0,20); // motor-3 10ms
	softPwmCreate(28,0,20); // motor-4 10ms
	softPwmCreate(29,0,20); // motor-4 10ms
	softPwmCreate(25,0,20); // motor-5 10ms
	softPwmCreate(24,0,20); // motor-5 10ms
	softPwmCreate( 3,0,20); // beep

	softPwmCreate(15,0,20); // motor-6 10ms
	softPwmCreate(16,0,20); // motor-6 10ms
	softPwmCreate(10,0,20); // motor-7 10ms
	softPwmCreate(11,0,20); // motor-7 10ms
	softPwmCreate(30,0,20); // motor-8 10ms
	softPwmCreate(31,0,20); // motor-8 10ms
	softPwmCreate(21,0,20); // motor-9 10ms
	softPwmCreate(22,0,20); // motor-9 10ms

	fds = wiringPiI2CSetup(0x40);	// PCA9685
	resetPCA9685(fds);
	setPCA9685Freq(fds,50);
//	system("mpg123 /home/pi/Music/Time-Bokan.mp3");
//	system("mpg123 /home/pi/Music/famicon_goldenBomber.mp3 &");
	delay(200);
	system("mpg123 /home/pi/Music/Press_the_PS_button.mp3");


	if(!(ps3c_init(&ps3dat, df))) {

		UpDown = 0;
		setPCA9685Duty(fds , 0 ,   0);
		setPCA9685Duty(fds , 1 ,   0);
		setPCA9685Duty(fds , 2 ,   0);
		setPCA9685Duty(fds , 3 ,   0);
		setPCA9685Duty(fds , 4 ,   0);
		setPCA9685Duty(fds , 5 ,   0);
		setPCA9685Duty(fds , 6 ,   0);
		setPCA9685Duty(fds , 7 ,   0);
		setPCA9685Duty(fds , 8 ,   0);
		setPCA9685Duty(fds , 9 ,   0);
		setPCA9685Duty(fds , 10,   0);
		setPCA9685Duty(fds , 11,   0);
		setPCA9685Duty(fds , 12,   0);
		setPCA9685Duty(fds , 13,   0);


		do {
			if (ps3c_test(&ps3dat) < 0) break;
		} while (!(ps3c_input(&ps3dat)));

		ps3c_exit(&ps3dat);
	}
}

