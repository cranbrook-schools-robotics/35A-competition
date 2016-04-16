#pragma config(UserModel, "C:/Users/rstudent/code/robot-configs/35A-in-and-out.c")
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma platform(VEX)

//Competition Control and Duration Settings
#pragma competitionControl(Competition)
#pragma autonomousDuration(20)
#pragma userControlDuration(120)

#include "Vex_Competition_Includes.c"   //Main competition background code...do not modify!

#define POWER_EXPANDER_A1
#include <CKFlywheelSpeedController.h>




FlywheelSpeedController speedCtlr;


task FlywheelSpeedControl() {
	while(true){
		update(speedCtlr);
		delay(50);
	}
}


void setDrive( int left, int right )
{
	motor[mDriveL] = left;
	motor[mDriveR] = right;
}

void setIntakeRoller( int power ){
	motor[mIntakeRoller] = power;
}

void setIntakeChain( int power ){
	motor[mIntakeF] = motor[mIntakeM] = motor[mIntakeB] = power;
}

const float DriveWheelDiameter = 4; //inches

int feetToTicks( float feet ){
	static const float conversion = 12 / (DriveWheelDiameter*PI) * TicksPerRev_393Turbo;
	return (int)( feet * conversion );
}

float feetToDegrees( float feet ){
	static const float conversion = 12 / (DriveWheelDiameter*PI) * 360;
	return (int)( feet * conversion );
}


int getIMETicks(tMotor port){
	int ticks = nMotorEncoder[port];
	nMotorEncoder[port] = 0;
	return ticks;
}

float driveDistanceFeet = 0;

int slowingDistanceTicks = feetToTicks(2);

task driveStraightTask(){
	int ticks = feetToTicks( driveDistanceFeet );
	int power = 127;
	setDrive( power, power );
	delay(100);
	int leftTicks = 0, rightTicks = 0;
	while( leftTicks < ticks || rightTicks < ticks ){
		leftTicks += abs( getIMETicks(mDriveL) );
		rightTicks += abs( getIMETicks(mDriveR) );
		int error = leftTicks - rightTicks;
		int turningOffset = 10 * error;
		//int remaining = ticks - (leftTicks + rightTicks)/2;
		setDrive( power - turningOffset, power + turningOffset );
		delay( 50 );
	}
	setDrive(0,0);
}

void driveStraight( float distFt ){
	driveDistanceFeet = distFt;
	startTask( driveStraightTask );

	//float angle = feetToDegrees(driveDistanceFeet);
	//setMotorTarget(mDriveL, angle, 127);
	//setMotorTarget(mDriveR, angle, 127);
}



/////////////////////////////////////////////////////////////////////////////////////////
//
//                          Pre-Autonomous Functions
//
// You may want to perform some actions before the competition starts. Do them in the
// following function.
//
/////////////////////////////////////////////////////////////////////////////////////////

void pre_auton()
{
  // Set bStopTasksBetweenModes to false if you want to keep user created tasks running between
  // Autonomous and Tele-Op modes. You will need to manage all user created tasks if set to false.
  bStopTasksBetweenModes = true;


	// power = A e^( B speed )
	//const float A = 1.2235, B = 0.1072;
  const float A = 0.7686, B = 0.1304; // April 13th recharacterization

	// Controller coefficients
	const float Kq = 0.2, Ki = 0.02, Kd = 0;

	const tMotor motorPorts[] =	{ mFly1, mFly2, mFly3, mFly4 };

  FlywheelSpeedControllerInit( speedCtlr, Kq, Ki, Kd, A, B, motorPorts, 4, M393HighSpeed );

  // half of the flywheel motors are on the main brain battery; other half on the power expander.
  setFlywheelBatteryConfig( speedCtlr, vPowerExpander, 0.5 );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 Autonomous Task
//
// This task is used to control your robot during the autonomous phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

task autonomous()
{
	startTask( FlywheelSpeedControl, kHighPriority );

	driveStraight( 4 );
	delay(4000);
	driveStraight( 2 );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                                 User Control Task
//
// This task is used to control your robot during the user control phase of a VEX Competition.
// You must modify the code to add your own robot specific commands here.
//
/////////////////////////////////////////////////////////////////////////////////////////

task usercontrol()
{
	const float FlyspeedMin = 7.5, FlyspeedMid = 10, FlyspeedMax = 16, FlyspeedIncrement = 0.5;

	float flyspeed = FlyspeedMin;
	bool isFlywheelOn = false;

	startTask( FlywheelSpeedControl, kHighPriority );

	time1[T1] = 0;
	while (true)
	{
		setDrive( vexRT[ChJoyLY], vexRT[ChJoyRY] );
		setIntakeRoller( buttonsToPower(Btn5D, Btn5U) );
		setIntakeChain( buttonsToPower(Btn6D, Btn6U) );

		int turnOnFlywheel = vexRT[Btn8L];
		int turnOffFlywheel = vexRT[Btn8R];

		if( turnOnFlywheel || turnOffFlywheel ){
			isFlywheelOn = (bool)turnOnFlywheel;
		}

		int speedUpFlywheel = vexRT[Btn8U];
		int slowDownFlywheel = vexRT[Btn8D];
		if( speedUpFlywheel || slowDownFlywheel ){
			flyspeed += (speedUpFlywheel ? +FlyspeedIncrement : -FlyspeedIncrement);
			flyspeed = bound( flyspeed, FlyspeedMin, FlyspeedMax );
			delay(100);
		}

		if( vexRT[Btn7L] ){
			flyspeed = FlyspeedMax;
			isFlywheelOn = true;
		}

		if( vexRT[Btn7R] ){
			flyspeed = FlyspeedMin;
			isFlywheelOn = true;
		}

		if( vexRT[Btn7U] ){
			flyspeed = FlyspeedMid;
			isFlywheelOn = true;
		}

		setTargetSpeed( speedCtlr, isFlywheelOn ? flyspeed : 0 );

		delay(10);
	}
}
//}
//if ( limitswitch = true){
//	limitcount = 1;

//	if (limitcount = 1 && limitswitch = false) {
//		IntakeDown until limitswitch = true
//	}
