#include <Windows.h>
#include <dwmapi.h>
#include <string>

#pragma comment(lib, "Dwmapi.lib")

void print_help() {
	printf(
		"usage: AcryllicWindow\n"
		"  --class          Window Class.\n"
		"  --title          Window Title.\n"
		"  --handle         Window Handle.\n"
		"  --frame          Window Frame. Can be auto | custom | default.\n"
		"  --backdrop       Backdrop Type. Can be auto | none | mica | acryllic | tabbed.\n"
		"  --border-color   Border color in hex format. Example: 0 fff #fff 0xfff 123456 123456ff\n"
		"  --caption-color  Caption color in hex format\n"
		"  --text-color     Text color in hex format.\n"
		"  --verbose        Show additional information.\n"
		"  --help           Print this help.\n"
		"  --wait           Wait for user input to close the application.\n"
		"  \n"
		"example: AcryllicWindow --title Godot --frame custom --backdrop acryllic --border-color 330000 --caption-color 0 --text-color fff\n"
	);
}

enum Frame {
	FRAME_AUTO,
	FRAME_CUSTOM,
	FRAME_DEFAULT
};

bool parse_frame(const char* arg, Frame* frame) {
	if (_stricmp(arg, "auto") == 0) {
		*frame = FRAME_AUTO;
	}
	else if (_stricmp(arg, "custom") == 0) {
		*frame = FRAME_CUSTOM;
	}
	else if (_stricmp(arg, "default") == 0) {
		*frame = FRAME_DEFAULT;
	}
	else {
		fprintf(stderr, "Incorrect argument for the --frame option. Must be auto, custom or default, not \"%s\".\n", arg);
		return false;
	}

	return true;
}

bool parse_backdrop(const char* arg, int* backdrop) {
	if (_stricmp(arg, "auto") == 0) {
		*backdrop = DWMSBT_AUTO;
	}
	else if (_stricmp(arg, "none") == 0) {
		*backdrop = DWMSBT_NONE;
	}
	else if (_stricmp(arg, "mica") == 0) {
		*backdrop = DWMSBT_MAINWINDOW;
	}
	else if (_stricmp(arg, "acryllic") == 0) {
		*backdrop = DWMSBT_TRANSIENTWINDOW;
	}
	else if (_stricmp(arg, "tabbed") == 0) {
		*backdrop = DWMSBT_TABBEDWINDOW;
	}
	else {
		fprintf(stderr, "Incorrect argument for the --backdrop option. Must be auto, none, mica, acryllic or tabbed, not \"%s\".\n", arg);
		return false;
	}

	return true;
}

int hex2int(char ch)
{
	if (ch >= '0' && ch <= '9') {
		return ch - '0';
	}
	else if (ch >= 'A' && ch <= 'F') {
		return ch - 'A' + 10;
	}
	else if (ch >= 'a' && ch <= 'f') {
		return ch - 'a' + 10;
	}

	return -1;
}

bool parse_color(const char* arg, const char* color_name, COLORREF* color) {
	size_t arg_len = strlen(arg);
	if (arg_len < 1) {
		fprintf(stderr, "Failed to parse %s. Error: The argument is empty.", color_name);
		return false;
	}

	// skip # and 0x
	size_t arg_pos = 0;
	if (arg[0] == '#') {
		arg_pos = 1;
	}
	else if (arg[0] == '0') {
		if (arg_len > 1) {
			if (arg[1] == 'x') {
				arg_pos = 2;
			}
		}
	}

	// check that there is more data except # and 0x
	if (arg_pos >= arg_len) {
		fprintf(stderr, "Failed to parse %s. Error: There is no color data.", color_name);
		return false;
	}

	// check that there are 8 digits max
	size_t num_digits = arg_len - arg_pos;
	if (num_digits > 8) {
		fprintf(stderr, "Failed to parse %s. Error: There are more than 8 digits.", color_name);
		return false;
	}

	// check that the color data is in hex format
	int digits[8] = { 0 };
	for (size_t i = arg_pos; i < arg_len; i++) {
		int digit = hex2int(arg[i]);
		if (digit < 0) {
			fprintf(stderr, "Failed to parse %s. Error: the color not in hex format.", color_name);
			return false;
		}

		digits[i - arg_pos] = digit;
	}

	// make color
	int r = 0;
	int g = 0;
	int b = 0;

	switch (num_digits) {
	case 1:
		r = digits[0] * 16 + digits[0];
		g = r;
		b = r;
		break;

	case 3:
		r = digits[0] * 16 + digits[0];
		g = digits[1] * 16 + digits[1];
		b = digits[2] * 16 + digits[2];
		break;

	case 6:
	case 8:
		r = digits[0] * 16 + digits[1];
		g = digits[2] * 16 + digits[3];
		b = digits[4] * 16 + digits[5];
		break;

	default:
		fprintf(stderr, "Failed to parse %s. Error: The color must have 1, 3, 6 or 8 digits.", color_name);
		return false;
	}

	*color = RGB(r, g, b);

	return true;
}

void set_frame(HWND hwnd, Frame frame, bool verbose) {
	if (frame == FRAME_AUTO)
		return;

	LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
	if (!style) {
		fprintf(stderr, "Failed to get GWL_STYLE. Error: %d\n", GetLastError());
		return;
	}

	bool apply = false;

	switch (frame) {
	case FRAME_CUSTOM:
		if (style & WS_CAPTION) {
			style &= ~WS_CAPTION;
			apply = true;
		}

		if (style & WS_THICKFRAME) {
			style &= ~WS_THICKFRAME;
			apply = true;
		}

		break;

	case FRAME_DEFAULT:
		if (!(style & WS_CAPTION)) {
			style |= WS_CAPTION;
			apply = true;
		}

		if (!(style & WS_THICKFRAME)) {
			style |= WS_THICKFRAME;
			apply = true;
		}

		break;
	}

	if (apply) {
		if (verbose)
			printf("Setting the new frame.\n");

		if (!SetWindowLongPtr(hwnd, GWL_STYLE, style))
			fprintf(stderr, "Failed to set the new style. Error: %d\n", GetLastError());

		if (!SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE))
			fprintf(stderr, "Failed to notify SWP_FRAMECHANGED. Error: %d\n", GetLastError());
	}
	else {
		if (verbose)
			printf("The new frame is the same as the old one. No changes made.\n");
	}
}

void set_backdrop(HWND hwnd, int backdrop, bool verbose) {
	bool apply = false;

	int old_backdrop = DWMSBT_AUTO;
	HRESULT hresult = DwmGetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &old_backdrop, sizeof(old_backdrop));
	if (FAILED(hresult)) {
		fprintf(stderr, "Failed to get DWMWA_SYSTEMBACKDROP_TYPE. Error: %d.\n", hresult);
		apply = true;
	}
	else {
		apply = backdrop != old_backdrop;
	}

	if (apply) {
		if (verbose)
			printf("Setting the new backdrop %d. The current backdrop is %d.\n", backdrop, old_backdrop);

		hresult = DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
		if (FAILED(hresult))
			fprintf(stderr, "Failed to set DWMWA_SYSTEMBACKDROP_TYPE. Error: %d.\n", hresult);
	}
	else {
		if (verbose)
			printf("The new backdrop is the same as the old one. No changes made.\n");
	}
}

#define WAIT_AND_RETURN(return_code) {	\
	if (should_wait)					\
		(void)getchar();				\
										\
	return return_code;					\
}

#define CHECK_HAS_ARGUMENT(option_name) \
	if (i + 1 == argc) {				\
		fprintf(stderr, "Not enough arguments for the --%s option.\n", option_name); \
		WAIT_AND_RETURN(EXIT_FAILURE);  \
	}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		print_help();
		return EXIT_SUCCESS;
	}

	bool verbose = false;
	bool should_return = false;
	bool should_wait = false;

	// ARGUMENTS: --verbose & --help
	for (int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		if (_stricmp(arg, "--verbose") == 0) {
			verbose = true;
		}
		else if (_stricmp(arg, "--help") == 0) {
			should_return = true;
			print_help();
		}
		else if (_stricmp(arg, "--wait") == 0) {
			should_wait = true;
		}
	}

	if (verbose) {
		printf("Argument Count: %d. Arguments:\n", argc);
		for (int i = 0; i < argc; i++)
			printf("  \"%s\"\n", argv[i]);
	}

	if (should_return)
		WAIT_AND_RETURN(EXIT_SUCCESS);

	// ARGUMENTS: --class, --title, --handle, --frame, --backdrop & colors
	HWND hwnd = NULL;
	Frame frame = FRAME_AUTO;
	int backdrop = DWMSBT_AUTO;
	COLORREF border_color = 0;
	COLORREF caption_color = 0;
	COLORREF text_color = 0;

	bool should_set_frame = false;
	bool should_set_backdrop = false;
	bool should_set_border_color = false;
	bool should_set_caption_color = false;
	bool should_set_text_color = false;

	for (int i = 1; i < argc; i++) {
		const char* option = argv[i];
		if (_stricmp(option, "--class") == 0) {
			CHECK_HAS_ARGUMENT("--class");
			const char* window_class = argv[i + 1];
			hwnd = FindWindowA(window_class, NULL);
			if (hwnd == NULL) {
				fprintf(stderr, "Failed to find window with class = \"%s\". Error: %d\n", window_class, GetLastError());
				WAIT_AND_RETURN(EXIT_FAILURE);
			}

			i++;
		}
		else if (_stricmp(option, "--title") == 0) {
			CHECK_HAS_ARGUMENT("--title");
			const char* window_title = argv[i + 1];
			hwnd = FindWindowA(NULL, window_title);
			if (hwnd == NULL) {
				fprintf(stderr, "Failed to find window with title = \"%s\". Error: %d\n", window_title, GetLastError());
				WAIT_AND_RETURN(EXIT_FAILURE);
			}

			i++;
		}
		else if (_stricmp(option, "--handle") == 0) {
			CHECK_HAS_ARGUMENT("--handle");
			const char* window_handle = argv[i + 1];
			hwnd = reinterpret_cast<HWND>(strtoll(window_handle, nullptr, 10));
			if (!IsWindow(hwnd)) {
				fprintf(stderr, "Failed to find window with handle = \"%s\". Error: %d\n", window_handle, GetLastError());
				WAIT_AND_RETURN(EXIT_FAILURE);
			}

			i++;
		}
		else if (_stricmp(option, "--frame") == 0) {
			CHECK_HAS_ARGUMENT("--frame");
			if (!parse_frame(argv[i + 1], &frame))
				WAIT_AND_RETURN(EXIT_FAILURE);

			should_set_frame = true;
			i++;
		}
		else if (_stricmp(option, "--backdrop") == 0) {
			CHECK_HAS_ARGUMENT("--backdrop");
			if (!parse_backdrop(argv[i + 1], &backdrop))
				WAIT_AND_RETURN(EXIT_FAILURE);

			should_set_backdrop = true;
			i++;
		}
		else if (_stricmp(option, "--border-color") == 0) {
			CHECK_HAS_ARGUMENT("--border-color");
			if (!parse_color(argv[i + 1], "border-color", &border_color))
				WAIT_AND_RETURN(EXIT_FAILURE);

			should_set_border_color = true;
			i++;
		}
		else if (_stricmp(option, "--caption-color") == 0) {
			CHECK_HAS_ARGUMENT("--caption-color");
			if (!parse_color(argv[i + 1], "caption-color", &caption_color))
				WAIT_AND_RETURN(EXIT_FAILURE);

			should_set_caption_color = true;
			i++;
		}
		else if (_stricmp(option, "--text-color") == 0) {
			CHECK_HAS_ARGUMENT("--text-color");
			if (!parse_color(argv[i + 1], "text-color", &text_color))
				WAIT_AND_RETURN(EXIT_FAILURE);

			should_set_text_color = true;
			i++;
		}
	}

	if (hwnd == NULL) {
		fprintf(stderr, "Failed to get window. Specify one of the options: --class, --title or --handle.\n");
		WAIT_AND_RETURN(EXIT_FAILURE);
	}

	// SET NEW SETTINGS
	if (should_set_backdrop)
		set_backdrop(hwnd, backdrop, verbose);

	if (should_set_border_color) {
		HRESULT hresult = DwmSetWindowAttribute(hwnd, DWMWA_BORDER_COLOR, &border_color, sizeof(border_color));
		if (FAILED(hresult))
			fprintf(stderr, "Failed to set DWMWA_BORDER_COLOR. Error: %d.\n", hresult);
	}

	if (should_set_caption_color) {
		HRESULT hresult = DwmSetWindowAttribute(hwnd, DWMWA_CAPTION_COLOR, &caption_color, sizeof(caption_color));
		if (FAILED(hresult))
			fprintf(stderr, "Failed to set DWMWA_CAPTION_COLOR. Error: %d.\n", hresult);
	}

	if (should_set_text_color) {
		HRESULT hresult = DwmSetWindowAttribute(hwnd, DWMWA_TEXT_COLOR, &text_color, sizeof(text_color));
		if (FAILED(hresult))
			fprintf(stderr, "Failed to set DWMWA_TEXT_COLOR. Error: %d.\n", hresult);
	}

	if (should_set_frame)
		set_frame(hwnd, frame, verbose);

	WAIT_AND_RETURN(EXIT_SUCCESS);
}
