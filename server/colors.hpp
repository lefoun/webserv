#ifndef MY_CUSTOM_COLORS
# define MY_CUSTOM_COLORS

// Escape code

#define ESC "\x1B"

// Color Codes

#define RESET ESC"[0m"

#define BLACK_COLOR "30"
#define RED_COLOR "31"
#define GREEN_COLOR "32"
#define YELLOW_COLOR "33"
#define BLUE_COLOR "34"
#define MAGENTA_COLOR "35"
#define CYAN_COLOR "36"
#define WHITE_COLOR "37"

// Form codes

#define NORMAL "0"
#define BOLD "1"
#define ITALIC "3"
#define UNDERLINE "4"

#ifdef COLOR_FORM
	#define FORM COLOR_FORM
#else
	#define FORM BOLD
#endif //COLOR_FORM

#define BLACK ESC "[" FORM ";" BLACK_COLOR "m"
#define RED ESC "[" FORM ";" RED_COLOR "m"
#define GREEN ESC "[" FORM ";" GREEN_COLOR "m"
#define YELLOW ESC "[" FORM ";" YELLOW_COLOR "m"
#define BLUE ESC "[" FORM ";" BLUE_COLOR "m"
#define MAGENTA ESC "[" FORM ";" MAGENTA_COLOR "m"
#define CYAN ESC "[" FORM ";" CYAN_COLOR "m"
#define WHITE ESC "[" FORM ";" WHITE_COLOR "m"

#endif
