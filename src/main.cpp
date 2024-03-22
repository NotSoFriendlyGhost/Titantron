#include "main.h"
#include "EZ-Template/util.hpp"
#include "pros/misc.h"
#include "pros/misc.hpp"
#include "pros/motors.h"
#include "titantron/globals.hpp"
#include "titantron/recording.hpp"

/////
// For installation, upgrading, documentations and tutorials, check out our website!
// https://ez-robotics.github.io/EZ-Template/
/////

// Chassis constructor
ez::Drive chassis (
  // Left Chassis Ports (negative port will reverse it!)
  //   the first port is used as the sensor
  {-1, -11}

  // Right Chassis Ports (negative port will reverse it!)
  //   the first port is used as the sensor
  ,{10, 20}

  // IMU Port
  ,13

  // Wheel Diameter (Remember, 4" wheels without screw holes are actually 4.125!)
  ,3.25

  // Cartridge RPM
  ,200

  // External Gear Ratio (MUST BE DECIMAL) This is WHEEL GEAR / MOTOR GEAR
  // eg. if your drive is 84:36 where the 36t is powered, your RATIO would be 84/36 which is 2.333
  // eg. if your drive is 60:36 where the 36t is powered, your RATIO would be 60/36 which is 0.6
  // eg. if your drive is 36:60 where the 60t is powered, your RATIO would be 36/60 which is 0.6
  ,0.6
);



/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
  // Print our branding over your terminal :D
  ez::ez_template_print();
  
  pros::delay(500); // Stop the user from doing anything while legacy ports configure

  // Configure chassis
  chassis.opcontrol_curve_buttons_toggle(false); // Enables modifying the controller curve with buttons on the joysticks
  chassis.opcontrol_drive_activebrake_set(0.1);

  // Configure braking modes
  intake.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
	flywheel.set_brake_mode(pros::E_MOTOR_BRAKE_COAST);
	lifter.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
  default_constants(); // Set the drive to your own constants from autons.cpp!

  // These are already defaulted to these buttons, but you can change the left/right curve buttons here!
  // chassis.opcontrol_curve_buttons_left_set (pros::E_CONTROLLER_DIGITAL_LEFT, pros::E_CONTROLLER_DIGITAL_RIGHT); // If using tank, only the left side is used. 
  // chassis.opcontrol_curve_buttons_right_set(pros::E_CONTROLLER_DIGITAL_Y,    pros::E_CONTROLLER_DIGITAL_A);

  // Autonomous Selector using LLEMU
  ez::as::auton_selector.autons_add({
    Auton("Driver skills macro", skills),
  });

  // Initialize chassis and auton selector
  chassis.initialize();
  ez::as::initialize();
  master.rumble(".");
  pros::delay(60);
  master.clear();
  pros::delay(60);
}



/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
  // . . .
}



/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
  // . . .
}



/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
  chassis.pid_targets_reset(); // Resets PID targets to 0
  chassis.drive_imu_reset(); // Reset gyro position to 0
  chassis.drive_sensor_reset(); // Reset drive sensors to 0
  chassis.drive_brake_set(MOTOR_BRAKE_HOLD); // Set motors to hold.  This helps autonomous consistency

  ez::as::auton_selector.selected_auton_call(); // Calls selected auton from autonomous selector
}



/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
  // This is preference to what you like to drive on
  if(!pros::competition::is_connected()){
    master.set_text(0,0,"Right to record");
    pros::delay(60);
  }
  else{
    master.set_text(0,0,"Don't sell Matthew");
    pros::delay(60);
  }
  autonomous();
  chassis.drive_brake_set(pros::E_MOTOR_BRAKE_COAST);
  master.set_text(1,0,"Flywheel: 75%");
	pros::delay(60);
  bool endgameDirection = 1;
  chassis.drive_brake_set(pros::E_MOTOR_BRAKE_COAST);
  //teamLogo();
  
  while (true) {
    
    if (!pros::competition::is_connected()) { 
      // Trigger the selected autonomous routine
      if (master.get_digital_new_press(DIGITAL_X)){
        autonomous();
        chassis.drive_brake_set(pros::E_MOTOR_BRAKE_COAST);
      } 

      // Toggle recording with right button
      if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_RIGHT)){
        // Start recording
        if(!recording){
          master.clear_line(0);
          pros::delay(60);
          master.set_text(0,0,"Recording...");
          pros::delay(60);
          chassis.drive_brake_set(pros::E_MOTOR_BRAKE_HOLD);
          startRecording("recording.txt");
        }
        // Stop recording
        else {
          stopRecording();
          chassis.drive_brake_set(pros::E_MOTOR_BRAKE_COAST);
          master.clear_line(0);
          pros::delay(60);
          master.set_text(0,0,"Recording Saved");
          pros::delay(60);
        }
      }
      // Start recorded playback
      if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_LEFT)){
        master.clear_line(0);
        pros::delay(60);
        master.set_text(0,0, "Replaying...");
        pros::delay(60);
        chassis.drive_brake_set(pros::E_MOTOR_BRAKE_HOLD);
        playback("recording.txt");
        chassis.drive_brake_set(pros::E_MOTOR_BRAKE_COAST);
        master.clear_line(0);
        pros::delay(60);
        master.set_text(0,0, "Replay Done");
        pros::delay(60);
      }
    } 
    chassis.opcontrol_arcade_standard(ez::SPLIT);

    // Track drivetrain motor voltages for recording
    if(recording) 
      drive.trackVoltage();
		
    // Intake balls
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_L1)){
			intake.move_voltage(12000);
		}
    // Release balls
		else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R1)){
			intake.move_voltage(-12000);
		}
    // Stop intake
		else{
			intake.brake();
		}

    // Print intake voltage to text file if recording
    if(recording)
      trackIntake();

    // Spin flywheel
    if(master.get_digital(pros::E_CONTROLLER_DIGITAL_R2))
		  flywheel.move_voltage(-12000*flywheelVelocity);
    else if(master.get_digital(pros::E_CONTROLLER_DIGITAL_Y))
		  flywheel.move_voltage(12000*flywheelVelocity);
    // Stop flywheel
    else flywheel.brake();

    // Toggle flywheel velocity between 75% and 100%
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_B)){
			if(flywheelVelocity==1){
				flywheelVelocity = 0.75;
				master.clear_line(1);
				pros::delay(60);
				master.set_text(1,0,"Flywheel: 75%");
				pros::delay(60);
			}
			else{
				flywheelVelocity = 1;
				master.clear_line(1);
				pros::delay(60);
				master.set_text(1,0,"Flywheel: 100%");
				pros::delay(60);
			}
		}

    // Print flywheel voltage to text file if recording
    if(recording)
      trackFlywheel();

    // Toggle wings
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_L2)){
			wingState = !wingState;
		} 
		wings.set_value(wingState);

    // Print wing state to text file if recording
    if(recording) 
      trackWings(wingState);

    // Reverse lifter direction everytime L2 is pressed
		if(master.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_DOWN)) endgameDirection = !endgameDirection;

    // Move lifter
		if(master.get_digital(pros::E_CONTROLLER_DIGITAL_DOWN)){
			if(endgameDirection) lifter.move_voltage(12000);
			else lifter.move_voltage(-12000);
		}
    // Stop lifter
		else lifter.brake();

    // Print lifter voltage to text file if recording
    if(recording)
      trackLifter();

    // Start next line of recording
		if(recording) ofs<<'\n';
		
    if(recording)
      pros::delay(1);
    else
      pros::delay(ez::util::DELAY_TIME); // This is used for timer calculations!  Keep this ez::util::DELAY_TIME
  }
}
