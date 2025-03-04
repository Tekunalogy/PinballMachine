#include "Arduino.h"
#include "PongSlider.h"
// #include "constants.h"
// #include "Adafruit_MotorShield.h"
class Adafruit_MotorShield;

namespace Pinball::PongSlide
{

PongSlider::PongSlider(Difficulty difficulty, Adafruit_DCMotor *sliderMotor)
{
    // AFMS = Adafruit_MotorShield(0x60);
    // sliderMotor = AFMS.getMotor(3);
    
    switch (difficulty)
    {
    case EASY:
    sliderSpeedPercent = 255;
        break;
    case MEDIUM:
    sliderSpeedPercent = 180;
        break;
    case HARD:
    sliderSpeedPercent = 100;
        break;
    default:
        break;
    }

    this->sliderMotor = sliderMotor;
}

void PongSlider::init()
{
    pinMode(Pinball::Constants::SLIDE_R_BTN_PIN, INPUT_PULLUP);
    pinMode(Pinball::Constants::SLIDE_L_BTN_PIN, INPUT_PULLUP);
    pinMode(Pinball::Constants::FLIP_R_BTN_PIN, INPUT_PULLUP);
    pinMode(Pinball::Constants::FLIP_L_BTN_PIN, INPUT_PULLUP);
    pinMode(Pinball::Constants::SLIDE_R_LS_PIN, INPUT_PULLUP);
    pinMode(Pinball::Constants::SLIDE_L_LS_PIN, INPUT_PULLUP);

    pinMode(Pinball::Constants::SLIDE_MOTOR_FWD_PIN, OUTPUT);
    pinMode(Pinball::Constants::SLIDE_MOTOR_PWM_PIN, OUTPUT);
    pinMode(Pinball::Constants::SLIDE_MOTOR_REV_PIN, OUTPUT);
    pinMode(Pinball::Constants::FLIP_L_SOLND_PIN, OUTPUT);
    pinMode(Pinball::Constants::FLIP_R_SOLND_PIN, OUTPUT);

    leftFlipFlag = false;
    rightFlipFlag = false;

    //adafruit motor shield implementation
    sliderMotor->setSpeed(sliderSpeedPercent);
}

void PongSlider::update(unsigned long currTime)
{
    //update flippers
    //set a flag when flipper button is pressed
    //set solenoid to full power
    //watch the time, if too much time elapsed, set to hold mode to prevent solenoid burnout
    //if button released, reset flag

    if(getFlipButton(RIGHT) && rightFlipFlag == false) //initial press
    {
        // Serial.println("R Flipper Pressed");
        rightFlipFlag = true;
        timeRightFlip = currTime; //start counting time
        analogWrite(Pinball::Constants::FLIP_R_SOLND_PIN, Constants::FLIP_PERCENT); //set output to 100%
    }
    else if(getFlipButton(RIGHT) && rightFlipFlag == true && currTime > timeRightFlip + Constants::FLIP_TIME) //switch to hold mode
    {
        // Serial.println("R flipper switching to hold current");
        analogWrite(Pinball::Constants::FLIP_R_SOLND_PIN, Constants::HOLD_PERCENT);
    }
    else if(!getFlipButton(RIGHT) && rightFlipFlag == true) //when button released, turn solenoid off
    {
        // Serial.println("R flipper released");
        rightFlipFlag = false;
        analogWrite(Pinball::Constants::FLIP_R_SOLND_PIN, Constants::OFF_PERCENT);
    }

    if(getFlipButton(LEFT) && leftFlipFlag == false) //initial press
    {
        // Serial.println("L Flipper Pressed");
        leftFlipFlag = true;
        timeLeftFlip = currTime; //start counting time
        analogWrite(Pinball::Constants::FLIP_L_SOLND_PIN, Constants::FLIP_PERCENT); //set output to 100%
    }
    else if(getFlipButton(LEFT) && leftFlipFlag == true && currTime > timeLeftFlip + Constants::FLIP_TIME) //switch to hold mode
    {
        // Serial.println("L flipper switching to hold current");
        analogWrite(Pinball::Constants::FLIP_L_SOLND_PIN, Constants::HOLD_PERCENT);
    }
    else if(!getFlipButton(LEFT) && leftFlipFlag == true) //when button released, turn solenoid off
    {
        // Serial.println("L flipper released");
        leftFlipFlag = false;
        analogWrite(Pinball::Constants::FLIP_L_SOLND_PIN, Constants::OFF_PERCENT);
    }

    //update slider
    slide(getSlideDirection());
}

void PongSlider::slide(Side side) //hbridge controller
{
    switch(side)
    {
    case RIGHT:
        analogWrite(Pinball::Constants::SLIDE_MOTOR_PWM_PIN, sliderSpeedPercent);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_FWD_PIN, LOW);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_REV_PIN, HIGH);

        //adafruit motor shield implementation
        sliderMotor->setSpeed(sliderSpeedPercent);
        sliderMotor->run(FORWARD);

        // Serial.println("Moving right");
        break;
    case LEFT:
        analogWrite(Pinball::Constants::SLIDE_MOTOR_PWM_PIN, sliderSpeedPercent);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_FWD_PIN, HIGH);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_REV_PIN, LOW);

        //adafruit motor shield implementation
        sliderMotor->setSpeed(sliderSpeedPercent);
        sliderMotor->run(BACKWARD);

        // Serial.println("Moving left");
        break;
    case HOLD:
        analogWrite(Pinball::Constants::SLIDE_MOTOR_PWM_PIN, sliderHoldPercent);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_FWD_PIN, LOW);
        digitalWrite(Pinball::Constants::SLIDE_MOTOR_REV_PIN, LOW);

        //adafruit motor shield implementation
        sliderMotor->setSpeed(0);
        sliderMotor->run(FORWARD);

        // Serial.println("Holding");
        break;
    default:
        Serial.print("code broketh, how did you even get to here");
        break;
    }
}

bool PongSlider::getFlipButton(Side side)
{
    if(side == RIGHT)
    {
        return !digitalRead(Pinball::Constants::FLIP_R_BTN_PIN);
    }
    else
    {
        return !digitalRead(Pinball::Constants::FLIP_L_BTN_PIN);
    }
}

PongSlider::Side PongSlider::getSlideDirection()
{
    bool slideRButton = !digitalRead(Pinball::Constants::SLIDE_R_BTN_PIN); 
    bool slideLButton = !digitalRead(Pinball::Constants::SLIDE_L_BTN_PIN);
    bool slideRLim = !digitalRead(Pinball::Constants::SLIDE_R_LS_PIN);
    bool slideLLim = !digitalRead(Pinball::Constants::SLIDE_L_LS_PIN);

    if(!slideLButton && slideRButton) //only pressing right button
    {
        if(slideRLim) //check if slider at right limit
        {
            return HOLD;
        }
        return RIGHT;
    }
    else if(slideLButton && !slideRButton) //only pressing left button
    {
        if(slideLLim) //check if slider at left limit
        {
            return HOLD;
        }
        return LEFT;
    }
    else //holding left and right buttons, or no buttons
    {
        return HOLD;
    }
}
}