#include <windows.h>
#include <stdint.h>
#include <stdio.h>

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
);

void SendKbdChar(DWORD ch);

int main()
{
	HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	if (hHook == NULL)
		MessageBox(NULL, L"Error ;-(", L"error", MB_OK);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0));
}

BOOL CtrlEnabled = FALSE, AltEnabled = FALSE, ShiftEnabled = FALSE, Converting = FALSE, OpeningQuotes = FALSE, PressedSpace = FALSE;

DWORD asciiL[] = { 'a',   'b',    'c',    'd',    'e',    'f',    'g',    'h',    'i',    'j',    'k',    'l',    'm',    'n',    'o',    'p',    'q',    'r',    's',    't',    'u',    'v',    'w',    'x',    'y',    'z',    0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,   0x38,   0x39,   /*-*/189, /*=*/187, /*[*/219, /*]*/221, /*\*/220, /*;*/186, /*'*/222, /*,*/188, /*.*/190, /*/*/191, /*`*/192 };
DWORD wideL[] = { 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F, 0xFF50, 0xFF51, 0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 0xFF10, 0xFF11, 0xFF12, 0xFF13, 0xFF14, 0xFF15, 0xFF16, 0xFF17, 0xFF18, 0xFF19, 0xFF0D,   0xFF1D,   0xFF3B,   0xFF3D,   0xFF3C,   0xFF1B,   0xFF07,   0xFF0C,   0xFF0E,   0xFF0F,   0xFF40 };

DWORD asciiU[] = { 'A',   'B',    'C',    'D',    'E',    'F',    'G',    'H',    'I',    'J',    'K',    'L',    'M',    'N',    'O',    'P',    'Q',    'R',    'S',    'T',    'U',    'V',    'W',    'X',    'Y',    'Z',    0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,   0x38,   0x39,   0x30,   /*_*/189, /*+*/187, /*{*/219, /*}*/221, /*|*/220, /*:*/186, 0,        /*<*/188, /*>*/190, /*?*/191, /*~*/192 };
DWORD wideU[] = { 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F, 0xFF30, 0xFF31, 0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 0xFF01, 0xFF20, 0xFF03, 0xFF04, 0xFF05, 0xFF3E, 0xFF06, 0xFF0A, 0xFF08, 0xFF09, 0xFF3F,   0xFF0B,   0xFF5B,   0xFF5D,   0xFF5C,   0xFF1A,   0,        0xFF1C,   0xFF1E,   0xFF1F,   0xFF5E };

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
)
{
	WPARAM identifier = wParam;
	KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT*)lParam;
	BOOL enabled;

	if (nCode != HC_ACTION)
		goto nextHook;

	enabled = identifier == WM_KEYDOWN || identifier == WM_SYSKEYDOWN;

	switch (kbd->vkCode) {
	case VK_CONTROL:
	case VK_LCONTROL:
	case VK_RCONTROL:
		CtrlEnabled = enabled;
		break;
	case VK_MENU:
	case VK_RMENU:
	case VK_LMENU:
		AltEnabled = enabled;
		break;
	case VK_SHIFT:
	case VK_LSHIFT:
	case VK_RSHIFT:
		ShiftEnabled = enabled;
		break;
	}

	if (!enabled)
		goto nextHook;

	if (CtrlEnabled && ShiftEnabled && kbd->vkCode == VK_F4) {
		printf("%sconverting\n", Converting ? "not " : "");
		Converting = !Converting;
	}
	
	if (Converting && !CtrlEnabled) {

		if (kbd->vkCode == 222 /* " */ && ShiftEnabled) {
			SendKbdChar(OpeningQuotes ? 0x300D : 0x300C);
			OpeningQuotes = !OpeningQuotes;
			return -1;
		}

		if (kbd->vkCode == 32 /* space */ && !PressedSpace) {
			SendKbdChar(32);
			goto nextHook;
		}

		if (PressedSpace)
			PressedSpace = FALSE;

		DWORD *ascii = ShiftEnabled ? asciiU : asciiL;
		DWORD *wide = ShiftEnabled ? wideU : wideL;
		for (size_t x = 0; x < 47; x++) {
			if (tolower(ascii[x]) == tolower(kbd->vkCode)) {
				wprintf(L"%d -> %d\n", ascii[x], wide[x]);
				SendKbdChar(wide[x]);
				return -1;
			}
		}
	}

nextHook:
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void SendKbdChar(DWORD ch)
{
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.dwFlags = KEYEVENTF_UNICODE;
	input.ki.wVk = 0;
	input.ki.wScan = ch;
	SendInput(1, &input, sizeof(input));

	input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(input));
}