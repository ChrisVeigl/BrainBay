The BrainBay Skindialog-Object provides a simple Graphic User interface
for controlling ohter object-properties. The Code is based on
John Roark's Win32 SkinStyle - library and the MFC-Skinsys-Library by
Cüneyt Elybol.

The description of the Skinned Dialog is stored in a .ini File which contains
the Filenames of Bitmap-Graphics for the activated and deactivated dialog
and the pressed buttons.

This (modified) version of the Skinstyle-library supports Pushbuttons, Textfields
and a Value-Bar which is a combination of two Pushbuttons and one Textfield.

The .ini file contains the following sections:

[SCREEN]                             // BMP-files for background and buttons
Mask=HeadMouse_Mask.bmp              // mask for transparent region
Main=HeadMouse_Active.bmp
Down=HeadMouse_Selected.bmp
Over=HeadMouse_Selected.bmp
Disabled=HeadMouse_Active.bmp

[BUTTONINFO]                                 // the Buttons
// format :
// serialnumber, ButtonName, top, left, width, height, Tooltip-Info,FALSE, SliderNumber, ButtonMode
// Slidernumber = 0: normal button
// Slidernumber = n, n > 0 : increase slider n
// Slidernumber = n, n < 0 : decrease slider n
// ButtonMode = 0: Pushbutton
// ButtonMode= n: RadioButton, Group n

// examples:
1=ON,114,9,52,42,Start and Stop,FALSE,0,0   // normal button
2=ON,150,10,72,22,Mode1,FALSE,0,1   // Radio Button, Group 1
3=ON,160,10,82,22,Mode2,FALSE,0,1   // Radio Button, Group 1
4=SLOW,6,54,38,61,SLOWER,FALSE,-1         // decrease slider nr. 1
5=FAST,237,55,36,61,FASTER,FALSE,1        // increase slider nr. 1

[TEXTINFO]           // Text-Fields or sliders
// format :
// serialnumber, FieldName, Font, FALSE, TRUE, -fontsize, fontcolor, top, left, width, heigth, text, tooltip, min, max, init, type
// text and tooltip have to be specified when this field is a text field 
// min,max, init, type have to be specified when this field is a slider which 
// shall output values from min to max. 
// type=0 : normal textfield
// type=1 : progress-bar 
// type=2: position marker

// example:
1=SPEED_SLIDER,Arial,FALSE,TRUE,-30,255,42,71,195,27, , ,0,20,10,1
