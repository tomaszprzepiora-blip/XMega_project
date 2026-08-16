#include "TFT_objects.h"
#include "TFT_resources.h"

// TFT module connections
char TFT_DataPort at PORTK_OUT;
char TFT_DataPort_Direction at PORTK_DIR;
sbit TFT_RST at PORTJ_OUT.B1;
sbit TFT_BLED at PORTJ_OUT.B0;
sbit TFT_RS at PORTJ_OUT.B4;
sbit TFT_CS at PORTJ_OUT.B5;
sbit TFT_RD at PORTJ_OUT.B2;
sbit TFT_WR at PORTJ_OUT.B3;
sbit TFT_RST_Direction at PORTJ_DIR.B1;
sbit TFT_BLED_Direction at PORTJ_DIR.B0;
sbit TFT_RS_Direction at PORTJ_DIR.B4;
sbit TFT_CS_Direction at PORTJ_DIR.B5;
sbit TFT_RD_Direction at PORTJ_DIR.B2;
sbit TFT_WR_Direction at PORTJ_DIR.B3;
// End TFT module connections

// Touch Panel module connections
sbit DriveX_Left at PORTA_OUT.B2;
sbit DriveX_Right at PORTQ_OUT.B2;
sbit DriveY_Up at PORTQ_OUT.B3;
sbit DriveY_Down at PORTA_OUT.B1;
sbit DriveX_Left_Direction at PORTA_DIR.B2;
sbit DriveX_Right_Direction at PORTQ_DIR.B2;
sbit DriveY_Up_Direction at PORTQ_DIR.B3;
sbit DriveY_Down_Direction at PORTA_DIR.B1;
// End Touch Panel module connections

// Global variables
unsigned int Xcoord, Ycoord;
const ADC_THRESHOLD = 900;
char PenDown;
void *PressedObject;
int PressedObjectType;
unsigned int caption_length, caption_height;
unsigned int display_width, display_height;

int i;
unsigned short object_pressed;
TLabel *local_label;
TLabel *exec_label;
short label_order;
TImage *local_image;
TImage *exec_image;
short image_order;
TCircle *local_circle;
TCircle *exec_circle;
short circle_order;


static void InitializeTouchPanel() {
  TFT_Set_Default_Mode();
  TFT_Init_ILI9341_8bit(240, 320);

}


/////////////////////////
  TScreen*  CurrentScreen;

  TScreen                Screen1;
  TImage               Image1;
  TCircle                Circle1;
  TCircle                Circle2;
  TCircle                Circle3;
  TCircle                Circle4;
  TCircle                Circle5;
  TCircle                Circle6;
  TCircle                Circle7;
  TCircle                Circle8;
  TCircle                Circle9;
  TCircle                Circle10;
  TCircle                Circle11;
  TCircle                Circle12;
  TCircle                Circle13;
  TCircle                Circle14;
  TCircle                Circle15;
  TLabel                 Label1;
char Label1_Caption[11] = "mikromedia";

  TLabel                 Label2;
char Label2_Caption[10] = "for XMEGA";

  TCircle                Circle16;
  TCircle                Circle17;
  TCircle                Circle18;
  TCircle                Circle20;
  TLabel                 * const code far Screen1_Labels[2]=
         {
         &Label1,              
         &Label2               
         };
  TImage                 * const code far Screen1_Images[1]=
         {
         &Image1               
         };
  TCircle                * const code far Screen1_Circles[19]=
         {
         &Circle1,             
         &Circle2,             
         &Circle3,             
         &Circle4,             
         &Circle5,             
         &Circle6,             
         &Circle7,             
         &Circle8,             
         &Circle9,             
         &Circle10,            
         &Circle11,            
         &Circle12,            
         &Circle13,            
         &Circle14,            
         &Circle15,            
         &Circle16,            
         &Circle17,            
         &Circle18,            
         &Circle20             
         };




static void InitializeObjects() {
  Screen1.Color                     = 0x0000;
  Screen1.Width                     = 240;
  Screen1.Height                    = 320;
  Screen1.LabelsCount               = 2;
  Screen1.Labels                    = Screen1_Labels;
  Screen1.ImagesCount               = 1;
  Screen1.Images                    = Screen1_Images;
  Screen1.CirclesCount              = 19;
  Screen1.Circles                   = Screen1_Circles;
  Screen1.ObjectsCount              = 22;


  Image1.OwnerScreen     = &Screen1;
  Image1.Order          = 0;
  Image1.Left           = 0;
  Image1.Top            = 151;
  Image1.Width          = 240;
  Image1.Height         = 169;
  Image1.Picture_Type   = 0;
  Image1.Picture_Ratio  = 1;
  Image1.Picture_Name   = xmega2_BMP;
  Image1.Visible        = 1;
  Image1.Active         = 1;
  Image1.OnUpPtr         = 0;
  Image1.OnDownPtr       = 0;
  Image1.OnClickPtr      = 0;
  Image1.OnPressPtr      = 0;

  Circle1.OwnerScreen     = &Screen1;
  Circle1.Order           = 1;
  Circle1.Left            = 148;
  Circle1.Top             = 7;
  Circle1.Radius          = 14;
  Circle1.Pen_Width       = 1;
  Circle1.Pen_Color       = 0xFFFF;
  Circle1.Visible         = 1;
  Circle1.Active          = 1;
  Circle1.Transparent     = 1;
  Circle1.Gradient        = 0;
  Circle1.Gradient_Orientation    = 0;
  Circle1.Gradient_Start_Color    = 0xFFFF;
  Circle1.Gradient_End_Color      = 0xC618;
  Circle1.Color           = 0x0000;
  Circle1.PressColEnabled = 1;
  Circle1.Press_Color     = 0x8410;
  Circle1.OnUpPtr         = 0;
  Circle1.OnDownPtr       = 0;
  Circle1.OnClickPtr      = 0;
  Circle1.OnPressPtr      = 0;

  Circle2.OwnerScreen     = &Screen1;
  Circle2.Order           = 2;
  Circle2.Left            = 218;
  Circle2.Top             = 50;
  Circle2.Radius          = 6;
  Circle2.Pen_Width       = 1;
  Circle2.Pen_Color       = 0xFFFF;
  Circle2.Visible         = 1;
  Circle2.Active          = 1;
  Circle2.Transparent     = 1;
  Circle2.Gradient        = 0;
  Circle2.Gradient_Orientation    = 0;
  Circle2.Gradient_Start_Color    = 0xFFFF;
  Circle2.Gradient_End_Color      = 0xC618;
  Circle2.Color           = 0x0000;
  Circle2.PressColEnabled = 1;
  Circle2.Press_Color     = 0x8410;
  Circle2.OnUpPtr         = 0;
  Circle2.OnDownPtr       = 0;
  Circle2.OnClickPtr      = 0;
  Circle2.OnPressPtr      = 0;

  Circle3.OwnerScreen     = &Screen1;
  Circle3.Order           = 3;
  Circle3.Left            = 222;
  Circle3.Top             = 214;
  Circle3.Radius          = 4;
  Circle3.Pen_Width       = 1;
  Circle3.Pen_Color       = 0xFFFF;
  Circle3.Visible         = 1;
  Circle3.Active          = 1;
  Circle3.Transparent     = 1;
  Circle3.Gradient        = 0;
  Circle3.Gradient_Orientation    = 0;
  Circle3.Gradient_Start_Color    = 0xFFFF;
  Circle3.Gradient_End_Color      = 0xC618;
  Circle3.Color           = 0x0000;
  Circle3.PressColEnabled = 1;
  Circle3.Press_Color     = 0x8410;
  Circle3.OnUpPtr         = 0;
  Circle3.OnDownPtr       = 0;
  Circle3.OnClickPtr      = 0;
  Circle3.OnPressPtr      = 0;

  Circle4.OwnerScreen     = &Screen1;
  Circle4.Order           = 4;
  Circle4.Left            = 183;
  Circle4.Top             = 116;
  Circle4.Radius          = 12;
  Circle4.Pen_Width       = 1;
  Circle4.Pen_Color       = 0xFFFF;
  Circle4.Visible         = 1;
  Circle4.Active          = 1;
  Circle4.Transparent     = 1;
  Circle4.Gradient        = 0;
  Circle4.Gradient_Orientation    = 0;
  Circle4.Gradient_Start_Color    = 0xFFFF;
  Circle4.Gradient_End_Color      = 0xC618;
  Circle4.Color           = 0x0000;
  Circle4.PressColEnabled = 1;
  Circle4.Press_Color     = 0x8410;
  Circle4.OnUpPtr         = 0;
  Circle4.OnDownPtr       = 0;
  Circle4.OnClickPtr      = 0;
  Circle4.OnPressPtr      = 0;

  Circle5.OwnerScreen     = &Screen1;
  Circle5.Order           = 5;
  Circle5.Left            = 220;
  Circle5.Top             = 167;
  Circle5.Radius          = 6;
  Circle5.Pen_Width       = 1;
  Circle5.Pen_Color       = 0xFFFF;
  Circle5.Visible         = 1;
  Circle5.Active          = 1;
  Circle5.Transparent     = 1;
  Circle5.Gradient        = 0;
  Circle5.Gradient_Orientation    = 0;
  Circle5.Gradient_Start_Color    = 0xFFFF;
  Circle5.Gradient_End_Color      = 0xC618;
  Circle5.Color           = 0x0000;
  Circle5.PressColEnabled = 1;
  Circle5.Press_Color     = 0x8410;
  Circle5.OnUpPtr         = 0;
  Circle5.OnDownPtr       = 0;
  Circle5.OnClickPtr      = 0;
  Circle5.OnPressPtr      = 0;

  Circle6.OwnerScreen     = &Screen1;
  Circle6.Order           = 6;
  Circle6.Left            = 178;
  Circle6.Top             = 50;
  Circle6.Radius          = 12;
  Circle6.Pen_Width       = 1;
  Circle6.Pen_Color       = 0xFFFF;
  Circle6.Visible         = 1;
  Circle6.Active          = 1;
  Circle6.Transparent     = 1;
  Circle6.Gradient        = 0;
  Circle6.Gradient_Orientation    = 0;
  Circle6.Gradient_Start_Color    = 0xFFFF;
  Circle6.Gradient_End_Color      = 0xC618;
  Circle6.Color           = 0x0000;
  Circle6.PressColEnabled = 1;
  Circle6.Press_Color     = 0x8410;
  Circle6.OnUpPtr         = 0;
  Circle6.OnDownPtr       = 0;
  Circle6.OnClickPtr      = 0;
  Circle6.OnPressPtr      = 0;

  Circle7.OwnerScreen     = &Screen1;
  Circle7.Order           = 7;
  Circle7.Left            = 210;
  Circle7.Top             = 108;
  Circle7.Radius          = 4;
  Circle7.Pen_Width       = 1;
  Circle7.Pen_Color       = 0xFFFF;
  Circle7.Visible         = 1;
  Circle7.Active          = 1;
  Circle7.Transparent     = 1;
  Circle7.Gradient        = 0;
  Circle7.Gradient_Orientation    = 0;
  Circle7.Gradient_Start_Color    = 0xFFFF;
  Circle7.Gradient_End_Color      = 0xC618;
  Circle7.Color           = 0x0000;
  Circle7.PressColEnabled = 1;
  Circle7.Press_Color     = 0x8410;
  Circle7.OnUpPtr         = 0;
  Circle7.OnDownPtr       = 0;
  Circle7.OnClickPtr      = 0;
  Circle7.OnPressPtr      = 0;

  Circle8.OwnerScreen     = &Screen1;
  Circle8.Order           = 8;
  Circle8.Left            = 225;
  Circle8.Top             = 195;
  Circle8.Radius          = 4;
  Circle8.Pen_Width       = 1;
  Circle8.Pen_Color       = 0xFFFF;
  Circle8.Visible         = 1;
  Circle8.Active          = 1;
  Circle8.Transparent     = 1;
  Circle8.Gradient        = 0;
  Circle8.Gradient_Orientation    = 0;
  Circle8.Gradient_Start_Color    = 0xFFFF;
  Circle8.Gradient_End_Color      = 0xC618;
  Circle8.Color           = 0x0000;
  Circle8.PressColEnabled = 1;
  Circle8.Press_Color     = 0x8410;
  Circle8.OnUpPtr         = 0;
  Circle8.OnDownPtr       = 0;
  Circle8.OnClickPtr      = 0;
  Circle8.OnPressPtr      = 0;

  Circle9.OwnerScreen     = &Screen1;
  Circle9.Order           = 9;
  Circle9.Left            = 230;
  Circle9.Top             = 115;
  Circle9.Radius          = 4;
  Circle9.Pen_Width       = 1;
  Circle9.Pen_Color       = 0xFFFF;
  Circle9.Visible         = 1;
  Circle9.Active          = 1;
  Circle9.Transparent     = 1;
  Circle9.Gradient        = 0;
  Circle9.Gradient_Orientation    = 0;
  Circle9.Gradient_Start_Color    = 0xFFFF;
  Circle9.Gradient_End_Color      = 0xC618;
  Circle9.Color           = 0x0000;
  Circle9.PressColEnabled = 1;
  Circle9.Press_Color     = 0x8410;
  Circle9.OnUpPtr         = 0;
  Circle9.OnDownPtr       = 0;
  Circle9.OnClickPtr      = 0;
  Circle9.OnPressPtr      = 0;

  Circle10.OwnerScreen     = &Screen1;
  Circle10.Order           = 10;
  Circle10.Left            = 201;
  Circle10.Top             = 195;
  Circle10.Radius          = 6;
  Circle10.Pen_Width       = 1;
  Circle10.Pen_Color       = 0xFFFF;
  Circle10.Visible         = 1;
  Circle10.Active          = 1;
  Circle10.Transparent     = 1;
  Circle10.Gradient        = 0;
  Circle10.Gradient_Orientation    = 0;
  Circle10.Gradient_Start_Color    = 0xFFFF;
  Circle10.Gradient_End_Color      = 0xC618;
  Circle10.Color           = 0x0000;
  Circle10.PressColEnabled = 1;
  Circle10.Press_Color     = 0x8410;
  Circle10.OnUpPtr         = 0;
  Circle10.OnDownPtr       = 0;
  Circle10.OnClickPtr      = 0;
  Circle10.OnPressPtr      = 0;

  Circle11.OwnerScreen     = &Screen1;
  Circle11.Order           = 11;
  Circle11.Left            = 215;
  Circle11.Top             = 122;
  Circle11.Radius          = 6;
  Circle11.Pen_Width       = 1;
  Circle11.Pen_Color       = 0xFFFF;
  Circle11.Visible         = 1;
  Circle11.Active          = 1;
  Circle11.Transparent     = 1;
  Circle11.Gradient        = 0;
  Circle11.Gradient_Orientation    = 0;
  Circle11.Gradient_Start_Color    = 0xFFFF;
  Circle11.Gradient_End_Color      = 0xC618;
  Circle11.Color           = 0x0000;
  Circle11.PressColEnabled = 1;
  Circle11.Press_Color     = 0x8410;
  Circle11.OnUpPtr         = 0;
  Circle11.OnDownPtr       = 0;
  Circle11.OnClickPtr      = 0;
  Circle11.OnPressPtr      = 0;

  Circle12.OwnerScreen     = &Screen1;
  Circle12.Order           = 12;
  Circle12.Left            = 202;
  Circle12.Top             = 74;
  Circle12.Radius          = 6;
  Circle12.Pen_Width       = 1;
  Circle12.Pen_Color       = 0xFFFF;
  Circle12.Visible         = 1;
  Circle12.Active          = 1;
  Circle12.Transparent     = 1;
  Circle12.Gradient        = 0;
  Circle12.Gradient_Orientation    = 0;
  Circle12.Gradient_Start_Color    = 0xFFFF;
  Circle12.Gradient_End_Color      = 0xC618;
  Circle12.Color           = 0x0000;
  Circle12.PressColEnabled = 1;
  Circle12.Press_Color     = 0x8410;
  Circle12.OnUpPtr         = 0;
  Circle12.OnDownPtr       = 0;
  Circle12.OnClickPtr      = 0;
  Circle12.OnPressPtr      = 0;

  Circle13.OwnerScreen     = &Screen1;
  Circle13.Order           = 13;
  Circle13.Left            = 219;
  Circle13.Top             = 82;
  Circle13.Radius          = 8;
  Circle13.Pen_Width       = 1;
  Circle13.Pen_Color       = 0xFFFF;
  Circle13.Visible         = 1;
  Circle13.Active          = 1;
  Circle13.Transparent     = 1;
  Circle13.Gradient        = 0;
  Circle13.Gradient_Orientation    = 0;
  Circle13.Gradient_Start_Color    = 0xFFFF;
  Circle13.Gradient_End_Color      = 0xC618;
  Circle13.Color           = 0x0000;
  Circle13.PressColEnabled = 1;
  Circle13.Press_Color     = 0x8410;
  Circle13.OnUpPtr         = 0;
  Circle13.OnDownPtr       = 0;
  Circle13.OnClickPtr      = 0;
  Circle13.OnPressPtr      = 0;

  Circle14.OwnerScreen     = &Screen1;
  Circle14.Order           = 14;
  Circle14.Left            = 186;
  Circle14.Top             = 9;
  Circle14.Radius          = 4;
  Circle14.Pen_Width       = 1;
  Circle14.Pen_Color       = 0xFFFF;
  Circle14.Visible         = 1;
  Circle14.Active          = 1;
  Circle14.Transparent     = 1;
  Circle14.Gradient        = 0;
  Circle14.Gradient_Orientation    = 0;
  Circle14.Gradient_Start_Color    = 0xFFFF;
  Circle14.Gradient_End_Color      = 0xC618;
  Circle14.Color           = 0x0000;
  Circle14.PressColEnabled = 1;
  Circle14.Press_Color     = 0x8410;
  Circle14.OnUpPtr         = 0;
  Circle14.OnDownPtr       = 0;
  Circle14.OnClickPtr      = 0;
  Circle14.OnPressPtr      = 0;

  Circle15.OwnerScreen     = &Screen1;
  Circle15.Order           = 15;
  Circle15.Left            = 215;
  Circle15.Top             = 144;
  Circle15.Radius          = 4;
  Circle15.Pen_Width       = 1;
  Circle15.Pen_Color       = 0xFFFF;
  Circle15.Visible         = 1;
  Circle15.Active          = 1;
  Circle15.Transparent     = 1;
  Circle15.Gradient        = 0;
  Circle15.Gradient_Orientation    = 0;
  Circle15.Gradient_Start_Color    = 0xFFFF;
  Circle15.Gradient_End_Color      = 0xC618;
  Circle15.Color           = 0x0000;
  Circle15.PressColEnabled = 1;
  Circle15.Press_Color     = 0x8410;
  Circle15.OnUpPtr         = 0;
  Circle15.OnDownPtr       = 0;
  Circle15.OnClickPtr      = 0;
  Circle15.OnPressPtr      = 0;

  Label1.OwnerScreen     = &Screen1;
  Label1.Order          = 16;
  Label1.Left           = 4;
  Label1.Top            = 72;
  Label1.Width          = 164;
  Label1.Height         = 27;
  Label1.Visible        = 1;
  Label1.Active         = 1;
  Label1.Caption        = Label1_Caption;
  Label1.FontName       = Tahoma29x29;
  Label1.Font_Color     = 0xFFFF;
  Label1.OnUpPtr         = 0;
  Label1.OnDownPtr       = 0;
  Label1.OnClickPtr      = 0;
  Label1.OnPressPtr      = 0;

  Label2.OwnerScreen     = &Screen1;
  Label2.Order          = 17;
  Label2.Left           = 7;
  Label2.Top            = 100;
  Label2.Width          = 148;
  Label2.Height         = 27;
  Label2.Visible        = 1;
  Label2.Active         = 1;
  Label2.Caption        = Label2_Caption;
  Label2.FontName       = Tahoma29x29;
  Label2.Font_Color     = 0xFFFF;
  Label2.OnUpPtr         = 0;
  Label2.OnDownPtr       = 0;
  Label2.OnClickPtr      = 0;
  Label2.OnPressPtr      = 0;

  Circle16.OwnerScreen     = &Screen1;
  Circle16.Order           = 18;
  Circle16.Left            = 212;
  Circle16.Top             = 0;
  Circle16.Radius          = 20;
  Circle16.Pen_Width       = 1;
  Circle16.Pen_Color       = 0xFFFF;
  Circle16.Visible         = 1;
  Circle16.Active          = 1;
  Circle16.Transparent     = 1;
  Circle16.Gradient        = 0;
  Circle16.Gradient_Orientation    = 0;
  Circle16.Gradient_Start_Color    = 0xFFFF;
  Circle16.Gradient_End_Color      = 0xC618;
  Circle16.Color           = 0x0000;
  Circle16.PressColEnabled = 1;
  Circle16.Press_Color     = 0x8410;
  Circle16.OnUpPtr         = 0;
  Circle16.OnDownPtr       = 0;
  Circle16.OnClickPtr      = 0;
  Circle16.OnPressPtr      = 0;

  Circle17.OwnerScreen     = &Screen1;
  Circle17.Order           = 19;
  Circle17.Left            = 190;
  Circle17.Top             = 30;
  Circle17.Radius          = 4;
  Circle17.Pen_Width       = 1;
  Circle17.Pen_Color       = 0xFFFF;
  Circle17.Visible         = 1;
  Circle17.Active          = 1;
  Circle17.Transparent     = 1;
  Circle17.Gradient        = 0;
  Circle17.Gradient_Orientation    = 0;
  Circle17.Gradient_Start_Color    = 0xFFFF;
  Circle17.Gradient_End_Color      = 0xC618;
  Circle17.Color           = 0x0000;
  Circle17.PressColEnabled = 1;
  Circle17.Press_Color     = 0x8410;
  Circle17.OnUpPtr         = 0;
  Circle17.OnDownPtr       = 0;
  Circle17.OnClickPtr      = 0;
  Circle17.OnPressPtr      = 0;

  Circle18.OwnerScreen     = &Screen1;
  Circle18.Order           = 20;
  Circle18.Left            = 99;
  Circle18.Top             = 6;
  Circle18.Radius          = 18;
  Circle18.Pen_Width       = 1;
  Circle18.Pen_Color       = 0xFFFF;
  Circle18.Visible         = 1;
  Circle18.Active          = 1;
  Circle18.Transparent     = 1;
  Circle18.Gradient        = 0;
  Circle18.Gradient_Orientation    = 0;
  Circle18.Gradient_Start_Color    = 0xFFFF;
  Circle18.Gradient_End_Color      = 0xC618;
  Circle18.Color           = 0x0000;
  Circle18.PressColEnabled = 1;
  Circle18.Press_Color     = 0x8410;
  Circle18.OnUpPtr         = 0;
  Circle18.OnDownPtr       = 0;
  Circle18.OnClickPtr      = 0;
  Circle18.OnPressPtr      = 0;

  Circle20.OwnerScreen     = &Screen1;
  Circle20.Order           = 21;
  Circle20.Left            = 158;
  Circle20.Top             = 46;
  Circle20.Radius          = 4;
  Circle20.Pen_Width       = 1;
  Circle20.Pen_Color       = 0xFFFF;
  Circle20.Visible         = 1;
  Circle20.Active          = 1;
  Circle20.Transparent     = 1;
  Circle20.Gradient        = 0;
  Circle20.Gradient_Orientation    = 0;
  Circle20.Gradient_Start_Color    = 0xFFFF;
  Circle20.Gradient_End_Color      = 0xC618;
  Circle20.Color           = 0x0000;
  Circle20.PressColEnabled = 1;
  Circle20.Press_Color     = 0x8410;
  Circle20.OnUpPtr         = 0;
  Circle20.OnDownPtr       = 0;
  Circle20.OnClickPtr      = 0;
  Circle20.OnPressPtr      = 0;
}

static char IsInsideObject (unsigned int X, unsigned int Y, unsigned int Left, unsigned int Top, unsigned int Width, unsigned int Height) { // static
  if ( (Left<= X) && (Left+ Width - 1 >= X) &&
       (Top <= Y)  && (Top + Height - 1 >= Y) )
    return 1;
  else
    return 0;
}


#define GetLabel(index)               CurrentScreen->Labels[index]
#define GetImage(index)               CurrentScreen->Images[index]
#define GetCircle(index)              CurrentScreen->Circles[index]


void DrawLabel(TLabel *ALabel) {
int x_pos, y_pos;
  x_pos = 0;
  y_pos = 0;
  if (ALabel->Visible == 1) {
    TFT_Set_Font(ALabel->FontName, ALabel->Font_Color, FO_HORIZONTAL);
    TFT_Write_Text_Return_Pos(ALabel->Caption, ALabel->Left, ALabel->Top);
    x_pos = ALabel->Left + ((int)(ALabel->Width - caption_length) / 2);
    y_pos = ALabel->Top + ((int)(ALabel->Height - caption_height) / 2);
    if ((x_pos >= 0) && (y_pos >= 0)) {
      TFT_Write_Text(ALabel->Caption, x_pos, y_pos);
    }
    else {
      TFT_Write_Text(ALabel->Caption, ALabel->Left, ALabel->Top);
    }
  }
}

void DrawImage(TImage *AImage) {
  if (AImage->Visible) {
    TFT_Image(AImage->Left, AImage->Top, AImage->Picture_Name, AImage->Picture_Ratio);
  }
}

void DrawCircle(TCircle *ACircle) {
  if (ACircle->Visible == 1) {
    if (object_pressed == 1) {
      object_pressed = 0;
      TFT_Set_Brush(ACircle->Transparent, ACircle->Color, ACircle->Gradient, ACircle->Gradient_Orientation,
                    ACircle->Gradient_End_Color, ACircle->Gradient_Start_Color);
    }
    else {
      TFT_Set_Brush(ACircle->Transparent, ACircle->Color, ACircle->Gradient, ACircle->Gradient_Orientation,
                    ACircle->Gradient_Start_Color, ACircle->Gradient_End_Color);
    }
    TFT_Set_Pen(ACircle->Pen_Color, ACircle->Pen_Width);
    TFT_Circle(ACircle->Left + ACircle->Radius,
               ACircle->Top + ACircle->Radius,
               ACircle->Radius);
  }
}

void DrawScreen(TScreen *aScreen) {
  unsigned short order;
  unsigned short label_idx;
  TLabel *local_label;
  unsigned short image_idx;
  TImage *local_image;
  unsigned short circle_idx;
  TCircle *local_circle;
  char save_bled, save_bled_direction;

  order = 0;
  label_idx = 0;
  image_idx = 0;
  circle_idx = 0;
  CurrentScreen = aScreen;

  if ((display_width != CurrentScreen->Width) || (display_height != CurrentScreen->Height)) {
    save_bled = TFT_BLED;
    save_bled_direction = TFT_BLED_Direction;
    TFT_BLED_Direction = 1;
    TFT_BLED           = 0;
    TFT_Init(CurrentScreen->Width, CurrentScreen->Height);
    TFT_Fill_Screen(CurrentScreen->Color);
    display_width = CurrentScreen->Width;
    display_height = CurrentScreen->Height;
    TFT_BLED           = save_bled;
    TFT_BLED_Direction = save_bled_direction;
  }
  else
    TFT_Fill_Screen(CurrentScreen->Color);

  CurrentScreen = aScreen;

  while (order < CurrentScreen->ObjectsCount) {
    if (label_idx < CurrentScreen->LabelsCount) {
      local_label = GetLabel(label_idx);
      if (order == local_label->Order) {
        label_idx++;
        order++;
        DrawLabel(local_label);
      }
    }

    if (circle_idx < CurrentScreen->CirclesCount) {
      local_circle = GetCircle(circle_idx);
      if (order == local_circle->Order) {
        circle_idx++;
        order++;
        DrawCircle(local_circle);
      }
    }

    if (image_idx  < CurrentScreen->ImagesCount) {
      local_image = GetImage(image_idx);
      if (order == local_image->Order) {
        image_idx++;
        order++;
        DrawImage(local_image);
      }
    }

  }
}

void Init_MCU() {
  OSC_CTRL = 0x02;          // 32MHz internal RC oscillator
  while(RC32MRDY_bit == 0)
  ;
  CPU_CCP = 0xD8;
  CLK_CTRL = 1;             // set system clock to 32MHz internal RC oscillator
}

void Start_TP() {
  Init_MCU();

  InitializeTouchPanel();

  InitializeObjects();
  display_width = Screen1.Width;
  display_height = Screen1.Height;
  DrawScreen(&Screen1);
}