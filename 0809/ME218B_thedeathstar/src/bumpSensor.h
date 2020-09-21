/*
 * These defines should eventually be moved to the bump sensor module
 */

// Bump sensor states
#define BMP_ON 1
#define BMP_OFF 0

// Bump sensor names
#define BMP_F 0
#define BMP_B 1

void initBump(void);

// GetBump dummy declaration for compilation
char getBump(char sensor);