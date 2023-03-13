// I2C Addresses
// APDS-9960	->	0x39
// INA219 		->	0x40
// AlphaNum4 	->	0x70
// Bargraph 	->	0x71

//Digital Input
#define PIN_TOUCH_L 25
#define PIN_TOUCH_R 27
#define PIN_JOYSTICK_SW 19

struct Button {
    const uint8_t PIN;
    bool pressed;
    byte temp;
};

#define PIN_PUSH_1 15
Button button1 = {PIN_PUSH_1, false, 0};
#define PIN_PUSH_2 32
Button button2 = {PIN_PUSH_2, false, 0};
#define PIN_PUSH_3 16
Button button3 = {PIN_PUSH_3, false, 0};
#define PIN_PUSH_4 17
Button button4 = {PIN_PUSH_4, false, 0};
#define PIN_GESTURE_INT_MATRIX 18

//Digital Output
#define PIN_BUZZER 4
#define PIN_MOT_EN 23
#define PIN_MOT_DIR 5
#define PIN_LED_MATRIX 13
#define PIN_LED_RING 26

//Analog Input
#define PIN_MIC 34
#define PIN_LIGHT 35
#define PIN_JOYSTICK_Y 36
#define PIN_JOYSTICK_X 39

//Extra
#define PIN_I2C_MS_SDA 14
#define PIN_I2C_MS_SCL 33
#define PIN_I2C_SL_SDA 21
#define PIN_I2C_SL_SCL 22