#include <sys/keyboard.h>

struct scan_code_mapping_format scan_code_mapping_format_arr[] = 
{
 	{0x1E,0x61,0x41},
  {0x30,0x62,0x42},
  {0x2E,0x63,0x43},
  {0x20,0x64,0x44},
  {0x12,0x65,0x45},
  {0x21,0x66,0x46},
  {0x22,0x67,0x47},
  {0x23,0x68,0x48},
  {0x17,0x69,0x49},
  {0x24,0x6A,0x4A},
  {0x25,0x6B,0x4B},
  {0x26,0x6C,0x4C},
  {0x32,0x6D,0x4D},
  {0x31,0x6E,0x4E},
  {0x18,0x6F,0x4F},
  {0x19,0x70,0x50},
  {0x10,0x71,0x51},
  {0x13,0x72,0x52},
  {0x1F,0x73,0x53},
  {0x14,0x74,0x54},
  {0x16,0x75,0x55},
  {0x2f,0x76,0x56},
  {0x11,0x77,0x57},
  {0x2d,0x78,0x58},
  {0x15,0x79,0x59},
  {0x2c,0x7A,0x5A},
		{0x2,0x31,0x21},
  {0x3,0x32,0x40},
  {0x4,0x33,0x23},
  {0x5,0x34,0x24},
  {0x6,0x35,0x25},
  {0x7,0x36,0x5E},
  {0x8,0x37,0x26},
  {0x9,0x38,0x2A},
  {0xa,0x39,0x28},
  {0xb,0x30,0x29},
		{0xc,0x2D,0x5F},
  {0xd,0x3D,0x2B},
  {0x1a,0x5B,0x7B},
  {0x1b,0x5D,0x7D},
  {0x27,0x3B,0x3A},
  {0x28,0x27,0x22},
  {0x29,0x60,0x7E},
  {0x2b,0x5C,0x7C},
  {0x33,0x2C,0x3C},
  {0x34,0x2E,0x3E},
		{0x35,0x2F,0x3F},
  {0x39,0x20, INVALID_SCAN_CODE},
		{0x1d,'^',INVALID_SCAN_CODE},
  {0x2A,INVALID_SCAN_CODE,INVALID_SCAN_CODE},
  {0x36,INVALID_SCAN_CODE,INVALID_SCAN_CODE},
  {0x1c,'\n','\n'},
  {0xe,'\b','\b'},
  {INVALID_SCAN_CODE,INVALID_SCAN_CODE,INVALID_SCAN_CODE}
};

void signal_gets_trigger()
{
gets_in_progress = 1;
}
void release_gets_trigger()
{
gets_in_progress = 0;
}

int get_gets_trigger()
{
return gets_in_progress;
}

int gets(uint64_t destinationStr)
{
//char * user_buffer_space = (char *)destinationStr;
int bytesReadOntoBuffer = 0;
signal_gets_trigger();

__asm__("sti");

while(get_gets_trigger());//block

memcpy((void*)destinationStr,(void *)buffer_used_for_gets,gets_buffer_sz);

bytesReadOntoBuffer = gets_buffer_sz;

//resetting buffer size for next iteration
gets_buffer_sz = 0;

return bytesReadOntoBuffer;
}

struct scan_code_mapping_format get_mapping(unsigned char inputScanCode)
{
int i =0;
struct scan_code_mapping_format emptyStruct;
for(;scan_code_mapping_format_arr[i].scan_code!=INVALID_SCAN_CODE;i++)
{
if(scan_code_mapping_format_arr[i].scan_code == inputScanCode)
{
emptyStruct = scan_code_mapping_format_arr[i];
break;
}
}
return emptyStruct;
}

void outputNewLine()
{
buffer_used_for_gets[gets_buffer_sz++] = '\0'; //terminating the buffer now
release_gets_trigger();
current_vga_cursor += (160 - linePosition);
linePosition = 0;
check_if_buffer_is_full(0);
return;
}

void backSpace()
{
if(gets_buffer_sz - 1 >= 0)
{
gets_buffer_sz -- ;
current_vga_cursor -=2;
linePosition -=2;
*current_vga_cursor = ' '; // backspacing the character last written into buffer
}
return;
}

void takeInputIntoBuffer(char c)
{
buffer_used_for_gets[gets_buffer_sz++] = c;
*current_vga_cursor = c;
current_vga_cursor +=2;
linePosition +=2;
check_if_buffer_is_full(0);
return;
}

void keyBoardInputHandler()
{
struct scan_code_mapping_format current_scan_code;
unsigned char scan_code = inByte(0x60);
if(scan_code_sequence_number == 0) // first scan_code
{
current_scan_code = get_mapping(scan_code);
switch(current_scan_code.scan_code)
{
case '^':
shift_key_pressed_seq_number = 0;
scan_code_sequence_number = 1;
ctrl_key_pressed_seq_number = 1;
break;
case 0x2A:
case 0x36:
shift_key_pressed_seq_number = 1;
scan_code_sequence_number = 1;
break;
default:
shift_key_pressed_seq_number = 0;
scan_code_sequence_number = 1;
ctrl_key_pressed_seq_number = 0;

if(get_gets_trigger())
{
switch(current_scan_code.ascii_char_value)
{
case '\n': 
//handle new line
outputNewLine();
break;
case '\b':
//handle back space
backSpace();
break;
default:
takeInputIntoBuffer(current_scan_code.ascii_char_value);
break;
}
modify_cursor();
}
break;
}
}
else if(shift_key_pressed_seq_number)
{
current_scan_code = get_mapping(scan_code);
if(current_scan_code.scan_code != INVALID_SCAN_CODE &&
current_scan_code.scan_code !=0x2A &&
current_scan_code.scan_code != 0x36)
{
shift_key_pressed_seq_number = 0;
scan_code_sequence_number = 2;

if(get_gets_trigger())
{
switch(current_scan_code.ascii_shift_combo_char_value)
{
case '\n':
outputNewLine();
break;
case '\b':
backSpace();
break;
default:
takeInputIntoBuffer(current_scan_code.ascii_shift_combo_char_value);
break;


}
modify_cursor();
}
}
}
else if(ctrl_key_pressed_seq_number)
{
current_scan_code = get_mapping(scan_code);

if(current_scan_code.scan_code !=INVALID_SCAN_CODE && current_scan_code.ascii_char_value !='^')
{
*current_vga_cursor = current_scan_code.ascii_shift_combo_char_value;
current_vga_cursor += 2;
linePosition += 2;
ctrl_key_pressed_seq_number = 0; //done handling ctrl so breaking the sequence
scan_code_sequence_number = 2;
check_if_buffer_is_full(0);
}
}
else
scan_code_sequence_number -- ;

outByte(0x20,0x20);// reset signal to master interuppt
}
