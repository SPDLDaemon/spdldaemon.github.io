// Definitions of the states and the thresholds
// Tape sensor states
#define BLACK 0
#define GREEN 1
#define WHITE 2

// Tape sensor names
#define TP_C 4
#define TP_FL 1
#define TP_FR 2
#define TP_BL 0
#define TP_BR 3

#define TAPE 1
#define NO_TAPE 0
#define TAPE_THRESH 250
#define R_FOLLOW 0
#define L_FOLLOW 1
#define FRONT 2
#define CENTER 3

// this is a 20ms delay
#define delay 3750


// initializes the tape sensor module.
void TapeSensorInit(void);


// returns if the sensor is over tape/no over tape (1/0) respectively
int TapeSensorR_Follow(void);
int TapeSensorL_Follow(void);
int TapeSensorFront(void);
int TapeSensorCenter(void);

// call in while loop, updates the states of all the tape sensors
void TapeSensorRead(void);

// GetTapeColor dummy declaration for compilation
char getTapeColor(char sensor);

